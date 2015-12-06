
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


// The maximum number of locals that can be allocated on the stack at once.
#define MAX_LOCALS 512

// The maximum number of else ifs that can be placed after an if statement.
#define MAX_ELSE_IFS 64

// The maximum number of break statements inside a loop.
#define MAX_BREAK_STATEMENTS 128


// A local variable.
typedef struct {
	// The name of the local.
	char *name;
	size_t length;

	// The scope depth the local was defined at.
	uint32_t scope_depth;
} Local;


// A loop.
typedef struct loop {
	// The position of all break statements that need to be patched to the end
	// of the loop once it's been compiled.
	uint32_t breaks[MAX_BREAK_STATEMENTS];
	int breaks_count;

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

// Parses a function call, storing the return value into the given slot.
void parse_fn_call_name(Parser *parser, char *name, size_t length,
	uint16_t return_slot);

// Parses a function definition body (starting at the arguments list).
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length);



//
//  Errors
//

// Triggers a custom error.
#define ERROR(...) \
	err_new(&parser->vm->err, lexer_line(parser->lexer), __VA_ARGS__);


// Triggers an unexpected token error.
#define UNEXPECTED(...) \
	err_unexpected(&parser->vm->err, parser->lexer, __VA_ARGS__);


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

// Creates a new local variable at the top of the locals stack.
Local * local_new(Parser *parser, uint16_t *slot) {
	uint16_t index = parser->locals_count++;
	if (slot != NULL) {
		*slot = index;
	}

	Local *local = &parser->locals[index];
	local->name = NULL;
	local->length = 0;
	local->scope_depth = parser->scope_depth;
	return local;
}


// Returns a pointer to the local with the given name, or NULL if no local with
// that name could be found.
Local * local_find(Parser *parser, char *name, size_t length, uint16_t *slot) {
	// Iterate over the locals backwards, as locals are more likely to be used
	// right after they've been defined (maybe)
	for (int i = parser->locals_count - 1; i >= 0; i--) {
		Local *local = &parser->locals[i];

		// Check if this is the correct local
		if (local->length == length && strncmp(local->name, name, length) == 0) {
			// Return the slot the local is in
			if (slot != NULL) {
				*slot = i;
			}

			return local;
		}
	}

	return NULL;
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


// Emits bytecode to convert a local operand into a jump.
Operand operand_to_jump(Parser *parser, Operand operand) {
	Operand result;
	result.type = OP_JUMP;

	// Emit a comparison and empty jump instruction
	emit(parser->fn, instr_new(IS_FALSE_L, 0, operand.slot, 0, 0));
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
	uint32_t last = jmp_last(fn, right.jump);
	jmp_point(fn, last, left.jump);

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
	uint32_t last = jmp_last(fn, right.jump);
	jmp_point(fn, last, left.jump);

	// Invert left's condition
	jmp_invert_condition(fn, left.jump);

	// Point left to after right
	jmp_set_target(fn, left.jump, right.jump + 1);

	// Iterate over left's jump list
	uint32_t current = jmp_next(fn, left.jump);
	while (current != JUMP_LIST_END) {
		// Point to after right by default
		uint32_t target = right.jump + 1;

		// For conditions part of AND statements
		if (jmp_type(fn, current) == JUMP_AND && current != left.jump) {
			// Point to last element in right's jump list
			target = last - 1;
		}

		// Set jump target
		jmp_set_target(fn, current, target);

		// Get next element in jump list
		current = jmp_next(fn, current);
	}

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
		ERROR("Invalid operand to binary operator");
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
		emit(parser->fn, instr_new(opcode, 0, slot, left.value, right.value));
	} else if (operator >= TOKEN_EQ && operator <= TOKEN_GE) {
		// Comparison
		operand.type = OP_JUMP;

		// Emit the comparison and the empty jump instruction following it
		Opcode opcode = comparison_opcode(operator, left.type, right.type);
		emit(parser->fn, instr_new(opcode, 0, left.value, right.value, 0));
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
		ERROR("Invalid operand to unary operator");
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
	emit(parser->fn, instr_new(opcode, 0, operand.slot, right.value, 0));

	// Return a the local in which we stored the result of the operation
	return operand;
}


// Places an operand in the next available local slot.
void expr_discharge(Parser *parser, uint16_t slot, Operand operand) {
	if (operand.type == OP_LOCAL) {
		// Copy a local if isn't in a deallocated scope
		if (operand.slot != slot && operand.slot < parser->locals_count) {
			emit(parser->fn, instr_new(MOV_LL, 0, slot, operand.slot, 0));
		}
	} else if (operand.type == OP_JUMP) {
		Function *fn = parser->fn;

		// Emit true case, jump over false case, and false case
		emit(fn, instr_new(MOV_LP, 0, slot, TRUE_TAG, 0));
		emit(fn, instr_new(JMP, 0, 2, 0, 0));
		uint32_t false_case = emit(fn, instr_new(MOV_LP, 0, slot, FALSE_TAG, 0));

		// Finish the condition now that we know the location of the false case
		jmp_patch(fn, operand.jump, false_case);
	} else {
		// Emit a store instruction for the appropriate type
		Opcode opcode = MOV_LL + operand.type;
		emit(parser->fn, instr_new(opcode, 0, slot, operand.value, 0));
	}
}


// Parses an operand.
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
		uint16_t var;
		char *name = lexer->value.identifier.start;
		size_t length = lexer->value.identifier.length;
		if (local_find(parser, name, length, &var) == NULL) {
			// Variable doesn't exist
			ERROR("Undefined variable `%.*s` in expression", length, name);
			break;
		}

		operand.type = OP_LOCAL;
		operand.slot = var;
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
			ERROR("Expected `)` to close `(` in expression");
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

	default:
		UNEXPECTED("Expected operand in expression");
		operand.type = OP_NONE;
		break;
	}

	return operand;
}


