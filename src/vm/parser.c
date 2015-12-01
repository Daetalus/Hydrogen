
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


// The maximum number of varaibles that can exist on the left hand side of an
// assignment.
#define MAX_ASSIGNMENT_VARIABLES 16

// The maximum number of locals that can be allocated on the stack at once.
#define MAX_LOCALS 512


// A local variable.
typedef struct {
	// The name of the local.
	char *name;
	size_t length;

	// The scope depth the local was defined at.
	uint32_t scope_depth;
} Local;


// A parser, which converts lexed source code into
// bytecode.
typedef struct _parser {
	// The virtual machine we're parsing for.
	VirtualMachine *vm;

	// A pointer to the parent parser. NULL if this parser
	// is top level.
	struct _parser *parent;

	// The lexer.
	Lexer *lexer;

	// The function we're compiling the source code into.
	Function *fn;

	// The current scope depth.
	uint32_t scope_depth;

	// All defined locals.
	Local locals[MAX_LOCALS];
	uint32_t locals_count;
} Parser;



//
//  Error Handling
//

// Triggers a custom error.
#define ERROR(...) \
	err_fatal(parser->vm, parser->lexer, __VA_ARGS__);

// Triggers an unexpected token error and returns from the
// current function.
#define UNEXPECTED(...) \
	err_unexpected(parser->vm, parser->lexer, __VA_ARGS__);


// Triggers an error if the current token doesn't match the
// given one.
#define EXPECT(expected, ...)                                   \
	if (lexer->token != (expected)) {                           \
		err_unexpected(parser->vm, parser->lexer, __VA_ARGS__); \
		return;                                                 \
	}



//
//  Bytecode
//

