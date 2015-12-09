
//
//  Parser
//

#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include "parser.h"
#include "bytecode.h"
#include "error.h"
#include "value.h"


// * The parser converts lexed source code into bytecode
// * A `Parser` struct is used for each function
// * The top level source of a file (not inside a function) is treated as the
//   package's main function
// * A function has a main block and arguments
// * A block consists of a series of statements (eg. if, while, loop, for, etc.)
// * A statement itself may have another block (eg. while loops), which is
//   parsed recursively
//
// * Variables (locals) are stored in a stack in the order they were defined
// * Each local stores the scope depth at which it was defined
// * A new scope is defined at the start of each block and freed at the end of
//   the block
// * When a scope is freed, all variables defined in that scope are freed


// The maximum number of locals that can be allocated on the stack at once.
#define MAX_LOCALS 512


// A local variable.
typedef struct {
	// The name of the local.
	char *name;
	size_t length;

	// The scope depth the local was defined at.
	uint32_t scope_depth;

	// The index of the upvalue in the VM's upvalue list, or -1 if this local
	// wasn't used in a closure.
	int upvalue_index;
} Local;


// A loop.
typedef struct loop {
	// The index of the last break statement's jump instruction in the bytecode.
	// Used to form a jump list which can be patched after the loop has finished
	// being compiled. -1 if no break statements are used.
	int jump;

	// The next loop in the linked list.
	struct loop *outer;
} Loop;


// A parser, which converts lexed source code into bytecode.
typedef struct _parser {
	// The virtual machine we're parsing for.
	VirtualMachine *vm;

	// A pointer to the parent parser. NULL if this parser is top level.
	struct _parser *parent;

	// The lexer.
	Lexer *lexer;

	// The function we're compiling the source code into.
	Function *fn;

	// The innermost loop, or NULL if we're not inside a loop.
	Loop *loop;

	// The current scope depth.
	uint32_t scope_depth;

	// All defined locals.
	Local locals[MAX_LOCALS];
	uint32_t locals_count;
} Parser;

// Creates a new parser.
Parser parser_new(Parser *parent);

// Parses a block of statements, terminated by the given character.
void parse_block(Parser *parser, Token terminator);

// Parses a function call to the function in `slot`, storing the return value in
// `return_slot`.
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
	uint16_t return_slot);

// Parses a function definition body (starting at the arguments list).
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length);

// Parses a struct instantiation, storing the resulting struct into `slot`.
void parse_struct_instantiation(Parser *parser, uint16_t slot);



//
//  Errors
//

// Triggers a custom error.
#define ERROR(...) \
	parser->vm->err = err_new(lexer_line(parser->lexer), __VA_ARGS__);


// Triggers an unexpected token error.
#define UNEXPECTED(...)                                         \
	parser->vm->err = err_unexpected(lexer_line(parser->lexer), \
		parser->lexer->token, parser->lexer->value, __VA_ARGS__);


// Triggers an unexpected token error if the current token does not match the
// given one.
#define EXPECT(expected, ...)                 \
	if (parser->lexer->token != (expected)) { \
		UNEXPECTED(__VA_ARGS__);              \
		return;                               \
	}



//
//  Imports
//

// Imports a package with the given name.
void import(Parser *parser, char *name) {

}


// Parses a multi-import statement.
void parse_multi_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Ensure there's at least one string within the parentheses
	EXPECT(TOKEN_STRING, "Expected package name after `(`");

	// Expect a comma separated list of strings
	while (!vm_has_error(parser->vm) && lexer->token == TOKEN_STRING) {
		// Import the package
		char *name = lexer_extract_string(lexer->value.identifier);
		import(parser, name);

		// Consume the string
		lexer_next(lexer);

		// Expect a comma
		EXPECT(TOKEN_COMMA, "Expected `,` after package name");
		lexer_next(lexer);
	}

	// Expect a close parenthesis
	EXPECT(TOKEN_CLOSE_PARENTHESIS, "Expected `)` to close import list");
	lexer_next(lexer);
}


// Parses a single import statement.
void parse_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Parse a multi-import statement if the next token is an open parenthesis
	if (lexer->token == TOKEN_OPEN_PARENTHESIS) {
		// Consume the open parenthesis
		lexer_next(lexer);

		// Multi-import statement
		parse_multi_import(parser);
	} else if (lexer->token == TOKEN_STRING) {
		// Single import statement
		char *name = lexer_extract_string(lexer->value.identifier);
		import(parser, name);

		// Consume the string token
		lexer_next(lexer);
	} else {
		// Unexpected token
		UNEXPECTED("Expected package name or `(` after `import`");
	}
}


// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Continually parse import statements
	while (!vm_has_error(parser->vm) && lexer->token == TOKEN_IMPORT) {
		// Consume the `import`
		lexer_next(lexer);

		// Parse the rest of the import
		parse_import(parser);
	}
}



//
//  Local Variables
//

// Creates a new local at the top of the locals stack. Returns NULL if a local
// couldn't be allocated.
Local * local_new(Parser *parser, uint16_t *slot) {
	// Check we haven't exceeded the maximum number of allowed locals
	if (parser->locals_count >= MAX_LOCALS) {
		ERROR("Cannot have more than %d variables in a scope", MAX_LOCALS);
		return NULL;
	}

	uint16_t index = parser->locals_count++;
	if (slot != NULL) {
		*slot = index;
	}

	Local *local = &parser->locals[index];
	local->name = NULL;
	local->length = 0;
	local->scope_depth = parser->scope_depth;
	local->upvalue_index = -1;
	return local;
}


// Searches a parser's locals list for a local called `name`, returning its
// stack slot index, or -1 if the local doesn't exist.
int local_find(Parser *parser, char *name, size_t length) {
	// Iterate over the locals backwards, as locals are more likely to be used
	// right after they've been defined (maybe)
	for (int i = parser->locals_count - 1; i >= 0; i--) {
		Local *local = &parser->locals[i];

		// Check if this is the correct local
		if (local->length == length && strncmp(local->name, name, length) == 0) {
			// Return the slot the local is in
			return i;
		}
	}

	return -1;
}


// Searches parent compiler locals recursively for a local called `name`,
// returning the index of a new upvalue created from the local, or -1 if no such
// local could be found.
int local_find_all(Parser *parser, char *name, size_t length) {
	if (parser == NULL) {
		// No more parent compilers, local is undefined
		return -1;
	}

	int slot = local_find(parser, name, length);
	if (slot >= 0) {
		// Create an upvalue from the local
		int index;
		Upvalue *upvalue = upvalue_new(parser->vm, &index);
		upvalue->name = name;
		upvalue->length = length;
		upvalue->defining_fn = parser->fn;
		upvalue->slot = slot;

		// Set the local as an upvalue
		Local *local = &parser->locals[slot];
		local->upvalue_index = index;

		return index;
	} else {
		// Search parent compiler
		return local_find_all(parser->parent, name, length);
	}
}


// The type of a variable.
typedef enum {
	VAR_UNDEFINED,
	VAR_LOCAL,
	VAR_UPVALUE,
} VariableType;