// Parses postfix operators after an operand.
Operand expr_postfix(Parser *parser, Operand operand, uint16_t return_slot) {
	Lexer *lexer = parser->lexer;
	Operand result = operand;

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


// Parses the left hand side of a binary operator.
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

		// Check for postfix operators (eg. function calls)
		return expr_postfix(parser, operand, slot);
	}
}


// Parses an expression, stopping when we reach a binary operator of lower
// precedence than the given precedence.
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


// Parses an expression, returning the final result.
Operand expr(Parser *parser, uint16_t slot) {
	return expr_prec(parser, slot, PREC_NONE);
}


// Returns true if the given token can begin an expression.
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
	if (local_find(parser, name, length, NULL) != NULL) {
		ERROR("Variable `%.*s` is already defined", length, name);
		return;
	}

	// Expect an equals sign
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Create a new local
	uint16_t slot;
	Local *local = local_new(parser, &slot);

	// Expect an expression
	Operand result = expr(parser, slot);
	expr_discharge(parser, slot, result);

	// Save the local's name
	local->name = name;
	local->length = length;
}


// Parses an assignment.
void parse_assignment(Parser *parser, Identifier var) {
	Lexer *lexer = parser->lexer;

	// Consume the assignment operator
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Check the variable is already defined
	uint16_t slot;
	if (local_find(parser, var.start, var.length, &slot) == NULL) {
		ERROR("Assigning to undefined variable `%.*s`", var.length, var.start);
		return;
	}

	// Parse an expression
	Operand result = expr(parser, slot);
	expr_discharge(parser, slot, result);
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
	jmp_patch(parser->fn, condition.jump, false_case);
}


