
//
//  Parser
//

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "parser.h"
#include "fn.h"
#include "pkg.h"
#include "vm.h"
#include "err.h"


// Returns a pointer to the current function we're emitting bytecode values to.
static inline Function * parser_fn(Parser *parser) {
	return &vec_at(parser->state->functions, parser->scope->fn_index);
}


// Returns a pointer to the package we're parsing.
static inline Package * parser_pkg(Parser *parser) {
	return &vec_at(parser->state->packages, parser->package);
}


// Forward declaration.
static void parse_block(Parser *parser, TokenType terminator);



//
//  Error Handling
//

// Expects a token with type `type` to be the current token on the lexer,
// triggering an error if it is not found. Variable argument version of the
// function below.
static void err_unexpected_vararg(Parser *parser, Token *token, char *fmt,
		va_list args) {
	HyError *err = err_new();

	// Print format string
	err_print_varargs(err, fmt, args);

	// Print found token
	err_print(err, ", found ");
	err_print_token(err, token);

	// Attach token and trigger error
	err_token(parser->state, err, token);
	err_trigger(parser->state, err);
}


// Expects a token with type `type` to be the current token on the lexer,
// triggering an error if it is not found.
static void err_unexpected(Parser *parser, Token *token, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	err_unexpected_vararg(parser, token, fmt, args);
	va_end(args);
}


// Expects a token with type `type` to be the lexer's current token, triggering
// an error on it if this is not the case.
static void err_expect(Parser *parser, TokenType type, Token *token,
		char *fmt, ...) {
	if (parser->lexer.token.type != type) {
		// Token doesn't match expected
		va_list args;
		va_start(args, fmt);
		err_unexpected_vararg(parser, token, fmt, args);
		va_end(args);
	}
}


// Triggers a fatal error on the token `token` with the message `fmt`.
static void err_fatal(Parser *parser, Token *token, char *fmt, ...) {
	HyError *err = err_new();

	// Print format string
	va_list args;
	va_start(args, fmt);
	err_print_varargs(err, fmt, args);
	va_end(args);

	// Attach token and trigger error
	err_token(parser->state, err, token);
	err_trigger(parser->state, err);
}



//
//  Function Scopes
//

// Create a new function scope (including the function on the interpreter).
static FunctionScope scope_new(Parser *parser) {
	FunctionScope scope;
	scope.parent = NULL;
	scope.fn_index = fn_new(parser->state);
	scope.is_method = false;
	scope.first_local = parser->locals_count;
	scope.loop = NULL;
	scope.block_depth = 0;

	Function *fn = &vec_at(parser->state->functions, scope.fn_index);
	fn->package = parser->package;
	fn->source = parser->source;
	fn->line = parser->lexer.line;
	return scope;
}


// Push a function scope on top of the parser's function scope stack.
static void scope_push(Parser *parser, FunctionScope *scope) {
	scope->parent = parser->scope;
	parser->scope = scope;
}


// Pop a function from the parser's function scope stack.
static void scope_pop(Parser *parser) {
	// All blocks and locals should have been freed here, so we're safe to pop
	// the function scope
	parser->scope = parser->scope->parent;
}



//
//  Locals
//

// Reserve space for a new local, returning its index.
static uint16_t local_reserve(Parser *parser) {
	uint16_t new_size = parser->locals_count++;

	// Increment the function's frame size
	uint16_t frame_size = new_size - parser->scope->first_local;
	Function *fn = parser_fn(parser);
	if (frame_size > fn->frame_size) {
		fn->frame_size = frame_size;
	}

	return new_size;
}


// Create a new, named local, returning its index.
static uint16_t local_new(Parser *parser) {
	vec_add(parser->locals);
	Local *local = &vec_last(parser->locals);
	local->name = NULL;
	local->length = 0;
	local->block = parser->scope->block_depth;
	return local_reserve(parser);
}