// A variable (upvalue or local).
typedef struct {
	// The type of the variable.
	VariableType type;

	// The slot position of the local or upvalue (index into the VM's upvalues
	// list).
	uint16_t slot;
} Variable;


// Returns a local or upvalue with the given name. First searches the parser's
// locals list, then the existing upvalues, then parent parsers' locals.
Variable local_capture(Parser *parser, char *name, size_t length) {
	Variable result;
	int slot;

	// Search parser locals
	slot = local_find(parser, name, length);
	if (slot >= 0) {
		result.type = VAR_LOCAL;
		result.slot = slot;
		return result;
	}

	// Search existing upvalues
	slot = upvalue_find(parser->vm, name, length);
	if (slot >= 0) {
		result.type = VAR_UPVALUE;
		result.slot = slot;
		return result;
	}

	// Search parent locals
	slot = local_find_all(parser->parent, name, length);
	if (slot >= 0) {
		result.type = VAR_UPVALUE;
		result.slot = slot;
		return result;
	}

	result.type = VAR_UNDEFINED;
	return result;
}


// Emits close upvalue instructions for all locals that have been used in
// closures.
void local_close_upvalues(Parser *parser) {
	// Emit in reverse order
	for (int i = parser->locals_count; i >= 0; i--) {
		int upvalue = parser->locals[i].upvalue_index;
		if (upvalue >= 0) {
			// Emit close upvalue instruction
			emit(parser->fn, instr_new(UPVALUE_CLOSE, upvalue, 0, 0));
		}
	}
}


// Increments the parser's scope depth.
void scope_new(Parser *parser) {
	parser->scope_depth++;
}


// Decrements the parser's scope depth, removing all locals from the stack
// created in that scope.
void scope_free(Parser *parser) {
	parser->scope_depth--;

	// Since the locals are stored in order of stack depth, with the locals
	// allocated in the deepest scope stored at the end of the array,
	// continually decrease the size of the array until we hit a local in a
	// scope that is still active
	int i = parser->locals_count - 1;
	while (i >= 0 && parser->locals[i].scope_depth > parser->scope_depth) {
		// Check if the local was used as an upvalue
		int upvalue = parser->locals[i].upvalue_index;
		if (upvalue >= 0) {
			// Close the upvalue
			emit(parser->fn, instr_new(UPVALUE_CLOSE, upvalue, 0, 0));
		}

		parser->locals_count--;
		i--;
	}
}



//
//  Expressions
//

// Possible operator precedences.
typedef enum {
	PREC_NONE,
	// Boolean operators
	PREC_OR,
	PREC_AND,
	// Bitwise operators
	PREC_BIT_OR,
	PREC_BIT_XOR,
	PREC_BIT_AND,
	// Equal, not equal
	PREC_EQ,
	// Less than, less than equal, greater than, greater than equal
	PREC_ORD,
	// Addition, subtraction
	PREC_ADD,
	// Concatenation
	PREC_CONCAT,
	// Multiplication, division, modulo
	PREC_MUL,
} Precedence;


// The type of an operand to an expression.
typedef enum {
	OP_LOCAL,
	OP_INTEGER,
	OP_NUMBER,
	OP_STRING,
	OP_PRIMITIVE,
	OP_FN,
	OP_JUMP,
	OP_NONE,
} OperandType;


// An operand in an expression.
typedef struct {
	// The type of the operand.
	OperandType type;

	// The value of the operand. Numbers and strings are stored as indices into
	// the VM's number/string list.
	union {
		int16_t integer;
		uint16_t number;
		uint16_t string;
		uint16_t primitive;
		uint16_t slot;
		uint16_t fn_index;
		uint16_t value;

		uint32_t jump;
	};
} Operand;


// Evaluates to true if an operand is a number.
#define IS_NUMBER(type) (type == OP_INTEGER || type == OP_NUMBER)

// Evaluates to true if an operand is a jump or local.
#define IS_JUMP_OR_LOCAL(type) ((type) == OP_LOCAL || (type) == OP_JUMP)


// Parses an expression, stopping when we reach a binary operator of lower
// precedence than the given precedence.
Operand expr_prec(Parser *parser, uint16_t slot, Precedence precedence);


// Returns the precedence of a binary operator.
Precedence binary_prec(Token operator) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
		return PREC_ADD;
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return PREC_MUL;
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return PREC_EQ;
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return PREC_ORD;
	case TOKEN_AND:
		return PREC_AND;
	case TOKEN_OR:
		return PREC_OR;
	case TOKEN_BIT_AND:
		return PREC_BIT_AND;
	case TOKEN_BIT_OR:
		return PREC_BIT_OR;
	case TOKEN_BIT_XOR:
		return PREC_BIT_XOR;
	default:
		return PREC_NONE;
	}
}


// Returns the opcode for an arithmetic operation (including concatenation).
// Either left or right must be a local.
Opcode arithmetic_opcode(Token operator, OperandType left, OperandType right) {
	if (operator == TOKEN_CONCAT) {
		// Concatenation
		int offset = (right == OP_STRING ? 1 : (left == OP_STRING ? 2 : 0));
		return CONCAT_LL + offset;
	} else {
		// Arithmetic
		int base = ADD_LL + (operator - TOKEN_ADD) * 5;
		return base + (left == OP_LOCAL ? right : left + 2);
	}
}


// Returns the inverted opcode for a comparison operation. Either left or right
// must be a local.
Opcode comparison_opcode(Token operator, OperandType left, OperandType right) {
	Opcode base;
	switch (operator) {
	case TOKEN_EQ:
		base = NEQ_LL;
		break;
	case TOKEN_NEQ:
		base = EQ_LL;
		break;
	case TOKEN_LT:
		base = GE_LL;
		break;
	case TOKEN_LE:
		base = GT_LL;
		break;
	case TOKEN_GT:
		base = LE_LL;
		break;
	case TOKEN_GE:
		base = LT_LL;
		break;
	default:
		return NO_OP;
	}

	return base + (left == OP_LOCAL ? right : left);
}


// Returns true if the given operands are valid for the given binary operation.
bool binary_valid(Token operator, OperandType left, OperandType right) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return (IS_NUMBER(left) || left == OP_LOCAL) &&
			(IS_NUMBER(right) || right == OP_LOCAL);
	case TOKEN_CONCAT:
		return (left == OP_STRING || left == OP_LOCAL) &&
			(right == OP_STRING || right == OP_LOCAL);
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return (left >= OP_LOCAL && left <= OP_PRIMITIVE) &&
			(right >= OP_LOCAL && right <= OP_PRIMITIVE);
	case TOKEN_AND:
	case TOKEN_OR:
		return ((left >= OP_LOCAL && left <= OP_PRIMITIVE) || left == OP_JUMP) &&
			((right >= OP_LOCAL && right <= OP_PRIMITIVE) || right == OP_JUMP);
	default:
		return false;
	}
}