// Emits an empty jump instruction. Returns the position
// in the bytecode of the instruction.
uint32_t emit_jump(Parser *parser) {
	return fn_emit(parser->fn, INSTRUCTION(JMP, 0, 0, 0));
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

	// Ensure there's at least one string within the
	// parentheses
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

	// Parse a multi-import statement if the next token is
	// an open parenthesis
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

// Creates a new local variable at the top of the locals
// stack.
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


// Returns a pointer to the local with the given name, or
// NULL if no local with that name could be found.
Local * local_find(Parser *parser, char *name, size_t length, uint16_t *slot) {
	for (int i = parser->locals_count - 1; i >= 0; i--) {
		Local *local = &parser->locals[i];
		if (local->length == length &&
				strncmp(local->name, name, length) == 0) {
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


// Decrements the parser's scope depth, removing all locals
// from the stack created in that scope.
void scope_free(Parser *parser) {
	parser->scope_depth--;

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
	PREC_OR,
	PREC_AND,
	PREC_BIT_OR,
	PREC_BIT_XOR,
	PREC_BIT_AND,
	// Equal, not equal
	PREC_EQ,
	// Less than, less than equal, greater than, greater
	// than equal
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
	OP_CALL,
	OP_FUNCTION,
	OP_NONE,
} OperandType;


// An operand in an expression.
typedef struct {
	// The type of the operand.
	OperandType type;

	// The value of the operand. Numbers and strings are
	// stored as indices into the VM's number/string list.
	union {
		int16_t integer;
		uint16_t number;
		uint16_t string;
		uint16_t primitive;
		uint16_t local;

		uint16_t value;
	};
} Operand;


// Evaluates to true if an operand is a number.
#define IS_NUMBER(type) (type == OP_INTEGER || type == OP_NUMBER)


// Parses an expression, stopping when we reach a binary
// operator of lower precedence than the given precedence.
Operand expr_prec(Parser *parser, Precedence precedence);


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


// Returns the opcode for a binary operator. Either `left`
// or `right` must be a local.
Opcode binary_opcode(Token operator, OperandType left, OperandType right) {
	int opcodes_per_operation = ADD_NL - ADD_LL + 1;
	int start = ADD_LL + (operator - TOKEN_ADD) * opcodes_per_operation;
	return start + (left == OP_LOCAL ? right : left + 2);
}


// Returns true if the given operands are valid for the
// given binary operation.
bool binary_valid(Token operator, OperandType left, OperandType right) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return (IS_NUMBER(left) || left == OP_LOCAL) &&
			(IS_NUMBER(right) || right == OP_LOCAL);
	case TOKEN_CONCAT:
		return (left == OP_STRING || left == OP_LOCAL) &&
			(right == OP_STRING || right == OP_LOCAL);
	default:
		return false;
	}
}


// Returns true if a binary operator is an arithmetic operator.
bool binary_is_arithmetic(Token operator) {
	return operator == TOKEN_ADD || operator == TOKEN_SUB ||
		operator == TOKEN_MUL || operator == TOKEN_DIV ||
		operator == TOKEN_MOD;
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


// Returns true if the given operand is valid for the given
// unary operation.
bool unary_valid(Opcode operator, OperandType operand) {
	switch (operator) {
	case NEG_L:
		return IS_NUMBER(operand) || operand == OP_LOCAL;
	default:
		return false;
	}
}


// Converts an integer or number operand into a double value.
double operand_to_number(Operand operand) {
	if (operand.type == OP_NUMBER) {
		return operand.number;
	} else if (operand.type == OP_INTEGER) {
		return (double) operand.integer;
	} else {
		return 0.0;
	}
}


// Places an operand in the next available local slot.
void expr_discharge(Parser *parser, Operand operand) {
	// Place the variable in the next available local slot
	uint16_t slot = parser->locals_count;

	if (operand.type == OP_LOCAL) {
		// Copy a local if isn't in a deallocated scope
		if (operand.local < parser->locals_count) {
			fn_emit(parser->fn, INSTRUCTION(MOV_LL, slot, operand.local, 0));
		}
	} else {
		// Calculate the instruction to use
		Opcode opcode = MOV_LL + operand.type;
		fn_emit(parser->fn, INSTRUCTION(opcode, slot, operand.value, 0));
	}
}


// Attempts to fold a concatenation operation.
Operand expr_binary_fold_concat(Parser *parser, Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// If both aren't strings, we can't fold
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
Operand expr_binary_fold_arithmetic(Parser *parser, Token operator,
		Operand left, Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// If either isn't a number, we can't fold
	if (!IS_NUMBER(left.type) || !IS_NUMBER(right.type)) {
		return operand;
	}

	// If both are integers and the operation is not division, return an
	// integer
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
		double left_value = operand_to_number(left);
		double right_value = operand_to_number(right);
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm,
			binary_number_arithmetic(operator, left_value, right_value));
	}

	return operand;
}


// Attempts to fold a binary operation. Assumes the
// operands are valid for the operation.
Operand expr_binary_fold(Parser *parser, Token operator, Operand left,
		Operand right) {
	Operand operand;
	operand.type = OP_NONE;

	// Can't fold operation if either left or right are locals
	if (left.type == OP_LOCAL || right.type == OP_LOCAL) {
		return operand;
	}

	// Attempt to fold concatenation
	if (operator == TOKEN_CONCAT) {
		return expr_binary_fold_concat(parser, left, right);
	}

	// Attempt to fold arithmetic
	if (binary_is_arithmetic(operator)) {
		return expr_binary_fold_arithmetic(parser, operator, left, right);
	}

	return operand;
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
	operand = expr_binary_fold(parser, operator, left, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Configure the resulting operand
	operand.type = OP_LOCAL;
	operand.local = slot;

	// Emit the operation
	Opcode opcode = binary_opcode(operator, left.type, right.type);
	fn_emit(parser->fn, INSTRUCTION(opcode, operand.local,
			left.value, right.value));

	// Return a local pointing to the slot we stored the
	// result of the operation into
	return operand;
}


// Attempts to fold a unary operation.
Operand expr_unary_fold(Parser *parser, Opcode opcode, Operand right) {
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
	operand = expr_unary_fold(parser, opcode, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Allocate a temporary local on the stack
	operand.type = OP_LOCAL;
	scope_new(parser);
	local_new(parser, &operand.local);

	// Emit operation
	fn_emit(parser->fn, INSTRUCTION(opcode, operand.local, right.value, 0));

	// Return a local pointing to the slot we stored the
	// result of the operation into
	return operand;
}


// Parses an operand.
Operand expr_operand(Parser *parser) {
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
		uint16_t slot;
		Identifier *ident = &lexer->value.identifier;
		Local *local = local_find(parser, ident->start, ident->length, &slot);

		// Check for an undefined variable
		if (local == NULL) {
			ERROR("Undefined variable `%.*s` in expression", ident->length,
				ident->start);
			break;
		}

		operand.type = OP_LOCAL;
		operand.local = slot;
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
		operand = expr_prec(parser, 0);

		// Expect a closing parenthesis
		if (lexer->token != TOKEN_CLOSE_PARENTHESIS) {
			ERROR("Expected `)` to close `(` in expression");
			operand.type = OP_NONE;
		}

		// Consume the closing parenthesis
		lexer_next(lexer);
		break;

	default:
		UNEXPECTED("Expected operand in expression");
		operand.type = OP_NONE;
		break;
	}

	return operand;
}


// Parses the left hand side of a binary operator.
Operand expr_left(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Check for unary operators
	Token unary = lexer->token;
	Opcode opcode = unary_opcode(unary);
	if (opcode != NO_OP) {
		// Consume the unary operator
		lexer_next(lexer);

		// Parse more unary operators, or the operand
		// itself
		Operand right = expr_left(parser);

		// Emit the unary operand instruction
		return expr_unary(parser, opcode, right);
	} else {
		// Parse an operand
		return expr_operand(parser);
	}
}


// Parses an expression, stopping when we reach a binary
// operator of lower precedence than the given precedence.
Operand expr_prec(Parser *parser, Precedence precedence) {
	Lexer *lexer = parser->lexer;

	// Start a new variable scope depth
	scope_new(parser);

	// Create a local to use for this level
	uint16_t slot;
	local_new(parser, &slot);

	// Expect a left hand side operand
	Operand left = expr_left(parser);

	// Parse a binary operator
	while (!vm_has_error(parser->vm) &&
			binary_prec(lexer->token) > precedence) {
		// Consume the operator
		Token operator = lexer->token;
		lexer_next(lexer);

		// Parse the right hand side
		Precedence prec = binary_prec(operator);
		Operand right = expr_prec(parser, prec);

		// Emit the binary operator
		left = expr_binary(parser, slot, operator, left, right);
	}

	// Discard the variable scope we created
	scope_free(parser);
	return left;
}


// Parses an expression, returning the final result.
Operand expr(Parser *parser) {
	return expr_prec(parser, PREC_NONE);
}


// Parses an expression, placing results into consecutive
// local slots.
void expr_emit(Parser *parser) {
	Operand operand = expr(parser);
	expr_discharge(parser, operand);
}



//
//  Variable Assignment
//

// Parses the comma separated list of variables on the left hand side of an
// assignment. Returns true if at least one of the variables is unique.
bool parse_assignment_left(Parser *parser, Identifier *variables, int *count) {
	Lexer *lexer = parser->lexer;

	bool found_unique = false;
	while (!vm_has_error(parser->vm) && lexer->token == TOKEN_IDENTIFIER) {
		char *name = lexer->value.identifier.start;
		size_t length = lexer->value.identifier.length;

		// Check if the variable is unique
		if (local_find(parser, name, length, NULL) == NULL) {
			found_unique = true;
		}

		// Save the left hand side variable
		variables[(*count)++] = lexer->value.identifier;
		lexer_next(lexer);

		if (lexer->token == TOKEN_COMMA) {
			// Skip the comma
			lexer_next(lexer);
		} else {
			break;
		}
	}

	return found_unique;
}


// Parses the right hand side of an assignment.
void parse_assignment_right(Parser *parser, Identifier *variables, int count) {
	Lexer *lexer = parser->lexer;

	// Expect an equals sign
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Expect an expression
	expr_emit(parser);

	// Create new locals for each variable we're assigning to
	for (int i = 0; i < count; i++) {
		Local *local = local_new(parser, NULL);
		local->name = variables[i].start;
		local->length = variables[i].length;
	}
}


// Parses an assignment to a new variable (using a `let`
// token).
void parse_initial_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Consume the `let` token
	lexer_next(lexer);

	// Expect a comma separated list of identifiers
	Identifier variables[MAX_ASSIGNMENT_VARIABLES];
	int count = 0;

	// Check at least 1 of the variables is unique
	if (!parse_assignment_left(parser, variables, &count)) {
		ERROR("No new variable to assign in `let`");
	}

	// Expect at least 1 variable
	if (count == 0) {
		UNEXPECTED("Expected identifier after `let`");
		return;
	}

	parse_assignment_right(parser, variables, count);
}


// Parses an assignment.
void parse_assignment(Parser *parser, Identifier first_identifier) {
	Lexer *lexer = parser->lexer;

	// Expect a comma separated list of identifiers
	// Since the previous function already consumed the first identifier,
	// get rid of its following comma (if it exists)
	if (lexer->token == TOKEN_COMMA) {
		lexer_next(lexer);
	}

	Identifier variables[MAX_ASSIGNMENT_VARIABLES];
	variables[0] = first_identifier;
	int count = 1;

	// Check there are no unique variables
	if (parse_assignment_left(parser, variables, &count)) {
		ERROR("Assigning to undeclared variable");
	}

	parse_assignment_right(parser, variables, count);
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
		// TODO: ...
	} else {
		// Assignment
		parse_assignment(parser, identifier);
	}

	return true;
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


// Parses a block of statements, terminated by the given
// character.
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

// Parses a package into bytecode. Sets the main function
// index property on the package.
void parse_package(VirtualMachine *vm, Package *package) {
	// Create the lexer used for all parent and child parsers
	Lexer lexer = lexer_new(package->source);
	lexer_next(&lexer);

	// Create a new parser
	Parser parser;
	parser.vm = vm;
	parser.parent = NULL;
	parser.lexer = &lexer;
	parser.fn = fn_new(vm, package, &package->main_fn);
	parser.scope_depth = 0;
	parser.locals_count = 0;

	// Parse the import statements at the top of the file
	parse_imports(&parser);

	// Parse the rest of the file
	parse_block(&parser, TOKEN_EOF);

	// Append a return instruction
	fn_emit(parser.fn, INSTRUCTION(RET0, 0, 0, 0));
}