// Free the uppermost local.
static void local_free(Parser *parser) {
	parser->locals_count--;

	// Check if this was a named local
	if (parser->locals_count < vec_len(parser->locals)) {
		// Decrement the number of named locals
		vec_len(parser->locals)--;
	}
}


// Searches for a local in the parser's current function scope, returning its
// index if found.
static Index local_find(Parser *parser, char *name, uint32_t length) {
	// Search in reverse order
	for (int i = vec_len(parser->locals) - 1; i >= 0; i--) {
		// Break if we've gone outside the function's scope
		if ((uint32_t) i < parser->scope->first_local) {
			break;
		}

		// Check if this local's name matches the given one
		Local *local = &vec_at(parser->locals, i);
		if (length == local->length &&
				strncmp(name, local->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}


// The type of a resolved local.
typedef enum {
	RESOLVED_LOCAL,
	RESOLVED_UPVALUE,
	RESOLVED_TOP_LEVEL,
	RESOLVED_PACKAGE,
	RESOLVED_UNDEFINED,
} ResolvedLocalType;


// Information about a resolved local.
typedef struct {
	// The type of the value the local resolves to.
	ResolvedLocalType type;

	// The index of the local in the array that holds all the values of the
	// local's type.
	Index index;
} ResolvedLocal;


// Resolve a string (the name of a value) into a value.
static ResolvedLocal local_resolve(Parser *parser, char *name,
		uint32_t length) {
	ResolvedLocal resolved;

	// Local variables
	resolved.index = local_find(parser, name, length);
	if (resolved.index != NOT_FOUND) {
		resolved.type = RESOLVED_LOCAL;
		return resolved;
	}

	// TODO: upvalues

	// Top level variables
	Package *pkg = parser_pkg(parser);
	resolved.index = pkg_local_find(pkg, name, length);
	if (resolved.index != NOT_FOUND) {
		resolved.type = RESOLVED_TOP_LEVEL;
		return resolved;
	}

	// Packages
	resolved.index = pkg_find(parser->state, name, length);
	if (resolved.index != NOT_FOUND) {
		resolved.type = RESOLVED_PACKAGE;
		return resolved;
	}

	// Undefined variable
	resolved.type = RESOLVED_UNDEFINED;
	return resolved;
}


// Create a new block scope for named locals.
static void block_new(Parser *parser) {
	// Increase block depth
	parser->scope->block_depth++;
}


// Free a block and all variables defined within it.
static void block_free(Parser *parser) {
	// No temporary locals should be allocated here
	// Free locals inside this block
	if (vec_len(parser->locals) > 0) {
		while (vec_last(parser->locals).block >= parser->scope->block_depth) {
			local_free(parser);
		}
	}

	// Decrement block depth
	parser->scope->block_depth--;
}


// Emit bytecode to move an upvalue into a local slot.
static void upvalue_demote(Parser *parser, uint32_t index, uint16_t local) {
	fn_emit(parser_fn(parser), MOV_LU, local, index, 0);
}


// Emit bytecde to move a top level local into a local slot from an external
// package (not the one we're parsing).
static void top_level_demote_external(Parser *parser, Index pkg,
		uint32_t top_level, uint16_t local) {
	fn_emit(parser_fn(parser), MOV_LT, local, top_level, pkg);
}


// Emit bytecode to move a top level local into a local slot.
static void top_level_demote(Parser *parser, uint32_t index, uint16_t local) {
	top_level_demote_external(parser, parser->package, index, local);
}



//
//  Expressions
//

// The type of an operand in an expression. The ordering is important, because
// they are in the same order as the MOV_L*, MOV_U*, MOV_T*, and STRUCT_SET_*
// bytecode opcodes, so we can simply add an operand's type to the base opcode
// to get the correct one.
typedef enum {
	OP_LOCAL,
	OP_INTEGER,
	OP_NUMBER,
	OP_STRING,
	OP_PRIMITIVE,
	OP_FUNCTION,
	OP_NATIVE,
	OP_JUMP,
	OP_NONE,
} OpType;


// An operand in an expression.
typedef struct {
	// The type of the operand.
	OpType type;

	// Whether this operand has side effects (ie. is an assignment or function
	// call).
	bool has_side_effects;

	// The value of the operand.
	union {
		uint16_t value;
		uint32_t index;

		// If the operand is a jump, then we need to store the index into the
		// bytecode of the jump instruction instead of its value.
		Index jump;
	};
} Operand;


// Forward declaration.
static Operand parse_expr(Parser *parser, uint16_t slot);


// The precedence level of operators, in the proper order.
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


// Returns the precedence of a binary operator.
static Precedence prec_binary(TokenType operator) {
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
	case TOKEN_CONCAT:
		return PREC_CONCAT;
	default:
		return PREC_NONE;
	}
}


// Returns the opcode for an arithmetic operation. Either `left` or `right` must
// be a local.
static inline BytecodeOpcode opcode_arith(TokenType operator, OpType left,
		OpType right) {
	int base = ADD_LL + (operator - TOKEN_ADD) * 5;
	return base + (left == OP_LOCAL ? right : left + 2);
}


// Returns the opcode for a concatenation operation. Either `left` or `right`
// must be a local.
static inline BytecodeOpcode opcode_concat(TokenType operator, OpType left,
		OpType right) {
	int offset = (right == OP_STRING ? 1 : (left == OP_STRING ? 2 : 0));
	return CONCAT_LL + offset;
}


// Returns the opcode for an equality operation. Either `left` or `right` must
// be a local.
static inline BytecodeOpcode opcode_eq(TokenType operator, OpType left,
		OpType right) {
	int base = EQ_LL + (operator - TOKEN_EQ) * 7;
	return base + (left == OP_LOCAL ? right : left);
}


// Returns the opcode for an order operation. Either `left` or `right` must be
// a local.
static inline BytecodeOpcode opcode_ord(TokenType operator, OpType left,
		OpType right) {
	int base = LT_LL + (operator - TOKEN_LT) * 3;
	return base + (left == OP_LOCAL ? right : left);
}


// Returns the opcode for a unary operation. The argument to the unary operator
// must be a local.
static inline BytecodeOpcode opcode_unary(TokenType operator) {
	switch (operator) {
	case TOKEN_SUB: return NEG_L;
	default: return NO_OP;
	}
}


// Create a new operand with type `OP_NONE`.
static inline Operand operand_new(void) {
	Operand operand = operand_new();
	operand.type = OP_NONE;
	operand.has_side_effects = false;
	return operand;
}


// Returns true if a token is a unary operator.
static inline bool is_unary(TokenType operator) {
	return operator == TOKEN_SUB || operator == TOKEN_NOT;
}


// Returns true if a token is a postfix operator.
static inline bool is_postfix(TokenType operator) {
	return operator == TOKEN_DOT || operator == TOKEN_OPEN_PARENTHESIS;
}


// Attempt to fold an arithmetic operation.
static Operand fold_arith(Parser *parser, TokenType operator, Operand left,
		Operand right) {

}


// Attempt to fold a concatenation operation.
static Operand fold_concat(Parser *parser, Operand left, Operand right) {

}


// Attempt to fold an equality operation.
static Operand fold_eq(Parser *parser, TokenType operator, Operand left,
		Operand right) {

}


// Attempt to fold an order operation.
static Operand fold_ord(Parser *parser, TokenType operator, Operand left,
		Operand right) {

}


// Attempt to fold a conditional operation (`and` or `or` operation).
static Operand fold_cond(Parser *parser, TokenType operator, Operand left,
		Operand Right) {

}


// Attempt to fold a binary operation, returning an operand with type `OP_NONE`
// if the operation cannot be folded.
static Operand fold_binary(Parser *parser, TokenType operator, Operand left,
		Operand right) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return fold_arith(parser, operator, left, right);
	case TOKEN_CONCAT:
		return fold_concat(parser, left, right);
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return fold_eq(parser, operator, left, right);
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return fold_ord(parser, operator, left, right);
	case TOKEN_AND:
	case TOKEN_OR:
		return fold_cond(parser, operator, left, right);
	default:
		return operand_new();
	}
}