// Performs an arithmetic operation on two integers.
int32_t binary_integer_arithmetic(Token operator, int16_t left, int16_t right) {
	switch (operator) {
	case TOKEN_ADD:
		return left + right;
	case TOKEN_SUB:
		return left - right;
	case TOKEN_MUL:
		return left * right;
	case TOKEN_MOD:
		return left % right;
	default:
		return 0;
	}
}


// Performs an arithmetic operation on two doubles.
double binary_number_arithmetic(Token operator, double left, double right) {
	switch (operator) {
	case TOKEN_ADD:
		return left + right;
	case TOKEN_SUB:
		return left - right;
	case TOKEN_MUL:
		return left * right;
	case TOKEN_DIV:
		return left / right;
	case TOKEN_MOD:
		return fmod(left, right);
	default:
		return 0;
	}
}


// Returns the opcode for a unary operator.
Opcode unary_opcode(Token operator) {
	switch (operator) {
	case TOKEN_SUB:
		return NEG_L;
	default:
		return NO_OP;
	}
}


// Returns true if the given operand is valid for the given unary operation.
bool unary_valid(Opcode operator, OperandType operand) {
	switch (operator) {
	case NEG_L:
		return IS_NUMBER(operand) || operand == OP_LOCAL;
	default:
		return false;
	}
}


// Converts an integer or number operand into a double value.
double operand_to_number(Parser *parser, Operand operand) {
	if (operand.type == OP_NUMBER) {
		return parser->vm->numbers[operand.number];
	} else {
		return (double) operand.integer;
	}
}


// Converts an operand (that isn't a local) into a true or false value.
bool operand_to_boolean(Operand operand) {
	return operand.type == OP_PRIMITIVE && operand.primitive == TRUE_TAG;
}


// Attempts to fold an `and` or `or` operation.
Operand fold_condition(Token operator, Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Don't fold if both are locals
	if (IS_JUMP_OR_LOCAL(left.type) && IS_JUMP_OR_LOCAL(right.type)) {
		return operand;
	}

	// Convert each operand into a boolean
	bool first = operand_to_boolean(left);
	bool second = operand_to_boolean(right);

	// If neither are locals
	if (!IS_JUMP_OR_LOCAL(left.type) && !IS_JUMP_OR_LOCAL(right.type)) {
		bool result = (operator == TOKEN_OR) ? (first || second) : (first && second);
		operand.type = OP_PRIMITIVE;
		operand.primitive = result ? TRUE_TAG : FALSE_TAG;
		return operand;
	}

	// Make operand and constant agnostic of order
	Operand local = IS_JUMP_OR_LOCAL(left.type) ? left : right;
	bool constant = IS_JUMP_OR_LOCAL(left.type) ? second : first;

	// Either left or right is a local, but not both
	if (operator == TOKEN_AND) {
		if (constant) {
			// Something && true = Something
			return local;
		} else {
			// Something && false = false
			operand.type = OP_PRIMITIVE;
			operand.primitive = FALSE_TAG;
			return operand;
		}
	} else {
		if (!constant) {
			// Something || false = something
			return local;
		} else {
			// Something || true = true
			operand.type = OP_PRIMITIVE;
			operand.primitive = TRUE_TAG;
			return operand;
		}
	}
}


// Attempts to fold an equality test.
Operand fold_equal(Parser *parser, Token operator, Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Only fold if types are equal
	if (left.type != right.type) {
		return operand;
	}

	// If their values are equal
	operand.type = OP_PRIMITIVE;
	if (left.value == right.value) {
		operand.primitive = (operator == TOKEN_EQ) ? TRUE_TAG : FALSE_TAG;
		return operand;
	}

	// Don't fold non-identical locals
	if (left.type == OP_LOCAL) {
		operand.type = OP_NONE;
		return operand;
	}

	// Try equality tests depending on the operand type
	switch (left.type) {
	case OP_NUMBER: {
		double first = parser->vm->numbers[left.number];
		double second = parser->vm->numbers[right.number];
		operand.primitive = (first == second) ? TRUE_TAG : FALSE_TAG;
		break;
	}
	case OP_STRING: {
		char *first = parser->vm->strings[left.string];
		char *second = parser->vm->strings[right.string];
		operand.primitive = (strcmp(first, second) == 0) ? TRUE_TAG : FALSE_TAG;
		break;
	}
	default:
		operand.primitive = FALSE_TAG;
		break;
	}

	// Invert the result if we're testing for inequality
	if (operator == TOKEN_NEQ && operand.type == OP_PRIMITIVE) {
		bool equal = (operand.primitive == TRUE_TAG);
		operand.primitive = equal ? FALSE_TAG : TRUE_TAG;
	}

	return operand;
}


// Returns the result of a comparison between two identical locals.
uint16_t compare_locals(Token operator) {
	switch (operator) {
	case TOKEN_LT: return FALSE_TAG;
	case TOKEN_LE: return TRUE_TAG;
	case TOKEN_GT: return FALSE_TAG;
	case TOKEN_GE: return TRUE_TAG;
	default: return FALSE_TAG;
	}
}