// Parses an if statement.
void parse_if(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `if` token
	lexer_next(lexer);

	// Keep track of all jumps that need to be patched to after the whole if
	// statement
	uint32_t jumps[MAX_ELSE_IFS + 1];
	int jumps_count = 0;

	// Parse if statement
	parse_if_body(parser);

	// Parse following else if statements
	while (!vm_has_error(parser->vm) && lexer->token == TOKEN_ELSE_IF) {
		// Insert a jump at the end of the previous if body
		jumps[jumps_count++] = jmp_new(parser->fn);

		// Skip the else if token
		lexer_next(lexer);

		// Parse the body
		parse_if_body(parser);
	}

	// Check for an else statement
	if (lexer->token == TOKEN_ELSE) {
		// Jump for previous if body
		jumps[jumps_count++] = jmp_new(parser->fn);

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
	uint32_t target = parser->fn->bytecode_count;
	for (int i = 0; i < jumps_count; i++) {
		jmp_set_target(parser->fn, jumps[i], target);
	}
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
	loop.breaks_count = 0;
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
	emit(parser->fn, instr_new(LOOP, 0, offset, 0, 0));

	// Patch break statements to here
	uint32_t after = parser->fn->bytecode_count;
	for (int i = 0; i < loop.breaks_count; i++) {
		jmp_set_target(parser->fn, loop.breaks[i], after);
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
	loop.breaks_count = 0;
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
	emit(parser->fn, instr_new(LOOP, 0, offset, 0, 0));

	// Point the condition's false case here
	uint32_t after = parser->fn->bytecode_count;
	jmp_patch(parser->fn, condition.jump, after);

	// Point all break statements here
	for (int i = 0; i < loop.breaks_count; i++) {
		jmp_set_target(parser->fn, loop.breaks[i], after);
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

	// Emit a jump instruction and add it to the innermost loop's break
	// statements list
	parser->loop->breaks[parser->loop->breaks_count++] = jmp_new(parser->fn);
}



//
//  Function Definitions
//

// Parses a function definition body (starting at the arguments list).
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length) {
	Lexer *lexer = parser->lexer;

	// Expect an opening parenthesis
	if (lexer->token != TOKEN_OPEN_PARENTHESIS) {
		ERROR("Expected `(` after function name to begin arguments list");
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
		lexer_next(lexer);
		child.fn->arity++;

		// Skip a comma
		if (lexer->token == TOKEN_COMMA) {
			lexer_next(lexer);
		}
	}

	// Expect a closing parenthesis
	if (lexer->token != TOKEN_CLOSE_PARENTHESIS) {
		ERROR("Expected `)` to close function arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Expect an opening brace to begin the function block
	if (lexer->token != TOKEN_OPEN_BRACE) {
		ERROR("Expected `{` after arguments list to open function block");
		return 0;
	}
	lexer_next(lexer);

	// Parse the function body
	parse_block(&child, TOKEN_CLOSE_BRACE);

	// Emit a return statement at the end of the body
	emit(child.fn, instr_new(RET0, 0, 0, 0, 0));

	// Expect a closing brace
	if (lexer->token != TOKEN_CLOSE_BRACE) {
		ERROR("Expected `}` to close function block");
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
	local->name = name;
	local->length = length;

	// Emit bytecode to store the function into the created local
	emit(parser->fn, instr_new(MOV_LF, 0, slot, fn_index, 0));
}



//
//  Function Calls
//

// Parses a function call to the function in `slot`, storing the return value in
// `return_slot`.
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
			ERROR("Cannot pass more than 255 arguments to function call");
			return;
		}
		arity++;

		// Expect an expression
		Operand arg = expr(parser, slot);
		expr_discharge(parser, slot, arg);

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
	uint16_t arg_start = (arity == 0) ? 0 : parser->locals_count;

	// Skip the closing parenthesis
	lexer_next(lexer);

	// Call the function
	emit(parser->fn, instr_new(call, arity, slot, arg_start, return_slot));
}


// Parses a function call, storing the return value into the given slot.
void parse_fn_call_name(Parser *parser, char *name, size_t length,
		uint16_t return_slot) {
	Lexer *lexer = parser->lexer;

	// Find a local with the given name
	uint16_t fn_index;
	if (local_find(parser, name, length, &fn_index) == NULL) {
		// Undefined function
		ERROR("Attempt to call undefined function `%.*s`", length, name);
	}

	parse_fn_call_slot(parser, CALL_L, fn_index, return_slot);
}


// Parses a function call.
void parse_fn_call(Parser *parser, Identifier name) {
	// Parse a function call into a temporary slot
	uint16_t slot;
	scope_new(parser);
	local_new(parser, &slot);
	parse_fn_call_name(parser, name.start, name.length, slot);
	scope_free(parser);
}


// Parses an assignment or function call. Returns true if it was able to parse
// either.
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
	} else if (lexer->token == TOKEN_ASSIGN || lexer->token == TOKEN_COMMA) {
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
		// No return value
		emit(parser->fn, instr_new(RET0, 0, 0, 0, 0));
	} else {
		// Parse expression into a new local
		uint16_t slot;
		scope_new(parser);
		local_new(parser, &slot);
		Operand operand = expr(parser, slot);
		expr_discharge(parser, slot, operand);
		scope_free(parser);

		// Emit return instruction
		emit(parser->fn, instr_new(RET1, 0, slot, 0, 0));
	}
}



//
//  Statements
//

// Parses a single statement.
void parse_statement(Parser *parser) {
	Lexer *lexer = parser->lexer;

	switch (lexer->token) {
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


// Parses a block of statements, terminated by the given character.
void parse_block(Parser *parser, Token terminator) {
	Lexer *lexer = parser->lexer;

	// Continually parse statements
	while (!vm_has_error(parser->vm) && lexer->token != TOKEN_EOF &&
			lexer->token != terminator) {
		parse_statement(parser);
	}
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


// Parses a package into bytecode. Sets the main function index on the package.
void parse_package(VirtualMachine *vm, Package *package) {
	// Create the lexer used for all parent and child parsers
	Lexer lexer = lexer_new(package->source);
	lexer_next(&lexer);

	// Create a new parser
	Parser parser = parser_new(NULL);
	parser.vm = vm;
	parser.lexer = &lexer;
	parser.fn = fn_new(vm, package, &package->main_fn);
	parser.fn->arity = 0;
	parser.fn->package = package;

	// Parse the import statements at the top of the file
	parse_imports(&parser);

	// Parse the rest of the file
	parse_block(&parser, TOKEN_EOF);

	// Append a return instruction
	emit(parser.fn, instr_new(RET0, 0, 0, 0, 0));
}