// Attempt to fold a boolean `not` operation.
static Operand fold_boolean_not(Parser *parser, Operand operand) {

}


// Attempt to fold an arithmetic negation operation.
static Operand fold_neg(Parser *parser, Operand operand) {

}


// Attempt to fold a unary operation, returning an operand with type `OP_NONE`
// if the operation cannot be folded.
static Operand fold_unary(Parser *parser, TokenType operator, Operand operand) {
	switch (operator) {
	case TOKEN_SUB:
		return fold_neg(parser, operand);
	case TOKEN_NOT:
		return fold_boolean_not(parser, operand);
	default:
		return operand_new();
	}
}


// Reduces a jump into a local, keeping all other operands the same.
static Operand expr_reduce(Parser *parser, Operand operand, uint16_t slot,
		BytecodeOpcode opcode, uint16_t arg3) {
	// Only deal with jump operands
	if (operand.type != OP_JUMP) {
		return operand;
	}

	// TODO: Reduce jump to local
}


// Emits bytecode to move an operand of any type into a local, upvalue, top
// level local, or struct field.
static void expr_discharge(Parser *parser, Operand operand, uint16_t slot,
		BytecodeOpcode base, uint16_t arg3) {
	if (operand.type == OP_LOCAL) {
		// Only emit a move local instruction if this isn't a temporary
		// local
		if (operand.value != slot && operand.value < parser->locals_count) {
			fn_emit(parser_fn(parser), base, slot, operand.value, arg3);
		}
	} else if (operand.type == OP_JUMP) {
		// Put the jump instruction into a local
		BytecodeOpcode opcode = base + (MOV_LP - MOV_LL);
		expr_reduce(parser, operand, slot, opcode, arg3);
	} else {
		// The operand is a constant, so move it using the correct opcode
		BytecodeOpcode opcode = base + operand.type;
		fn_emit(parser_fn(parser), opcode, slot, operand.value, arg3);
	}
}