// Returns the result of a comparison between two numbers.
uint16_t compare_numbers(Token operator, double left, double right) {
	switch (operator) {
	case TOKEN_LT: return (left < right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_LE: return (left <= right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_GT: return (left > right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_GE: return (left >= right) ? TRUE_TAG : FALSE_TAG;
	default: return FALSE_TAG;
	}
}


// Attempts to fold an order operation.
Operand fold_order(Parser *parser, Token operator, Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	if (left.type == OP_LOCAL && right.type == OP_LOCAL &&
			left.slot == right.slot) {
		// Two identical locals
		operand.type = OP_PRIMITIVE;
		operand.primitive = compare_locals(operator);
	} else if (IS_NUMBER(left.type) && IS_NUMBER(right.type)) {
		// Convert operands into numbers
		double first = operand_to_number(parser, left);
		double second = operand_to_number(parser, right);

		// Compare them
		operand.type = OP_PRIMITIVE;
		operand.primitive = compare_numbers(operator, first, second);
	}

	// Can't fold otherwise
	return operand;
}


// Attempts to fold a concatenation operation.
Operand fold_concat(Parser *parser, Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Only fold if both are strings
	if (left.type != OP_STRING || right.type != OP_STRING) {
		return operand;
	}

	// Extract both strings from the VM
	char *left_str = parser->vm->strings[left.string];
	char *right_str = parser->vm->strings[right.string];
	size_t left_length = strlen(left_str);

	// Combine both strings
	char *result = malloc(sizeof(char) * (left_length + strlen(right_str) + 1));
	strcpy(result, left_str);
	strcpy(&result[left_length], right_str);

	operand.type = OP_STRING;
	operand.string = vm_add_string(parser->vm, result);
	return operand;
}


// Attempts to fold an arithmetic operation.
Operand fold_arithmetic(Parser *parser, Token operator, Operand left,
		Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Only fold if both are numbers
	if (!IS_NUMBER(left.type) || !IS_NUMBER(right.type)) {
		return operand;
	}

	// If both are integers and the operation is not division, return an integer
	if (left.type == OP_INTEGER && right.type == OP_INTEGER &&
			operator != TOKEN_DIV) {
		int32_t result = binary_integer_arithmetic(operator,
			left.integer, right.integer);

		// Return a double if the result is beyond the bounds of a 16 bit
		// integer
		if (result > SHRT_MAX || result < SHRT_MIN) {
			// Convert to a double
			operand.type = OP_NUMBER;
			operand.number = vm_add_number(parser->vm, (double) result);
		} else {
			// Fits inside the range of an unsigned 16 bit integer
			operand.type = OP_INTEGER;
			operand.integer = (int16_t) result;
		}
	} else {
		// Convert both to numbers and compute the operation
		double left_value = operand_to_number(parser, left);
		double right_value = operand_to_number(parser, right);
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm,
			binary_number_arithmetic(operator, left_value, right_value));
	}

	return operand;
}


// Attempts to fold a binary operation. Assumes the operands are valid for the
// operation.
Operand fold_binary(Parser *parser, Token operator, Operand left,
		Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	if (operator == TOKEN_CONCAT) {
		// Concatenation
		return fold_concat(parser, left, right);
	} else if (operator >= TOKEN_ADD && operator <= TOKEN_MOD) {
		// Arithmetic
		return fold_arithmetic(parser, operator, left, right);
	} else if (operator == TOKEN_EQ || operator == TOKEN_NEQ) {
		// Equality
		return fold_equal(parser, operator, left, right);
	} else if (operator >= TOKEN_LT && operator <= TOKEN_GE) {
		// Ordering
		return fold_order(parser, operator, left, right);
	} else if (operator == TOKEN_AND || operator == TOKEN_OR) {
		// Conditional
		return fold_condition(operator, left, right);
	} else {
		// Unknown operator
		return operand;
	}
}


// Returns the complementary conditional operation to that specified by
// `opcode`.
Opcode inverted_conditional_opcode(Opcode opcode) {
	if (opcode == IS_TRUE_L) {
		return IS_FALSE_L;
	} else if (opcode == IS_FALSE_L) {
		return IS_TRUE_L;
	} else if (opcode >= EQ_LL && opcode <= EQ_LP) {
		return NEQ_LL + (opcode - EQ_LL);
	} else if (opcode >= NEQ_LL && opcode <= NEQ_LP) {
		return EQ_LL + (opcode - NEQ_LL);
	} else if (opcode >= LT_LL && opcode <= LT_LN) {
		return GE_LL + (opcode - LT_LL);
	} else if (opcode >= LE_LL && opcode <= LE_LN) {
		return GT_LL + (opcode - LE_LL);
	} else if (opcode >= GT_LL && opcode <= GT_LN) {
		return LE_LL + (opcode - GT_LL);
	} else if (opcode >= GE_LL && opcode <= GE_LN) {
		return LT_LL + (opcode - GE_LL);
	} else {
		return NO_OP;
	}
}


// Inverts the condition of a conditional instruction.
void invert_condition(Function *fn, int index) {
	uint64_t condition = fn->bytecode[index];
	Opcode current = (Opcode) instr_opcode(condition);
	Opcode inverted = inverted_conditional_opcode(current);
	fn->bytecode[index] = instr_modify_opcode(condition, inverted);
}


// Emits bytecode to convert a local operand into a jump.
Operand operand_to_jump(Parser *parser, Operand operand) {
	Operand result;
	result.type = OP_JUMP;

	// Emit a comparison and empty jump instruction
	emit(parser->fn, instr_new(IS_FALSE_L, operand.slot, 0, 0));
	result.jump = jmp_new(parser->fn);
	return operand;
}


// Emits bytecode for an `and` operation. The left operand is expected to
// have the jump operand type.
Operand expr_and(Parser *parser, Operand left, Operand right) {
	// Convert right into a jump
	if (right.type != OP_JUMP) {
		right = operand_to_jump(parser, right);
	}

	Function *fn = parser->fn;

	// Point end of right's jump list to left
	int last = jmp_last(fn, right.jump);
	jmp_append(fn, last, left.jump);

	// Make both operands part of an `and` operation
	if (jmp_type(fn, left.jump) == JUMP_NONE) {
		jmp_set_type(fn, left.jump, JUMP_AND);
	}
	if (jmp_type(fn, right.jump) == JUMP_NONE) {
		jmp_set_type(fn, right.jump, JUMP_AND);
	}

	return right;
}


// Emits bytecode for an `or` operation. The left operand is expected to
// have the jump operand type.
Operand expr_or(Parser *parser, Operand left, Operand right) {
	// Convert right into a jump
	if (right.type != OP_JUMP) {
		right = operand_to_jump(parser, right);
	}

	Function *fn = parser->fn;

	// Point end of right's jump list to left
	int last = jmp_last(fn, right.jump);
	jmp_append
		(fn, last, left.jump);

	// Invert left's condition
	invert_condition(fn, left.jump - 1);

	// Iterate over left's jump list
	int current = left.jump;
	while (current != -1) {
		// Point to after right by default
		int target = right.jump + 1;

		// For conditions part of AND statements
		if (jmp_type(fn, current) == JUMP_AND) {
			// Point to last element in right's jump list
			target = last - 1;
		}

		// Set jump target
		jmp_target(fn, current, target);

		// Get next element in jump list
		current = jmp_next(fn, current);
	}

	// Point left to after right
	jmp_target(fn, left.jump, right.jump + 1);

	// Make both operands part of an `or` operation
	if (jmp_type(fn, left.jump) == JUMP_NONE) {
		jmp_set_type(fn, left.jump, JUMP_OR);
	}
	if (jmp_type(fn, right.jump) == JUMP_NONE) {
		jmp_set_type(fn, right.jump, JUMP_OR);
	}

	return right;
}


// Emits bytecode for a binary operator.
Operand expr_binary(Parser *parser, uint16_t slot, Token operator,
		Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Ensure the operands are valid for the operator
	if (!binary_valid(operator, left.type, right.type)) {
		UNEXPECTED("Invalid operand to binary operator");
		return operand;
	}

	// Attempt to fold the operation
	operand = fold_binary(parser, operator, left, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Handle conditional operations
	if (operator == TOKEN_AND) {
		return expr_and(parser, left, right);
	} else if (operator == TOKEN_OR) {
		return expr_or(parser, left, right);
	}

	if (operator >= TOKEN_ADD && operator <= TOKEN_CONCAT) {
		// Arithmetic
		operand.type = OP_LOCAL;
		operand.slot = slot;

		// Emit the operation
		Opcode opcode = arithmetic_opcode(operator, left.type, right.type);
		emit(parser->fn, instr_new(opcode, slot, left.value, right.value));
	} else if (operator >= TOKEN_EQ && operator <= TOKEN_GE) {
		// Comparison
		operand.type = OP_JUMP;

		// Emit the comparison and the empty jump instruction following it
		Opcode opcode = comparison_opcode(operator, left.type, right.type);
		emit(parser->fn, instr_new(opcode, left.value, right.value, 0));
		operand.jump = jmp_new(parser->fn);
	}

	return operand;
}


// Emits bytecode for the left operator in a binary expression.
Operand expr_binary_left(Parser *parser, Token operator, Operand left) {
	// Turn the operand into a jump statement if we're parsing an `and` or `or`
	// operator
	if ((operator == TOKEN_AND || operator == TOKEN_OR) &&
			left.type == OP_LOCAL) {
		operand_to_jump(parser, left);
	}

	// Don't make any modification to the operand
	return left;
}


// Attempts to fold a unary operation.
Operand fold_unary(Parser *parser, Opcode opcode, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Can only fold negation
	if (opcode != NEG_L) {
		return operand;
	}

	// Can only fold numbers and integers
	if (right.type == OP_NUMBER) {
		double value = parser->vm->numbers[right.number];
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm, -value);
	} else if (right.type == OP_INTEGER) {
		operand.type = OP_INTEGER;
		operand.integer = -right.integer;
	}

	return operand;
}


// Emits bytecode for a unary operator.
Operand expr_unary(Parser *parser, Opcode opcode, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Ensure the operand is valid for the operator
	if (!unary_valid(opcode, right.type)) {
		UNEXPECTED("Invalid operand to unary operator");
		return operand;
	}

	// Attempt to fold the operation
	operand = fold_unary(parser, opcode, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Allocate a temporary local on the stack
	operand.type = OP_LOCAL;
	scope_new(parser);
	local_new(parser, &operand.slot);

	// Emit operation
	emit(parser->fn, instr_new(opcode, operand.slot, right.value, 0));

	// Return a the local in which we stored the result of the operation
	return operand;
}


// Modifies the targets of the jump instructions in a conditional expression
// to update the location of the false case to `false_case`.
void expr_patch_false_case(Parser *parser, Operand operand, int false_case) {
	// Iterate over jump list
	int current = operand.jump;
	while (current != -1) {
		jmp_lazy_target(parser->fn, current, false_case);
		current = jmp_next(parser->fn, current);
	}

	// Point the operand to the false case
	jmp_target(parser->fn, operand.jump, false_case);
}


// Stores the value of an operand into `slot`.
void expr_discharge(Parser *parser, uint16_t slot, Operand operand) {
	if (operand.type == OP_LOCAL) {
		// Copy a local if isn't in a deallocated scope
		if (operand.slot != slot && operand.slot < parser->locals_count) {
			emit(parser->fn, instr_new(MOV_LL, slot, operand.slot, 0));
		}
	} else if (operand.type == OP_JUMP) {
		Function *fn = parser->fn;

		// Emit true case, jump over false case, and false case
		emit(fn, instr_new(MOV_LP, slot, TRUE_TAG, 0));
		emit(fn, instr_new(JMP, 2, 0, 0));
		uint32_t false_case = emit(fn, instr_new(MOV_LP, slot, FALSE_TAG, 0));

		// Finish the condition now that we know the location of the false case
		expr_patch_false_case(parser, operand, false_case);
	} else {
		// Emit a store instruction for the appropriate type
		Opcode opcode = MOV_LL + operand.type;
		emit(parser->fn, instr_new(opcode, slot, operand.value, 0));
	}
}


// Parses an operand into `slot`.
Operand expr_operand(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;
	Operand operand;

	switch (lexer->token) {
	case TOKEN_INTEGER:
		operand.type = OP_INTEGER;
		operand.integer = lexer->value.integer;
		lexer_next(lexer);
		break;

	case TOKEN_NUMBER:
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm, lexer->value.number);
		lexer_next(lexer);
		break;

	case TOKEN_STRING: {
		char *string = lexer_extract_string(lexer->value.identifier);
		operand.type = OP_STRING;
		operand.string = vm_add_string(parser->vm, string);
		lexer_next(lexer);
		break;
	}

	case TOKEN_IDENTIFIER: {
		// Find an existing variable with the given name
		char *name = lexer->value.identifier.start;
		size_t length = lexer->value.identifier.length;
		Variable var = local_capture(parser, name, length);

		if (var.type == VAR_LOCAL) {
			operand.type = OP_LOCAL;
			operand.slot = var.slot;
		} else if (var.type == VAR_UPVALUE) {
			// Store the upvalue into a local slot
			emit(parser->fn, instr_new(MOV_LU, slot, var.slot, 0));
			operand.type = OP_LOCAL;
			operand.slot = slot;
		} else {
			// Undefined
			ERROR("Undefined variable `%.*s` in expression", length, name);
			break;
		}

		lexer_next(lexer);
		break;
	}

	case TOKEN_TRUE:
		operand.type = OP_PRIMITIVE;
		operand.primitive = TRUE_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_FALSE:
		operand.type = OP_PRIMITIVE;
		operand.primitive = FALSE_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_NIL:
		operand.type = OP_PRIMITIVE;
		operand.primitive = NIL_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_OPEN_PARENTHESIS:
		// Skip the opening parenthesis
		lexer_next(lexer);

		// Parse the expression within the parentheses
		operand = expr_prec(parser, slot, 0);

		// Expect a closing parenthesis
		if (lexer->token != TOKEN_CLOSE_PARENTHESIS) {
			UNEXPECTED("Expected `)` to close `(` in expression");
			operand.type = OP_NONE;
			break;
		}
		lexer_next(lexer);
		break;

	case TOKEN_FN:
		// Skip the `fn` token
		lexer_next(lexer);

		// Parse an anonymous function definition
		operand.type = OP_FN;
		operand.fn_index = parse_fn_definition_body(parser, NULL, 0);
		break;

	case TOKEN_NEW:
		parse_struct_instantiation(parser, slot);
		operand.type = OP_LOCAL;
		operand.slot = slot;
		break;

	default:
		UNEXPECTED("Expected operand in expression");
		operand.type = OP_NONE;
		break;
	}

	return operand;
}


// Parses a postfix operator after an operand.
Operand expr_postfix(Parser *parser, Operand operand, uint16_t return_slot) {
	Lexer *lexer = parser->lexer;
	Operand result;
	result.type = OP_NONE;

	if (lexer->token == TOKEN_OPEN_PARENTHESIS) {
		// Function call
		if (operand.type == OP_LOCAL) {
			parse_fn_call_slot(parser, CALL_L, operand.slot, return_slot);
		} else if (operand.type == OP_FN) {
			parse_fn_call_slot(parser, CALL_F, operand.fn_index, return_slot);
		} else {
			ERROR("Attempt to call non-function");
		}

		result.type = OP_LOCAL;
		result.slot = return_slot;
	}

	return result;
}


// Parses the left hand side of a binary operation, including unary operators,
// an operand, and postfix operators.
Operand expr_left(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;

	// Check for unary operators
	Token unary = lexer->token;
	Opcode opcode = unary_opcode(unary);
	if (opcode != NO_OP) {
		// Consume the unary operator
		lexer_next(lexer);

		// Parse another unary operator, or the operand itself
		Operand right = expr_left(parser, slot);

		// Emit the unary operand instruction
		return expr_unary(parser, opcode, right);
	} else {
		// Parse an operand
		Operand operand = expr_operand(parser, slot);

		// Check for multiple postfix operators
		Operand postfix = expr_postfix(parser, operand, slot);
		while (postfix.type != OP_NONE) {
			operand = postfix;
			postfix = expr_postfix(parser, operand, slot);
		}

		return operand;
	}
}


// Parses an expression into `slot`, stopping when we reach a binary operator
// of lower precedence than `limit`.
Operand expr_prec(Parser *parser, uint16_t slot, Precedence limit) {
	Lexer *lexer = parser->lexer;

	// Expect a left hand side operand
	Operand left = expr_left(parser, slot);

	// Parse a binary operator
	while (!vm_has_error(parser->vm) && binary_prec(lexer->token) > limit) {
		// Consume the operator
		Token operator = lexer->token;
		lexer_next(lexer);

		// Emit bytecode for the left operand
		left = expr_binary_left(parser, operator, left);

		// Create a local to use for this level
		uint16_t new_slot;
		scope_new(parser);
		local_new(parser, &new_slot);

		// Parse the right hand side
		Precedence prec = binary_prec(operator);
		Operand right = expr_prec(parser, new_slot, prec);
		scope_free(parser);

		// Emit the binary operator
		left = expr_binary(parser, slot, operator, left, right);
	}

	return left;
}


// Parses an expression into `slot`, returning the value of the expression.
// For some expressions, nothing may need to be stored (ie. expressions
// consisting of only a constant), so `slot` will remain unused.
Operand expr(Parser *parser, uint16_t slot) {
	return expr_prec(parser, slot, PREC_NONE);
}


// Parses an expression, storing the result into `slot`.
void expr_emit(Parser *parser, uint16_t slot) {
	Operand operand = expr(parser, slot);
	expr_discharge(parser, slot, operand);
}


// Returns true if `token` can begin an expression.
bool expr_exists(Token token) {
	return token == TOKEN_IDENTIFIER || token == TOKEN_STRING ||
		token == TOKEN_INTEGER || token == TOKEN_NUMBER ||
		token == TOKEN_TRUE || token == TOKEN_FALSE || token == TOKEN_NIL ||
		token == TOKEN_FN || token == TOKEN_SUB || token == TOKEN_NOT ||
		token == TOKEN_BIT_NOT;
}



//
//  Variable Assignment
//

// Parses an assignment to a new variable (using a `let` token).
void parse_initial_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Consume the `let` token
	lexer_next(lexer);

	// Expect an identifier
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `let`");
	char *name = lexer->value.identifier.start;
	size_t length = lexer->value.identifier.length;
	lexer_next(lexer);

	// Check the variable isn't already defined
	Variable var = local_capture(parser, name, length);
	if (var.type != VAR_UNDEFINED) {
		ERROR("Variable `%.*s` is already defined", length, name);
		return;
	}

	// Expect an equals sign
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Create a new local
	uint16_t slot;
	Local *local = local_new(parser, &slot);
	if (local == NULL) {
		return;
	}

	// Expect an expression
	expr_emit(parser, slot);

	// Save the local's name
	local->name = name;
	local->length = length;
}


// Parses an assignment.
void parse_assignment(Parser *parser, Identifier name) {
	Lexer *lexer = parser->lexer;

	// Consume the assignment operator
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Check the variable is already defined
	Variable var = local_capture(parser, name.start, name.length);
	if (var.type == VAR_LOCAL) {
		// Parse an expression
		expr_emit(parser, var.slot);
	} else if (var.type == VAR_UPVALUE) {
		// Parse an expression into an empty local slot
		scope_new(parser);
		uint16_t slot;
		local_new(parser, &slot);
		expr_emit(parser, slot);
		scope_free(parser);

		// Store the local into an upvalue
		emit(parser->fn, instr_new(MOV_UL, var.slot, slot, 0));
	} else {
		ERROR("Assigning to undefined variable `%.*s`", name.length, name.start);
	}
}



//
//  If Statements
//

// Parses the condition and body of an if or else if statement.
void parse_if_body(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Parse the conditional expression
	Operand condition = expr(parser, parser->locals_count);
	// TODO: Check condition is a jump

	// Expect an opening brace
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after condition in if statement");
	lexer_next(lexer);

	// Parse the block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect a closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close block in if statement");
	lexer_next(lexer);

	// Get the location of the false case
	uint32_t false_case = parser->fn->bytecode_count;

	// If there's another if or else if after this
	if (lexer->token == TOKEN_ELSE_IF || lexer->token == TOKEN_ELSE) {
		// An extra jump is going to be inserted, so the false case is one more
		// instruction after what we think
		false_case++;
	}

	// Set the false case of the condition
	expr_patch_false_case(parser, condition, false_case);
}


// Parses an if statement.
void parse_if(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `if` token
	lexer_next(lexer);

	// Parse if statement
	parse_if_body(parser);

	// Save the first jump of the jump list
	int jump = -1;

	// Parse following else if statements
	while (!vm_has_error(parser->vm) && lexer->token == TOKEN_ELSE_IF) {
		// Insert a jump at the end of the previous if body
		int new_jump = jmp_new(parser->fn);
		if (jump == -1) {
			jump = new_jump;
		} else {

			jmp_append(parser->fn, new_jump, jump);
			jump = new_jump;
		}

		// Skip the else if token
		lexer_next(lexer);

		// Parse the body
		parse_if_body(parser);
	}

	// Check for an else statement
	if (lexer->token == TOKEN_ELSE) {
		// Insert a jump at the end of the previous if body
		int new_jump = jmp_new(parser->fn);
		if (jump == -1) {
			jump = new_jump;
		} else {

			jmp_append(parser->fn, new_jump, jump);
			jump = new_jump;
		}

		// Skip `else` token
		lexer_next(lexer);

		// Parse block
		EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after `else`");
		lexer_next(lexer);
		parse_block(parser, TOKEN_CLOSE_BRACE);
		EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` after else statement block");
		lexer_next(lexer);
	}

	// Patch jumps to after if statement
	jmp_target_all(parser->fn, jump, parser->fn->bytecode_count);
}



//
//  Infinite Loops
//

// Parses an infinite loop.
void parse_infinite_loop(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `loop` token
	lexer_next(lexer);

	// Expect an opening brace
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after `loop`");
	lexer_next(lexer);

	// Add the loop to the parser's linked list
	Loop loop;
	loop.jump = -1;
	loop.outer = parser->loop;
	parser->loop = &loop;

	// Parse the inner block
	uint32_t start = parser->fn->bytecode_count;
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect the closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close body of infinite loop");
	lexer_next(lexer);

	// Remove the loop from the linked list
	parser->loop = loop.outer;

	// Insert a jump statement to return to the start of the loop
	uint32_t offset = parser->fn->bytecode_count - start;
	emit(parser->fn, instr_new(LOOP, offset, 0, 0));

	// Patch break statements to here
	if (loop.jump >= 0) {
		jmp_target_all(parser->fn, loop.jump, parser->fn->bytecode_count);
	}
}



//
//  While Loops
//

// Parses a while loop.
void parse_while(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `while` token
	lexer_next(lexer);

	// Expect an expression
	// TODO: Check condition is a jump
	uint32_t start = parser->fn->bytecode_count;
	Operand condition = expr(parser, parser->locals_count);

	// Add a loop to the linked list
	Loop loop;
	loop.jump = -1;
	loop.outer = parser->loop;
	parser->loop = &loop;

	// Parse the block
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after condition in while loop");
	lexer_next(lexer);
	parse_block(parser, TOKEN_CLOSE_BRACE);
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close while loop block");
	lexer_next(lexer);

	// Remove the loop from the linked list
	parser->loop = loop.outer;

	// Insert a jump statement to return to the start of the loop
	uint32_t offset = parser->fn->bytecode_count - start;
	emit(parser->fn, instr_new(LOOP, offset, 0, 0));

	// Point the condition's false case here
	uint32_t after = parser->fn->bytecode_count;
	expr_patch_false_case(parser, condition, after);

	// Point all break statements here
	if (loop.jump >= 0) {
		jmp_target_all(parser->fn, loop.jump, after);
	}
}


// Parses a break statement.
void parse_break(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `break` token
	lexer_next(lexer);

	// Ensure we're inside a loop
	if (parser->loop == NULL) {
		ERROR("`break` not inside a loop");
		return;
	}

	// Emit a jump instruction
	uint32_t jump = jmp_new(parser->fn);

	// Add it to the loop's jump list
	Loop *loop = parser->loop;
	if (loop->jump == -1) {
		loop->jump = jump;
	} else {
		uint32_t last = jmp_last(parser->fn, loop->jump);
		jmp_append(parser->fn, last, jump);
	}
}



//
//  Function Definitions
//

// Parses a function definition body (starting at the arguments list) for a
// function with the name `name`.
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length) {
	Lexer *lexer = parser->lexer;

	// Expect an opening parenthesis
	if (lexer->token != TOKEN_OPEN_PARENTHESIS) {
		UNEXPECTED("Expected `(` after function name to begin arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Create the new child parser
	uint16_t fn_index;
	Parser child = parser_new(parser);
	child.fn = fn_new(parser->vm, parser->fn->package, &fn_index);
	child.fn->name = name;
	child.fn->length = length;

	// Parse the arguments list into the child parser's locals list
	while (lexer->token == TOKEN_IDENTIFIER) {
		// Save the argument
		Local *local = &child.locals[child.locals_count++];
		local->name = lexer->value.identifier.start;
		local->length = lexer->value.identifier.length;
		local->scope_depth = 0;
		local->upvalue_index = -1;
		lexer_next(lexer);
		child.fn->arity++;

		// Skip a comma
		if (lexer->token == TOKEN_COMMA) {
			lexer_next(lexer);
		}
	}

	// Expect a closing parenthesis
	if (lexer->token != TOKEN_CLOSE_PARENTHESIS) {
		UNEXPECTED("Expected `)` to close function arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Expect an opening brace to begin the function block
	if (lexer->token != TOKEN_OPEN_BRACE) {
		UNEXPECTED("Expected `{` after arguments list to open function block");
		return 0;
	}
	lexer_next(lexer);

	// Parse the function body
	parse_block(&child, TOKEN_CLOSE_BRACE);

	// Emit a return instruction at the end of the body
	emit(child.fn, instr_new(RET, 0, 0, 0));

	// Expect a closing brace
	if (lexer->token != TOKEN_CLOSE_BRACE) {
		UNEXPECTED("Expected `}` to close function block");
		return 0;
	}
	lexer_next(lexer);

	return fn_index;
}


// Parses a function definition.
void parse_fn_definition(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `fn` token
	lexer_next(lexer);

	// Expect an identifier (the name of the function)
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `fn`");
	char *name = lexer->value.identifier.start;
	size_t length = lexer->value.identifier.length;
	lexer_next(lexer);

	// Parse the remainder of the function
	uint16_t fn_index = parse_fn_definition_body(parser, name, length);

	// Create a new local to store the function in
	uint16_t slot;
	Local *local = local_new(parser, &slot);
	if (local == NULL) {
		return;
	}
	local->name = name;
	local->length = length;

	// Emit bytecode to store the function into the created local
	emit(parser->fn, instr_new(MOV_LF, slot, fn_index, 0));
}



//
//  Function Calls
//

// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
		uint16_t return_slot) {
	Lexer *lexer = parser->lexer;

	// Skip the opening parenthesis
	lexer_next(lexer);

	// Create a new scope for the function arguments
	scope_new(parser);

	// Parse function arguments into consecutive local slots
	uint8_t arity = 0;
	while (!vm_has_error(parser->vm) &&
			lexer->token != TOKEN_CLOSE_PARENTHESIS) {
		// Create local for the argument
		uint16_t slot;
		local_new(parser, &slot);

		// Increment the number of arguments we have
		if (arity >= 255) {
			// Since the arity of the function call must be stored in a single
			// byte in the instruction, it cannot be greater than 255
			ERROR("Cannot pass more than 255 arguments to function call");
			return;
		}
		arity++;

		// Expect an expression
		expr_emit(parser, slot);

		// Expect a comma or closing parenthesis
		if (lexer->token == TOKEN_COMMA) {
			lexer_next(lexer);
		} else if (lexer->token != TOKEN_CLOSE_PARENTHESIS) {
			// Unexpected token
			UNEXPECTED("Expected `)` to close arguments list in function call");
			return;
		}
	}

	// Free the scope created for the arguments
	scope_free(parser);

	// Skip the closing parenthesis
	lexer_next(lexer);

	// Call the function
	uint16_t arg_start = (arity == 0) ? 0 : parser->locals_count;
	emit(parser->fn, instr_new_4(call, arity, slot, arg_start, return_slot));
}


// Parses a function call, starting at the opening parenthesis of the arguments
// list. `ident` is the name of the function to call.
void parse_fn_call(Parser *parser, Identifier ident) {
	// Parse the result of the function call into a temporary slot
	uint16_t return_slot;
	scope_new(parser);
	local_new(parser, &return_slot);
	scope_free(parser);

	// Find a local with the name of the function
	char *name = ident.start;
	size_t length = ident.length;
	Variable var = local_capture(parser, name, length);
	if (var.type == VAR_LOCAL) {
		parse_fn_call_slot(parser, CALL_L, var.slot, return_slot);
	} else if (var.type == VAR_UPVALUE) {
		// Store the upvalue into a temporary local
		uint16_t slot;
		scope_new(parser);
		local_new(parser, &slot);
		emit(parser->fn, instr_new(MOV_LU, slot, var.slot, 0));

		// Call the function
		parse_fn_call_slot(parser, CALL_L, slot, return_slot);

		// Free the temporary local
		scope_free(parser);
	} else {
		// Undefined function
		ERROR("Attempt to call undefined function `%.*s`", length, name);
	}
}


// Parses an assignment or function call. Returns false if neither could be
// parsed.
bool parse_call_or_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Check for an identifier
	if (lexer->token != TOKEN_IDENTIFIER) {
		return false;
	}

	// Skip the identifier
	Identifier identifier = lexer->value.identifier;
	lexer_next(lexer);

	// Check the next character
	if (lexer->token == TOKEN_OPEN_PARENTHESIS) {
		// Function call
		parse_fn_call(parser, identifier);
		return true;
	} else if (lexer->token == TOKEN_ASSIGN) {
		// Assignment
		parse_assignment(parser, identifier);
		return true;
	}

	return false;
}



//
//  Function Returns
//

// Parses a return statement.
void parse_return(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `return` token
	lexer_next(lexer);

	// Check for a return value
	if (!expr_exists(lexer->token)) {
		// Emit close upvalue instructions for all locals in this function
		local_close_upvalues(parser);

		// No return value
		emit(parser->fn, instr_new(RET, 0, 0, 0));
	} else {
		// Parse expression into a new local
		uint16_t slot;
		scope_new(parser);
		local_new(parser, &slot);
		Operand operand = expr(parser, slot);
		scope_free(parser);

		// Emit close upvalue instructions for all locals in this function
		local_close_upvalues(parser);

		// Emit return instruction
		Opcode opcode = RET_L + operand.type;
		emit(parser->fn, instr_new(opcode, operand.value, 0, 0));
	}
}



//
//  Structs
//

// Parses a struct definition.
void parse_struct_definition(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `struct` token
	lexer_next(lexer);

	// Expect the name of the struct
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `struct`");
	char *name = lexer->value.identifier.start;
	size_t length = lexer->value.identifier.length;
	lexer_next(lexer);

	// Check the struct doesn't already exist
	if (struct_find(parser->vm, name, length, NULL) != NULL) {
		ERROR("Struct `%.*s` is already defined", length, name);
		return;
	}

	// Create the struct definition
	StructDefinition *def = struct_new(parser->vm);
	def->name = name;
	def->length = length;

	// Check for an optional brace
	if (lexer->token == TOKEN_OPEN_BRACE) {
		// Skip the opening brace
		lexer_next(lexer);

		// Parse struct fields
		while (!vm_has_error(parser->vm) && lexer->token == TOKEN_IDENTIFIER) {
			Identifier *field = struct_new_field(def);
			*field = lexer->value.identifier;
			lexer_next(lexer);

			// Expect a comma or closing brace
			if (lexer->token == TOKEN_COMMA) {
				lexer_next(lexer);
			} else if (lexer->token != TOKEN_CLOSE_BRACE) {
				UNEXPECTED("Expected `}` to close struct fields list");
			}
		}

		// Expect a closing brace
		EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close struct fields list");
		lexer_next(lexer);
	}
}


// Parses a struct instantiation, storing the resulting struct into `slot`.
void parse_struct_instantiation(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;

	// Skip the `new` token
	lexer_next(lexer);

	// Expect the name of a struct
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `new`");
	char *name = lexer->value.identifier.start;
	size_t length = lexer->value.identifier.length;
	lexer_next(lexer);

	// Find a struct with the given name
	uint16_t index;
	StructDefinition *def = struct_find(parser->vm, name, length, &index);
	if (def == NULL) {
		// Struct is undefined
		ERROR("Undefined struct `%.*s` in instantiation", length, name);
		return;
	}

	// Emit bytecode to create the struct
	emit(parser->fn, instr_new(STRUCT_NEW, slot, index, 0));

	// Call the constructor, if it exists
	if (def->constructor != -1) {
		// Create a new temporary slot for the return value
		uint16_t return_slot;
		scope_new(parser);
		local_new(parser, &return_slot);
		scope_free(parser);

		// Call the constructor
		parse_fn_call_slot(parser, CALL_F, def->constructor, return_slot);
	} else {
		// Expect an opening and closing parenthesis
		EXPECT(TOKEN_OPEN_PARENTHESIS,
			"Expected `(` after struct name in instantiation");
		lexer_next(lexer);
		EXPECT(TOKEN_CLOSE_PARENTHESIS,
			"Expected no arguments to struct instantiation, as struct has no"
			"constructor");
		lexer_next(lexer);
	}
}



//
//  Blocks
//

// Parses a single statement.
void parse_statement(Parser *parser) {
	switch (parser->lexer->token) {
		// Trigger a special error for misplaced imports
	case TOKEN_IMPORT:
		ERROR("Imports must be placed at the top of the file");
		break;

	case TOKEN_LET:
		parse_initial_assignment(parser);
		break;

	case TOKEN_IF:
		parse_if(parser);
		break;

	case TOKEN_LOOP:
		parse_infinite_loop(parser);
		break;

	case TOKEN_WHILE:
		parse_while(parser);
		break;

	case TOKEN_BREAK:
		parse_break(parser);
		break;

	case TOKEN_FN:
		parse_fn_definition(parser);
		break;

	case TOKEN_RETURN:
		parse_return(parser);
		break;

	case TOKEN_STRUCT:
		parse_struct_definition(parser);
		break;

	default:
		// Could be a function call or an assignment
		if (parse_call_or_assignment(parser)) {
			break;
		}

		// Unrecognised statement
		UNEXPECTED("Expected statement (eg. `if`, `while`)");
		break;
	}
}


// Parses a block of statements, terminated by `terminator`.
void parse_block(Parser *parser, Token terminator) {
	Lexer *lexer = parser->lexer;

	// Create a new scope for the block
	scope_new(parser);

	// Continually parse statements until an error is triggered, we reach the
	// end of the file, or we reach the terminating token
	while (!vm_has_error(parser->vm) &&
			lexer->token != TOKEN_EOF && lexer->token != terminator) {
		parse_statement(parser);
	}

	// Destroy the scope we created for the block
	scope_free(parser);
}




//
//  Parser
//

// Creates a new parser.
Parser parser_new(Parser *parent) {
	Parser parser;
	parser.parent = parent;
	parser.scope_depth = 0;
	parser.locals_count = 0;
	parser.loop = NULL;
	parser.fn = NULL;

	if (parent != NULL) {
		parser.lexer = parent->lexer;
		parser.vm = parent->vm;
	}

	return parser;
}


// Creates a new function on `vm`, used as `package`'s main function, and
// populates the function's bytecode based on `package`'s source code.
void parse_package(VirtualMachine *vm, Package *package) {
	// Create a lexer on the stack for all child parsers
	Lexer lexer = lexer_new(package->source);
	lexer_next(&lexer);

	// Create a new parser
	Parser parser = parser_new(NULL);
	parser.vm = vm;
	parser.lexer = &lexer;
	parser.fn = fn_new(vm, package, &package->main_fn);
	parser.fn->package = package;

	// Parse import statements at the top of the file
	parse_imports(&parser);

	// Parse the rest of the file
	parse_block(&parser, TOKEN_EOF);

	// Append a return instruction
	emit(parser.fn, instr_new(RET, 0, 0, 0));
}