// Emits bytecode to discharge an operand into a local.
static void expr_discharge_local(Parser *parser, Operand operand,
		uint16_t slot) {
	expr_discharge(parser, operand, slot, MOV_LL, 0);
}


// Emits bytecode to discharge an operand into an upvalue.
static void expr_discharge_upvalue(Parser *parser, Operand operand,
		uint16_t upvalue) {
	expr_discharge(parser, operand, upvalue, MOV_UL, 0);
}


// Emits bytecode to discharge an operand into a top level local in an external
// package.
static void expr_discharge_top_level_external(Parser *parser, Operand operand,
		Index package, uint16_t top_level) {
	expr_discharge(parser, operand, top_level, MOV_TL, package);
}


// Emits bytecode to discharge an operand into a top level local in the package
// currently being parsed.
static void expr_discharge_top_level(Parser *parser, Operand operand,
		uint16_t top_level) {
	expr_discharge_top_level_external(parser, operand, parser->package,
		top_level);
}


// Emits bytecode to discharge an operand into a struct field.
static void expr_discharge_field(Parser *parser, Operand operand,
		uint16_t field, uint16_t struct_slot) {
	expr_discharge(parser, operand, field, STRUCT_SET_L, struct_slot);
}


// Emit bytecode to perform a binary operation, storing the result into `slot`.
// Returns the result of the binary operation.
static Operand expr_binary(Parser *parser, uint16_t slot, TokenType operator,
		Operand left, Operand right) {

}


// Emit bytecode for the left operand to a binary operation before the right
// operand is parsed. Used for things like converting the left operand of a
// condition (ie. `and` or `or` statement) into a jump operand (by emitting an
// `IS_TRUE_L` or `IS_FALSE_L`).
static Operand expr_binary_left(Parser *parser, TokenType operator,
		Operand left) {

}


// Emit bytecode to perform a unary operation, storing the result into `slot`.
// Returns the result of the unary operation.
static Operand expr_unary(Parser *parser, uint16_t slot, TokenType operator,
		Operand operand) {

}


// Emit bytecode to perform a postfix operation, like a struct field access,
// function call, or array access. `operand` is the most recently parsed operand
// just before the postfix operator, on which we're performing the postfix
// operation.
static Operand expr_postfix(Parser *parser, uint16_t slot, Operand operand) {
	// TODO: postfix operators
	return operand;
}


// Create an integer operand from the token on the lexer.
static Operand operand_integer(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	Operand operand = operand_new();
	operand.type = OP_INTEGER;
	operand.value = signed_to_unsigned(lexer->token.integer);
	lexer_next(lexer);
	return operand;
}


// Create a number operand from the token on the lexer.
static Operand operand_number(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	HyValue value = num_to_val(lexer->token.number);
	Operand operand = operand_new();
	operand.type = OP_NUMBER;
	operand.value = state_add_constant(parser->state, value);
	lexer_next(lexer);
	return operand;
}


// Create a string operand from the token on the lexer.
static Operand operand_string(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Extract the literal into a new string on the interpreter
	Index index = state_add_string(parser->state, lexer->token.length);
	String *string = vec_at(parser->state->strings, index);
	lexer_extract_string(lexer, &lexer->token, string->contents);

	// Create an operand from it
	Operand operand = operand_new();
	operand.type = OP_STRING;
	operand.value = index;
	lexer_next(lexer);
	return operand;
}


// Returns a primitive operand with a type based off the lexer token `token`.
static Operand operand_primitive(Lexer *lexer) {
	Operand operand = operand_new();
	operand.type = OP_PRIMITIVE;
	operand.value = lexer->token.type - TOKEN_TRUE + TRUE_TAG;
	lexer_next(lexer);
	return operand;
}


// Expects a field access after a package name, creating an operand from the
// top level value that is indexed.
static Operand operand_top_level(Parser *parser, Index package, uint16_t slot) {
	Lexer *lexer = &parser->lexer;

	// Save the name of the package and skip over it
	Token pkg_name = lexer->token;
	lexer_next(lexer);

	// Expect a `.`
	err_expect(parser, TOKEN_DOT, &pkg_name,
		"Expected `.` after package name `%.*s`", pkg_name.length,
		pkg_name.start);
	lexer_next(lexer);

	// Expect an identifier
	err_expect(parser, TOKEN_IDENTIFIER, &pkg_name,
		"Expected identifier after `.` in package field access");

	// Find the index of the field
	Package *pkg = &vec_at(parser->state->packages, package);
	Index field = pkg_local_find(pkg, lexer->token.start, lexer->token.length);
	if (field == NOT_FOUND) {
		// Trigger an undefined field error
		err_fatal(parser, &lexer->token,
			"Undefined field `%.*s` on package `%.*s`", lexer->token.length,
			lexer->token.start, pkg_name.length, pkg_name.start);
	}
	lexer_next(lexer);

	// Move the field on the package into a local
	top_level_demote_external(parser, package, field, slot);

	// Return the operand
	Operand operand = operand_new();
	operand.type = OP_LOCAL;
	operand.value = slot;
	return operand;
}


// Creates an operand from the identifier on the lexer.
static Operand operand_identifier(Parser *parser, uint16_t slot) {
	Lexer *lexer = &parser->lexer;
	char *name = lexer->token.start;
	uint32_t length = lexer->token.length;

	Operand result = operand_new();
	result.type = OP_LOCAL;

	// Resolve the identifier into a value
	ResolvedLocal local = local_resolve(parser, name, length);
	switch (local.type) {
	case RESOLVED_LOCAL:
		// Copy the local into the operand
		result.value = local.index;
		break;

	case RESOLVED_UPVALUE:
		// Move the upvalue into the slot
		upvalue_demote(parser, local.index, slot);
		result.value = slot;
		break;

	case RESOLVED_TOP_LEVEL:
		// Move the top level local into the slot
		top_level_demote(parser, local.index, slot);
		result.value = slot;
		break;

	case RESOLVED_PACKAGE:
		// Expect a field access after a struct name
		// Return so we don't skip another token
		return operand_top_level(parser, local.index, slot);

	default:
		// Undefined variable
		err_fatal(parser, &lexer->token,
			"Undefined variable `%.*s` in expression", length, name);
		break;
	}

	lexer_next(lexer);
	return result;
}


// Parse a subexpression inside paretheses.
static Operand operand_subexpr(Parser *parser, uint16_t slot) {
	Lexer *lexer = &parser->lexer;

	// Save and skip the opening parenthesis
	Token start = lexer->token;
	lexer_next(lexer);

	// Parse an expression
	Operand operand = parse_expr(parser, slot);

	// Expect a closing parenthesis
	if (lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
		err_fatal(parser, &start,
			"Expected `)` to close `(` in expression");
	}

	// Skip the closing parenthesis
	lexer_next(lexer);
	return operand;
}


// Parse an anonymous function definition inside an expression.
static Operand operand_anonymous_fn(Parser *parser) {
	// TODO: anonymous functions in expressions
	Operand operand = operand_new();
	operand.type = OP_FUNCTION;
	operand.value = 0;
	return operand;
}


// Parse a struct instantiation.
static Operand operand_instantiation(Parser *parser, uint16_t slot) {
	// TODO: struct instantiation
	Operand operand = operand_new();
	operand.type = OP_LOCAL;
	operand.value = slot;
	return operand;
}


// Parse an operand to a binary operation, excluding preceding unary operators.
static Operand expr_operand(Parser *parser, uint16_t slot) {
	Lexer *lexer = &parser->lexer;
	switch (lexer->token.type) {
	case TOKEN_INTEGER:
		return operand_integer(parser);
	case TOKEN_NUMBER:
		return operand_number(parser);
	case TOKEN_STRING:
		return operand_string(parser);
	case TOKEN_TRUE:
	case TOKEN_FALSE:
	case TOKEN_NIL:
		return operand_primitive(lexer);
	case TOKEN_IDENTIFIER:
		return operand_identifier(parser, slot);
	case TOKEN_OPEN_PARENTHESIS:
		return operand_subexpr(parser, slot);
	case TOKEN_FN:
		return operand_anonymous_fn(parser);
	case TOKEN_NEW:
		return operand_instantiation(parser, slot);
	default:
		err_unexpected(parser, &lexer->token, "Expected operand in expression");
		return operand_new();
	}
}


// Parse the left operand to a binary operation, including unary operators
// before the content of the operand.
static Operand expr_left(Parser *parser, uint16_t slot) {
	Lexer *lexer = &parser->lexer;

	// Check for unary operator
	if (is_unary(lexer->token.type)) {
		// Save and skip the unary operator
		TokenType operator = lexer->token.type;
		lexer_next(lexer);

		// Parse the operand to the unary operation
		Operand operand = expr_left(parser, slot);

		// Emit bytecode for the unary operation
		return expr_unary(parser, slot, operator, operand);
	} else {
		// No more unary operators, parse an operand
		Operand operand = expr_operand(parser, slot);

		// Iteratively parse postfix operators like struct field access, array
		// indexing, and function calls
		while (is_postfix(lexer->token.type)) {
			operand = expr_postfix(parser, slot, operand);
		}

		return operand;
	}
}


// Parse part of an expression, up until we reach an operator with lower
// predence than `prec`.
static Operand expr_precedence(Parser *parser, uint16_t slot, Precedence prec) {
	Lexer *lexer = &parser->lexer;

	// Expect left operator to binary operation
	Operand left = expr_left(parser, slot);

	// Parse binary operations until we find one with a precedence lower than
	// the limit
	while (prec_binary(lexer->token.type) > prec) {
		// Skip operator
		TokenType operator = lexer->token.type;
		lexer_next(lexer);

		// Emit bytecode for the left operand (like converting it to a jump
		// operand if part of a condition)
		expr_binary_left(parser, operator, left);

		// Parse the right operand to the operation
		uint16_t right_slot = local_reserve(parser);
		Precedence right_prec = prec_binary(operator);
		Operand right = expr_precedence(parser, right_slot, right_prec);
		local_free(parser);

		// Emit the operation, where the result of the operation becomes the new
		// left operand to the next binary operation
		left = expr_binary(parser, slot, operator, left, right);
	}

	return left;
}


// Parse an expression into `slot`, returning the resulting operand. If the
// expression doesn't require any temporary locals (eg. consists of a single
// operand), then no temporary locals may be allocated and `slot` will go
// unused.
static Operand parse_expr(Parser *parser, uint16_t slot) {
	return expr_precedence(parser, slot, PREC_NONE);
}


// Parses an expression into the slot `slot`.
static void expr_emit(Parser *parser, uint16_t slot) {
	Operand operand = parse_expr(parser, slot);
	expr_discharge_local(parser, operand, slot);
}


// Parses an expression into a temporary slot on the top of the stack, returning
// the temporary slot.
static uint16_t expr_emit_temporary(Parser *parser) {
	uint16_t slot = local_reserve(parser);
	expr_emit(parser, slot);
	local_free(parser);
	return slot;
}



//
//  Blocks and Statements
//

// Parses an unconditional block (a new scope for locals).
static void parse_unconditional_block(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Skip the opening brace
	lexer_next(lexer);

	// Parse a block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect a closing brace
	err_expect(parser, TOKEN_CLOSE_BRACE, &lexer->token,
		"Expected `}` to close unconditional block");
	lexer_next(lexer);
}


// Parses a single statement, like an `if` or `while` construct.
static void parse_statement(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	switch (lexer->token.type) {
		// Unconditional block
	case TOKEN_OPEN_BRACE:
		parse_unconditional_block(parser);
		break;

	default: {
		// Parse an expression
		uint16_t slot = local_reserve(parser);
		Token start = lexer->token;
		Operand operand = parse_expr(parser, slot);

		// Lone expressions need side effects
		if (!operand.has_side_effects) {
			err_fatal(parser, &start, "Expression result unused");
		}

		expr_discharge_local(parser, operand, slot);
		local_free(parser);
		break;
	}
	}
}


// Parses a block of statements until we reach the terminating token or the end
// of the file.
static void parse_block(Parser *parser, TokenType terminator) {
	Lexer *lexer = &parser->lexer;

	// Allocate a new block for locals defined in this scope
	block_new(parser);

	// Continually parse statements until we reach the terminator or end of file
	while (lexer->token.type != TOKEN_EOF && lexer->token.type != terminator) {
		parse_statement(parser);
	}

	// Free our allocated block
	block_free(parser);
}



//
//  Parser
//

// Creates a new parser, which will append all functions, packages, etc it needs
// to define to the interpreter `state`.
Parser parser_new(HyState *state, Index pkg) {
	Parser parser;
	parser.state = state;
	parser.package = pkg;
	parser.source = NOT_FOUND;
	vec_new(parser.locals, Local, 8);
	parser.locals_count = 0;
	parser.scope = NULL;
	return parser;
}


// Releases resources allocated by a parser.
void parser_free(Parser *parser) {
	vec_free(parser->locals);
}


// Parses some source code, creating a function for the top level code in the
// source.
Index parser_parse(Parser *parser, Index source) {
	// Create a new lexer from the source code
	parser->source = source;
	parser->lexer = lexer_new(parser->state, parser->package, source);

	// Allocate a new function scope for the top level of the source code
	FunctionScope scope = scope_new(parser);
	scope_push(parser, &scope);

	// Parse the top level source
	parse_block(parser, TOKEN_EOF);

	// Emit a final return instruction
	fn_emit(parser_fn(parser), RET0, 0, 0, 0);

	// Free the scope we pushed
	scope_pop(parser);
	return scope.fn_index;
}
