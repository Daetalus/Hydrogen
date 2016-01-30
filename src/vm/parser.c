
//
//  Parser
//

#include <math.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "parser.h"
#include "fn.h"
#include "jmp.h"
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


// Returns true if a name is unique enough to be used in a `let` statement (can
// override locals outside the function scope and top level variables).
static bool local_is_unique(Parser *parser, char *name, uint32_t length) {
	return !(
		// Locals in current scope
		local_find(parser, name, length) != NOT_FOUND ||
		// Top level variables if in outermost scope
		(parser->scope->parent == NULL &&
		 	pkg_local_find(parser_pkg(parser), name, length) != NOT_FOUND)
	);
}



//
//  Blocks
//

// Create a new block scope for named locals.
static void block_new(Parser *parser) {
	// Increase block depth
	parser->scope->block_depth++;
}


// Free a block and all variables defined within it.
static void block_free(Parser *parser) {
	// No temporary locals should be allocated here
	// Free locals inside this block
	while (vec_len(parser->locals) > 0 &&
			vec_last(parser->locals).block >= parser->scope->block_depth) {
		local_free(parser);
	}

	// Decrement block depth
	parser->scope->block_depth--;
}


// Emit bytecode to move an upvalue into a local slot.
static void upvalue_demote(Parser *parser, uint16_t local, uint32_t index) {
	fn_emit(parser_fn(parser), MOV_LU, local, index, 0);
}


// Emit bytecde to move a top level local into a local slot from an external
// package (not the one we're parsing).
static void top_level_demote_external(Parser *parser, uint16_t local,
		uint32_t top_level, Index pkg) {
	fn_emit(parser_fn(parser), MOV_LT, local, top_level, pkg);
}


// Emit bytecode to move a top level local into a local slot.
static void top_level_demote(Parser *parser, uint16_t local,
		uint32_t top_level) {
	top_level_demote_external(parser, local, top_level, parser->package);
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

	// Whether this operand has side effects, like a function call.
	bool has_side_effects;

	// The value of the operand.
	union {
		uint16_t value;

		// If the operand is a jump, then we need to store the index into the
		// bytecode of the jump instruction instead of its value.
		Index jump;
	};
} Operand;


// Forward declaration.
static Operand parse_expr(Parser *parser, uint16_t slot);
static void expr_emit(Parser *parser, uint16_t slot);


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
static inline BytecodeOpcode opcode_concat(OpType left, OpType right) {
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


// Create a new operand with type `OP_NONE`.
static inline Operand operand_new(void) {
	Operand operand;
	operand.type = OP_NONE;
	operand.has_side_effects = false;
	return operand;
}


// Returns true if an operand is a number.
static inline bool operand_is_number(Operand *operand) {
	return operand->type == OP_NUMBER || operand->type == OP_INTEGER;
}


// Returns true if an operand is a local or jump.
static inline bool operand_is_jump_local(Operand *operand) {
	return operand->type == OP_LOCAL || operand->type == OP_JUMP;
}


// Converts a number operand (integer or number) into its double value.
static inline double operand_to_number(Parser *parser, Operand *operand) {
	if (operand->type == OP_NUMBER) {
		return val_to_num(vec_at(parser->state->constants, operand->value));
	} else if (operand->type == OP_INTEGER) {
		return (double) unsigned_to_signed(operand->value);
	} else {
		// Shouldn't happen
		return 0;
	}
}


// Converts a string operand into its underlying `char *` value.
static inline char * operand_to_string(Parser *parser, Operand *operand) {
	return &(vec_at(parser->state->strings, operand->value)->contents[0]);
}


// Converts an operand into a boolean.
static inline bool operand_to_bool(Operand *operand) {
	return operand->type != OP_PRIMITIVE || operand->value == TAG_TRUE;
}


// Converts an operand into a jump condition, emitting bytecode for this.
static void operand_to_jump(Parser *parser, Operand *operand) {
	Function *fn = parser_fn(parser);

	// Emit comparison
	fn_emit(fn, IS_FALSE_L, operand->value, 0, 0);

	operand->type = OP_JUMP;
	operand->jump = fn_emit(fn, JMP, 0, 0, 0);
}


// Returns the inverted operator for a comparison operation.
static TokenType operator_invert_comparison(TokenType operator) {
	switch (operator) {
	case TOKEN_EQ:
		return TOKEN_NEQ;
	case TOKEN_NEQ:
		return TOKEN_EQ;
	case TOKEN_LT:
		return TOKEN_GE;
	case TOKEN_LE:
		return TOKEN_GT;
	case TOKEN_GT:
		return TOKEN_LE;
	case TOKEN_GE:
		return TOKEN_LT;
	default:
		// Shouldn't happen
		return TOKEN_UNRECOGNISED;
	}
}


// Returns true if a token is a unary operator.
static inline bool operator_is_unary(TokenType operator) {
	return operator == TOKEN_SUB || operator == TOKEN_NOT;
}


// Computes the result of an integer fold.
static int32_t arith_integer(TokenType operator, int32_t left,
		int32_t right) {
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
		return left % right;
	default:
		// Shouldn't happen
		return 0;
	}
}


// Computes the result of a number fold.
static double arith_number(TokenType operator, double left,
		double right) {
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
		// Shouldn't happen
		return 0.0;
	}
}


// Attempt to fold an arithmetic operation on two integers.
static bool fold_arith_integers(Parser *parser, TokenType operator,
		Operand *left, Operand right) {
	// Extract integer values as 32 bit signed integers
	int32_t left_value = unsigned_to_signed(left->value);
	int32_t right_value = unsigned_to_signed(right.value);

	// If we're performing a division which results in a fractional answer, then
	// we can't fold this as integers
	if (operator == TOKEN_DIV && left_value % right_value != 0) {
		return false;
	}

	// Compute the integer result as a 32 bit integer in case it exceeds the
	// bounds of a 16 bit integer
	int32_t result = arith_integer(operator, left_value, right_value);

	// If the result exceeds the bounds of a signed 16 bit
	if (result > SHRT_MAX || result < SHRT_MIN) {
		// Store the result as a double
		HyValue value = num_to_val((double) result);
		left->type = OP_NUMBER;
		left->value = state_add_constant(parser->state, value);
	} else {
		// Store the result as an integer
		left->type = OP_INTEGER;
		left->value = signed_to_unsigned((int16_t) result);
	}

	return true;
}


// Attempt to fold an arithmetic operation.
static bool fold_arith(Parser *parser, TokenType operator, Operand *left,
		Operand right) {
	// Attempt to fold operation as integers
	if (left->type == OP_INTEGER && right.type == OP_INTEGER &&
			fold_arith_integers(parser, operator, left, right)) {
		return true;
	}

	// Only fold if both are numbers
	if (!operand_is_number(left) || !operand_is_number(&right)) {
		return false;
	}

	// Extract values and compute result
	double left_value = operand_to_number(parser, left);
	double right_value = operand_to_number(parser, &right);
	double result = arith_number(operator, left_value, right_value);

	// Set resulting operand
	left->type = OP_NUMBER;
	left->value = state_add_constant(parser->state, num_to_val(result));
	return true;
}


// Attempt to fold a concatenation operation.
static bool fold_concat(Parser *parser, Operand *left, Operand right) {
	// Only fold if left and right are strings
	if (left->type != OP_STRING || right.type != OP_STRING) {
		return false;
	}

	// Extract string values
	String *left_str = vec_at(parser->state->strings, left->value);
	String *right_str = vec_at(parser->state->strings, right.value);

	// Concatenate strings into result
	uint32_t length = left_str->length + right_str->length;
	Index index = state_add_string(parser->state, length);
	String *result = vec_at(parser->state->strings, index);
	strncpy(&result->contents[0], left_str->contents, left_str->length);
	strncpy(&result->contents[left_str->length], right_str->contents,
		right_str->length);
	result->contents[length] = '\0';

	// Add result as a string to the interpreter state
	left->type = OP_STRING;
	left->value = index;
	return true;
}


// Attempt to fold an equality operation.
static bool fold_eq(Parser *parser, TokenType operator, Operand *left,
		Operand right) {
	// Only fold if the types are equal
	if (left->type != right.type) {
		return false;
	} else if (left->type == OP_JUMP) {
		// Don't fold jump operands
		return false;
	}

	// If their values are equal (used for everything but numbers and strings)
	if (left->value == right.value) {
		left->type = OP_PRIMITIVE;
		left->value = (operator == TOKEN_EQ) ? TAG_TRUE : TAG_FALSE;
		return true;
	} else if (left->type == OP_LOCAL) {
		// Don't fold locals that have different values
		return false;
	}

	// Try special tests for numbers and strings
	bool result = false;
	if (left->type == OP_NUMBER) {
		result = operand_to_number(parser, left) ==
			operand_to_number(parser, &right);
	} else if (left->type == OP_STRING) {
		result = strcmp(operand_to_string(parser, left),
			operand_to_string(parser, &right)) == 0;
	}

	// Invert the result if we're comparing inequality
	if (operator == TOKEN_NEQ) {
		result = !result;
	}

	// Set the resulting operand
	left->type = OP_PRIMITIVE;
	left->value = result ? TAG_TRUE : TAG_FALSE;
	return true;
}


// Compute the result of an order operation on two numbers (integers or
// doubles). Use a define for this so we can avoid having to write two
// identical functions, one for integers and one for doubles.
#define ord_number(result, operator, left, right) \
	switch (operator) {                           \
	case TOKEN_LT:                                \
		result = (left) < (right);                \
		break;                                    \
	case TOKEN_LE:                                \
		result = (left) <= (right);               \
		break;                                    \
	case TOKEN_GT:                                \
		result = (left) > (right);                \
		break;                                    \
	case TOKEN_GE:                                \
		result = (left) >= (right);               \
		break;                                    \
	default:                                      \
		return false;                             \
	}


// Attempt to fold an order operation.
static bool fold_ord(Parser *parser, TokenType operator, Operand *left,
		Operand right) {
	bool result = false;

	// If we're comparing two identical locals (eg. `a < a`)
	if (left->type == OP_LOCAL && right.type == OP_LOCAL &&
			left->value == right.value) {
		result = operator == TOKEN_GE || operator == TOKEN_LE;
	} else if (left->type == OP_INTEGER && right.type == OP_INTEGER) {
		// Comparing two integers
		int16_t left_value = unsigned_to_signed(left->value);
		int16_t right_value = unsigned_to_signed(right.value);
		ord_number(result, operator, left_value, right_value);
	} else if (operand_is_number(left) && operand_is_number(&right)) {
		// Comparing two numbers
		double left_value = operand_to_number(parser, left);
		double right_value = operand_to_number(parser, &right);
		ord_number(result, operator, left_value, right_value);
	} else {
		// Can't fold
		return false;
	}

	// Set the resulting operand
	left->type = OP_PRIMITIVE;
	left->value = result ? TAG_TRUE : TAG_FALSE;
	return true;
}


// Fold two operands given that both are non-locals.
static void cond_non_locals(TokenType operator, Operand *left, Operand right) {
	// Convert each operand into a boolean
	bool left_bool = operand_to_bool(left);
	bool right_bool = operand_to_bool(&right);

	// Compute a result based on their boolean values
	bool result = (operator == TOKEN_AND) ?
		(left_bool && right_bool) :
		(left_bool || right_bool);
	left->type = OP_PRIMITIVE;
	left->value = result ? TAG_TRUE : TAG_FALSE;
}


// Fold a conditional operation where one of the two operands is a local.
static void cond_single_local(TokenType operator, Operand *result,
		Operand *local, Operand *constant) {
	// Convert the constant into a boolean
	bool constant_bool = operand_to_bool(constant);

	// Depending on the operator
	if (operator == TOKEN_AND) {
		// `and` condition
		if (constant_bool) {
			// <value> && true == <value>
			*result = *local;
		} else {
			// <value> && false == false
			result->type = OP_PRIMITIVE;
			result->value = TAG_FALSE;
		}
	} else {
		// `or` condition
		if (constant_bool) {
			// <value> || true == true
			result->type = OP_PRIMITIVE;
			result->value = TAG_TRUE;
		} else {
			// <value> || false == <value>
			*result = *local;
		}
	}
}


// Attempt to fold a conditional operation (`and` or `or` operation).
static bool fold_cond(Parser *parser, TokenType operator, Operand *left,
		Operand right) {
	if (!operand_is_jump_local(left) && !operand_is_jump_local(&right)) {
		// Neither operand is a local (or jump)
		cond_non_locals(operator, left, right);
	} else if (operand_is_jump_local(left) && !operand_is_jump_local(&right)) {
		// Left operand is local
		cond_single_local(operator, left, left, &right);
	} else if (!operand_is_jump_local(left) && operand_is_jump_local(&right)) {
		// Right operand is local
		cond_single_local(operator, left, &right, left);
	} else {
		// Can't fold (both are locals)
		return false;
	}
	return true;
}


// Attempt to fold a binary operation, returning an operand with type `OP_NONE`
// if the operation cannot be folded.
static bool fold_binary(Parser *parser, TokenType operator, Operand *left,
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
		return false;
	}
}


// Attempt to fold a boolean `not` operation.
static bool fold_boolean_not(Parser *parser, Operand *operand) {
	// TODO
	return false;
}


// Attempt to fold an arithmetic negation operation.
static bool fold_neg(Parser *parser, Operand *operand) {
	if (operand->type == OP_NUMBER) {
		// Fetch the current double value
		HyValue raw = vec_at(parser->state->constants, operand->value);
		double value = val_to_num(raw);

		// Negate the double value and add it as a new constant
		operand->value = state_add_constant(parser->state, num_to_val(-value));
	} else if (operand->type == OP_INTEGER) {
		// Negate the value stored directly in the operand
		int16_t value = -(unsigned_to_signed(operand->value));
		operand->value = signed_to_unsigned(value);
	} else {
		// Can only fold numbers
		return false;
	}
	return true;
}


// Attempt to fold a unary operation, returning an operand with type `OP_NONE`
// if the operation cannot be folded.
static bool fold_unary(Parser *parser, TokenType operator, Operand *operand) {
	switch (operator) {
	case TOKEN_SUB:
		return fold_neg(parser, operand);
	case TOKEN_NOT:
		return fold_boolean_not(parser, operand);
	default:
		return false;
	}
}


// Reduces a jump into a local, top level, upvalue, or struct field, keeping all
// other operands the same.
static void expr_reduce(Parser *parser, Operand *operand, uint16_t slot,
		BytecodeOpcode opcode, uint16_t arg3) {
	// Only deal with jump operands
	if (operand->type != OP_JUMP) {
		return;
	}

	// Emit true case, then jump over false case, then false case
	Function *fn = parser_fn(parser);
	fn_emit(fn, opcode, slot, TAG_TRUE, 0);
	fn_emit(fn, JMP, 2, 0, 0);
	Index false_case = fn_emit(fn, opcode, slot, TAG_FALSE, 0);

	// Patch false case of jump operand to the emitted false case
	jmp_false_case(fn, operand->jump, false_case);
}


// Reduces a jump into a local.
static void expr_reduce_local(Parser *parser, Operand *operand, uint16_t slot) {
	expr_reduce(parser, operand, slot, MOV_LP, 0);
}


// Emits bytecode to move an operand of any type into a local, upvalue, top
// level local, or struct field.
static void expr_discharge(Parser *parser, Operand operand, uint16_t slot,
		BytecodeOpcode base, uint16_t arg3) {
	if (operand.type == OP_LOCAL) {
		// Only emit a move local instruction if this isn't a temporary
		// local
		if (base != MOV_LL || (operand.value != slot &&
				operand.value < parser->locals_count)) {
			fn_emit(parser_fn(parser), base, slot, operand.value, arg3);
		}
	} else if (operand.type == OP_JUMP) {
		// Put the jump instruction into a local
		BytecodeOpcode opcode = base + (MOV_LP - MOV_LL);
		expr_reduce(parser, &operand, slot, opcode, arg3);
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
		uint16_t struct_slot, uint16_t field) {
	expr_discharge(parser, operand, field, STRUCT_SET_L, struct_slot);
}


// Returns true if the operand passed to the binary operator is valid (ie. a
// meaningful result can be computed).
static bool binary_is_valid(TokenType operator, OpType op) {
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
	case TOKEN_BIT_AND:
	case TOKEN_BIT_OR:
	case TOKEN_BIT_XOR:
		// Number or local
		return op == OP_LOCAL || op == OP_NUMBER || op == OP_INTEGER;
	case TOKEN_CONCAT:
		// String or local
		return op == OP_LOCAL || op == OP_STRING;
	case TOKEN_EQ:
	case TOKEN_NEQ:
	case TOKEN_AND:
	case TOKEN_OR:
		// Anything
		return true;
	default:
		// Invalid operator (shouldn't happen)
		return false;
	}
}


// Emit bytecode for a binary arithmetic operation.
static void binary_arith(Parser *parser, uint16_t slot, TokenType operator,
		Operand *left, Operand right) {
	// Emit the operation
	BytecodeOpcode opcode = opcode_arith(operator, left->type, right.type);
	fn_emit(parser_fn(parser), opcode, slot, left->value, right.value);

	// The result of the operation is the local we stored the arithmetic
	// operation into
	left->type = OP_LOCAL;
	left->value = slot;
}


// Emit bytecode for a concatenation operation.
static void binary_concat(Parser *parser, uint16_t slot, Operand *left,
		Operand right) {
	// Emit the operation
	BytecodeOpcode opcode = opcode_concat(left->type, right.type);
	fn_emit(parser_fn(parser), opcode, slot, left->value, right.value);

	// The result of the operation is the local we stored the arithmetic
	// operation into
	left->type = OP_LOCAL;
	left->value = slot;
}


// Emit bytecode for a comparison operation (equality or order).
static void binary_comp(Parser *parser, uint16_t slot, TokenType operator,
		Operand *left, Operand right) {
	// Invert the operator, since we want to trigger the following jump only
	// if the condition is false (since the jump shifts execution to the false
	// case)
	operator = operator_invert_comparison(operator);

	// The value for the left and right locals
	uint16_t left_value = left->value;
	uint16_t right_value = right.value;

	// If we're comparing a local and non-local, and the local is the right
	// operand
	if (left->type != OP_LOCAL && right.type == OP_LOCAL) {
		// We need to swap the left and right operands (since the bytecode
		// opcodes for comparison insist on having the local in the left hand
		// operand)

		// Since equality operations are independent of the order of their
		// operand (unlike ordering), only invert the opcode if this is an
		// order operation
		if (operator != TOKEN_EQ && operator != TOKEN_NEQ) {
			operator = operator_invert_comparison(operator);
		}

		// Swap the arguments to the instruction
		left_value = right.value;
		right_value = left->value;
	}

	// Get the opcode
	BytecodeOpcode opcode;
	if (operator == TOKEN_EQ || operator == TOKEN_NEQ) {
		opcode = opcode_eq(operator, left->type, right.type);
	} else {
		opcode = opcode_ord(operator, left->type, right.type);
	}

	// Emit the comparison and the empty jump instruction following it
	fn_emit(parser_fn(parser), opcode, left_value, right_value, 0);
	left->type = OP_JUMP;
	left->jump = fn_emit(parser_fn(parser), JMP, 0, 0, 0);
}


// Emit bytecode for an `and` operation.
static void binary_and(Parser *parser, uint16_t slot, Operand *left,
		Operand right) {
	// Convert the right operand into a jump condition (the left operand was
	// done by a call to `expr_binary_left`)
	if (right.type == OP_LOCAL) {
		operand_to_jump(parser, &right);
	}

	Function *fn = parser_fn(parser);

	// Join the end of right's jump list to left
	jmp_append(fn, right.jump, left->jump);

	// Associate the left and right jumps with the `and` operation
	jmp_set_type(fn, left->jump, JMP_AND);
	jmp_set_type(fn, right.jump, JMP_AND);

	// Let the operation evaluate to the right operand (since the left is
	// joined to it by the jump list)
	*left = right;
}


// Emit bytecode for an `or` operation.
static void binary_or(Parser *parser, uint16_t slot, Operand *left,
		Operand right) {
	// Convert the right operand into a jump.
	if (right.type == OP_LOCAL) {
		operand_to_jump(parser, &right);
	}

	Function *fn = parser_fn(parser);

	// Join the end of right's jump list to left
	Index last = jmp_last(fn, right.jump);
	jmp_append(fn, right.jump, left->jump);

	// Invert left's condition
	jmp_invert_condition(fn, left->jump);

	// Iterate over left's jump list
	Index current = left->jump;
	while (current != NOT_FOUND) {
		if (jmp_type(fn, current) == JMP_AND) {
			// Point to last element in right's jump list (subtract 1 to point
			// to condition before jump instruction)
			jmp_target(fn, current, last - 1);
		} else {
			// Point to after right's jump list
			jmp_target(fn, current, right.jump + 1);
		}
		current = jmp_next(fn, current);
	}

	// Point left to after right
	jmp_target(fn, left->jump, right.jump + 1);

	// Associate both operands with an `or` operation
	jmp_set_type(fn, left->jump, JMP_OR);
	jmp_set_type(fn, right.jump, JMP_OR);

	// Return right operand
	*left = right;
}


// Emit bytecode for a binary operation, assuming the operands are of a valid
// type and no folding is possible.
static void binary_emit(Parser *parser, uint16_t slot, TokenType operator,
		Operand *left, Operand right) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return binary_arith(parser, slot, operator, left, right);
	case TOKEN_CONCAT:
		return binary_concat(parser, slot, left, right);
	case TOKEN_EQ:
	case TOKEN_NEQ:
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return binary_comp(parser, slot, operator, left, right);
	case TOKEN_AND:
		return binary_and(parser, slot, left, right);
	case TOKEN_OR:
		return binary_or(parser, slot, left, right);
	default:
		// Invalid operator (shouldn't happen)
		return;
	}
}


// Emit bytecode to perform a binary operation, storing the result into `slot`.
// Returns the result of the binary operation.
static void expr_binary(Parser *parser, uint16_t slot, Token *op, Operand *left,
		Operand right) {
	// Ensure the operands to the operator are of a valid type
	if (!binary_is_valid(op->type, left->type) ||
			!binary_is_valid(op->type, right.type)) {
		// Trigger an invalid operand error
		err_unexpected(parser, op, "Invalid operand to binary operator `%.*s`",
			op->length, op->start);
		return;
	}

	// Attempt to fold the binary operation
	if (fold_binary(parser, op->type, left, right)) {
		return;
	}

	// Emit bytecode for the operation
	binary_emit(parser, slot, op->type, left, right);
}


// Emit bytecode for the left operand to a binary operation before the right
// operand is parsed. Used for things like converting the left operand of a
// condition (ie. `and` or `or` statement) into a jump operand (by emitting an
// `IS_TRUE_L` or `IS_FALSE_L`).
static void expr_binary_left(Parser *parser, TokenType operator,
		Operand *left) {
	// Convert the operand to a jump if it's a local and we're dealing with
	// a conditional operator
	if ((operator == TOKEN_AND || operator == TOKEN_OR) &&
			left->type == OP_LOCAL) {
		operand_to_jump(parser, left);
	}
}


// Returns true if the operand provided to a unary operator is of a valid type.
static bool unary_is_valid(TokenType operator, OpType op) {
	switch (operator) {
	case TOKEN_SUB:
		// Number
		return op == OP_LOCAL || op == OP_NUMBER || op == OP_INTEGER;
	case TOKEN_NOT:
		// Anything
		return true;
	default:
		// Invalid unary operator (shouldn't happen)
		return false;
	}
}


// Emit bytecode to perform a unary negation on an operand.
static void unary_neg(Parser *parser, uint16_t slot, Operand *operand) {
	// Emit negation instruction
	fn_emit(parser_fn(parser), NEG_L, slot, operand->value, 0);

	// Set operand to resulting local
	operand->type = OP_LOCAL;
	operand->value = slot;
}


// Emit bytecode to perform a boolean negation on an operand.
static void unary_boolean_not(Parser *parser, uint16_t slot, Operand *operand) {

}


// Emit bytecode to perform a unary operation, storing the result into `slot`.
// Returns the result of the unary operation.
static void expr_unary(Parser *parser, uint16_t slot, Token *op,
		Operand *operand) {
	// Ensure operand is of a valid type
	if (!unary_is_valid(op->type, operand->type)) {
		// Trigger an invalid operand error
		err_unexpected(parser, op, "Invalid operand to unary operator `%.*s`",
			op->length, op->start);
		return;
	}

	// Attempt to fold operation
	if (fold_unary(parser, op->type, operand)) {
		return;
	}

	// Depending on the operator
	switch (op->type) {
	case TOKEN_SUB:
		unary_neg(parser, slot, operand);
		break;
	case TOKEN_NOT:
		unary_boolean_not(parser, slot, operand);
		break;
	default:
		// Shouldn't happen
		break;
	}
}


// Emit bytecode for a struct field access as a postfix operator. Stores the
// resulting field in `slot`.
static void postfix_field_access(Parser *parser, uint16_t slot,
		Operand *operand) {
	Lexer *lexer = &parser->lexer;

	// Can only index locals
	if (operand->type != OP_LOCAL) {
		err_fatal(parser, &lexer->token, "Attempt to index non-local");
		return;
	}

	// Skip the dot
	Token dot = lexer->token;
	lexer_next(lexer);

	// Expect an identifier
	err_expect(parser, TOKEN_IDENTIFIER, &dot, "Expected identifier after `.`");

	// Add the field to the state's field list
	Identifier ident;
	ident.name = lexer->token.start;
	ident.length = lexer->token.length;
	Index field_index = state_add_field(parser->state, ident);
	fn_emit(parser_fn(parser), STRUCT_FIELD, slot, operand->value, field_index);

	// The field is now in `slot`
	operand->type = OP_LOCAL;
	operand->value = slot;
}


// Parse the arguments to a function call into consecutive local slots on the
// top of the stack. Returns the arity of the function call.
static uint16_t parse_call_args(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Skip the opening parenthesis
	Token open = lexer->token;
	lexer_next(lexer);

	// Parse consecutive arguments
	uint16_t arity = 0;
	while (lexer->token.type != TOKEN_EOF &&
			lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
		// Parse the argument into a slot on top of the stack
		arity++;
		uint16_t slot = local_reserve(parser);
		expr_emit(parser, slot);

		// Ensure we have a comma or closing parenthesis
		if (lexer->token.type == TOKEN_CLOSE_PARENTHESIS) {
			// Finished with arguments
			break;
		} else if (lexer->token.type == TOKEN_COMMA) {
			// Skip the comma
			lexer_next(lexer);
		} else {
			// Unexpected token
			err_unexpected(parser, &lexer->token,
				"Expected `,` after argument to function call");
		}
	}

	// Ensure we have a closing parenthesis
	err_expect(parser, TOKEN_CLOSE_PARENTHESIS, &open,
		"Expected `)` to close `(` in function call");
	lexer_next(lexer);
	return arity;
}


// Emit bytecode for a function call as a postfix operator. Stores the return
// value of the function call into `slot`.
static void postfix_function_call(Parser *parser, uint16_t slot,
		Operand *operand) {
	Lexer *lexer = &parser->lexer;

	// Save the number of locals on the top of the stack before we parse the
	// function call, so we know how many locals we have to free
	// Since we only allocate temporary locals (no named ones), we don't need
	// to bother manipulating the `parser->locals` array
	uint32_t locals_count = parser->locals_count;

	// Operand must be a local, function, or native function
	uint16_t base;
	if (operand->type == OP_FUNCTION || operand->type == OP_NATIVE) {
		// Move the function into a local on the top of the stack
		base = local_reserve(parser);
		expr_discharge_local(parser, *operand, base);
	} else if (operand->type == OP_LOCAL) {
		// Check that the local is on top of the stack
		if (operand->value < parser->locals_count - 1) {
			// Not on top of the stack, so move it to a temporary local there
			base = local_reserve(parser);
			fn_emit(parser_fn(parser), MOV_LL, base, operand->value, 0);
		} else {
			// On top of the stack
			base = operand->value;
		}
	} else {
		// Not calling a function
		err_fatal(parser, &lexer->token, "Attempt to call non-function");
		return;
	}

	// Parse the function arguments into consecutive slots on top of the stack
	uint16_t arity = parse_call_args(parser);

	// Emit the call instruction
	fn_emit(parser_fn(parser), CALL, base, arity, slot);

	// Free allocated locals
	parser->locals_count = locals_count;

	// Set resulting operand to return value of function
	operand->type = OP_LOCAL;
	operand->value = slot;
}


// Emit bytecode to perform a postfix operation which can be set. These are
// struct field and array accesses.
static bool postfix_accesses(Parser *parser, uint16_t slot, Operand *operand) {
	switch (parser->lexer.token.type) {
	case TOKEN_DOT:
		postfix_field_access(parser, slot, operand);
		return true;
	default:
		return false;
	}
}


// Emit bytecode to perform a postfix operation, like a struct field access,
// function call, or array access. `operand` is the most recently parsed operand
// just before the postfix operator, on which we're performing the postfix
// operation.
static bool expr_postfix(Parser *parser, uint16_t slot, Operand *operand) {
	switch (parser->lexer.token.type) {
	case TOKEN_OPEN_PARENTHESIS:
		postfix_function_call(parser, slot, operand);
		return true;
	default:
		return postfix_accesses(parser, slot, operand);
	}
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
	operand.value = lexer->token.type - TOKEN_TRUE + TAG_TRUE;
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
	top_level_demote_external(parser, slot, field, package);

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
		upvalue_demote(parser, slot, local.index);
		result.value = slot;
		break;

	case RESOLVED_TOP_LEVEL:
		// Move the top level local into the slot
		top_level_demote(parser, slot, local.index);
		result.value = slot;
		break;

	case RESOLVED_PACKAGE:
		// Expect a field access after a struct name
		// Return so we don't skip another token
		return operand_top_level(parser, slot, local.index);

	default:
		// Undefined variable
		err_fatal(parser, &lexer->token, "Undefined variable `%.*s`", length,
			name);
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
	if (operator_is_unary(lexer->token.type)) {
		// Save and skip the unary operator
		Token operator = lexer->token;
		lexer_next(lexer);

		// Parse the operand to the unary operation
		Operand operand = expr_left(parser, slot);

		// Emit bytecode for the unary operation
		expr_unary(parser, slot, &operator, &operand);
		return operand;
	} else {
		// No more unary operators, parse an operand
		Operand operand = expr_operand(parser, slot);

		// Iteratively parse postfix operators like struct field access, array
		// indexing, and function calls
		while (expr_postfix(parser, slot, &operand)) {}

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
		Token operator = lexer->token;
		lexer_next(lexer);

		// Emit bytecode for the left operand (like converting it to a jump
		// operand if part of a condition)
		expr_binary_left(parser, operator.type, &left);

		// Parse the right operand to the operation
		uint16_t right_slot = local_reserve(parser);
		Precedence right_prec = prec_binary(operator.type);
		Operand right = expr_precedence(parser, right_slot, right_prec);
		local_free(parser);

		// Emit the operation, where the result of the operation becomes the new
		// left operand to the next binary operation
		expr_binary(parser, slot, &operator, &left, right);
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
//  Assignment
//

// Parses an expression into a new local with the name `name`.
static void parse_declaration_local(Parser *parser, char *name,
		uint32_t length) {
	// Allocate new local
	uint16_t slot = local_new(parser);
	Local *local = &vec_at(parser->locals, slot);
	local->name = name;
	local->length = length;

	// Parse expression into new local
	expr_emit(parser, slot);
}


// Parses an expression into a new top level local with the name `name`.
static void parse_declaration_top_level(Parser *parser, char *name,
		uint32_t length) {
	// Allocate new top level local
	Package *pkg = parser_pkg(parser);
	Index top_level = pkg_local_add(pkg, name, length, VALUE_NIL);

	// Parse expression into top level
	uint16_t temp = local_reserve(parser);
	Operand result = parse_expr(parser, temp);
	expr_discharge_top_level(parser, result, top_level);
	local_free(parser);
}


// Parses an initial assignment using the `let` token.
static void parse_declaration(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Skip the `let`
	Token let = lexer->token;
	lexer_next(lexer);

	// Expect an identifier
	err_expect(parser, TOKEN_IDENTIFIER, &let,
		"Expected identifier after `let`");
	Token name = lexer->token;
	lexer_next(lexer);

	// Expect an assignment token
	err_expect(parser, TOKEN_ASSIGN, &name,
		"Expected `=` after identifier in `let` assignment");
	lexer_next(lexer);

	// Ensure the local isn't already defined
	if (!local_is_unique(parser, name.start, name.length)) {
		err_fatal(parser, &name, "Variable `%.*s` already defined",
			name.length, name.start);
		return;
	}

	// Parse expression into top level local if this is the uppermost function
	// scope
	if (parser->scope->parent == NULL && parser->scope->block_depth == 1) {
		parse_declaration_top_level(parser, name.start, name.length);
	} else {
		parse_declaration_local(parser, name.start, name.length);
	}
}


// Parses the remainder of an assignment after and including the `=`. `operand`
// is the operand containing information about the first identifier in the list
// to the left of the `=`.
static void parse_assignment(Parser *parser, Operand operand) {
	Lexer *lexer = &parser->lexer;

	// Skip the assignment token
	lexer_next(lexer);

	// Save the last retrieval instruction, which will need to be converted
	// into a storage instruction after parsing the expression
	Instruction retrieval = vec_last(parser_fn(parser)->instructions);
	BytecodeOpcode opcode = ins_arg(retrieval, 0);

	// If the retrieval was a specially emitted storage instruction
	if (opcode == MOV_LT || opcode == MOV_LU || opcode == STRUCT_FIELD) {
		// Remove the last retrieval instruction
		vec_len(parser_fn(parser)->instructions)--;

		// Parse an expression into a temporary local
		uint16_t expr_slot = local_reserve(parser);
		Operand result = parse_expr(parser, expr_slot);

		if (opcode == MOV_LT) {
			// Top level
			uint16_t top_level = ins_arg(retrieval, 2);
			uint16_t package = ins_arg(retrieval, 3);
			expr_discharge_top_level_external(parser, result, package, top_level);
		} else if (opcode == MOV_LU) {
			// Upvalue
			uint16_t upvalue = ins_arg(retrieval, 2);
			expr_discharge_upvalue(parser, result, upvalue);
		} else if (opcode == STRUCT_FIELD) {
			// Struct field
			uint16_t struct_slot = ins_arg(retrieval, 2);
			uint16_t field = ins_arg(retrieval, 3);
			expr_discharge_field(parser, result, struct_slot, field);
		}

		// Free the temporary local we parsed the expression into
		local_free(parser);
	} else {
		// Parse the expression directly into the local
		expr_emit(parser, operand.value);
	}
}


// Parses an assignment or function call (no way to tell between the two easily
// as they both start with an identifier followed by field/array accesses).
static void parse_assignment_or_call(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Expect an identifier
	err_expect(parser, TOKEN_IDENTIFIER, &lexer->token, "Expected identifier");
	Token ident = lexer->token;

	// Parse identifier into a temporary local
	uint16_t slot = local_reserve(parser);
	Operand operand = operand_identifier(parser, slot);

	// Iteratively parse postfix struct field or array accesses
	bool requires_slot = false;
	while (postfix_accesses(parser, slot, &operand)) {
		requires_slot = true;
	}

	// If we don't actually require storing the first identifier, then we can
	// get rid of the allocated slot here and save a stack slot
	if (!requires_slot) {
		local_free(parser);
	}

	// Depending on the token after that
	if (lexer->token.type == TOKEN_ASSIGN) {
		// Assignment
		parse_assignment(parser, operand);
	} else if (lexer->token.type == TOKEN_OPEN_PARENTHESIS) {
		// Function call, so just keep parsing postfix operators
		while (expr_postfix(parser, slot, &operand)) {}
	} else {
		err_unexpected(parser, &ident, "Expected `=` or `(` after identifier");
		return;
	}

	// Free the temporary local we allocated for the original identifier
	if (requires_slot) {
		local_free(parser);
	}
}



//
//  If Statements
//

// Parses a block surrounded by braces.
static void parse_braced_block(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Opening brace
	Token open = lexer->token;
	err_expect(parser, TOKEN_OPEN_BRACE, &lexer->token,
		"Expected `{`");
	lexer_next(lexer);

	// Block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Closing brace
	err_expect(parser, TOKEN_CLOSE_BRACE, &open,
		"Expected `}` to close `{`");
	lexer_next(lexer);
}


// Parses the condition of an if branch.
static Operand parse_conditional_expr(Parser *parser) {
	// Parse the condition
	uint16_t slot = local_reserve(parser);
	Operand condition = parse_expr(parser, slot);
	local_free(parser);

	// Convert the condition to a jump if it's a local
	if (condition.type == OP_LOCAL) {
		operand_to_jump(parser, &condition);
	}

	return condition;
}


// Returns true if a condition is constant and equivalent to false.
static bool cond_is_const_false(Operand *condition) {
	return condition->type == OP_PRIMITIVE && condition->value != TAG_TRUE;
}


// Returns true if a condition is constant and equivalent to true.
static bool cond_is_const_true(Operand *condition) {
	return !cond_is_const_false(condition) && condition->type != OP_JUMP;
}


// Parse an if statement, and any subsequent else if or else branches.
static void parse_if(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	Function *fn = parser_fn(parser);

	// Store the condition of the branch before the current one so we can patch
	// its false case if we need to insert a jump. Only stores the condition of
	// constant true and non-constant branches (constant false branches are
	// ignored)
	Operand previous_condition = operand_new();

	// Keep a jump list of all final jumps emitted at the end of if blocks, so
	// we can point all of them to after the whole if statement
	Index list = NOT_FOUND;

	// True if we should stop emitting bytecode for branches because we found a
	// constant true condition
	bool fold = false;

	// Trick the loop into thinking the first if is actually an else if
	lexer->token.type = TOKEN_ELSE_IF;

	while (lexer->token.type == TOKEN_ELSE_IF ||
			lexer->token.type == TOKEN_ELSE) {
		// Save the instruction length so we can delete the bytecode emitted
		// for this branch if needed
		uint32_t saved_length = vec_len(fn->instructions);

		// If the condition for the current branch doesn't turn out constant
		// false, then we need to insert a jump for the previous branch over
		// the current one. This needs to be before the current branch's
		// condition, so insert it here
		Index final_jump = NOT_FOUND;
		if (previous_condition.type == OP_JUMP) {
			final_jump = fn_emit(fn, JMP, 0, 0, 0);
		}

		// Check if we're parsing an else or else if branch
		bool is_else = lexer->token.type == TOKEN_ELSE;
		lexer_next(lexer);

		// Parse the condition
		Operand condition;
		if (is_else) {
			// Treat else branches as constant true else if branches
			condition.type = OP_PRIMITIVE;
			condition.value = TAG_TRUE;
		} else {
			condition = parse_conditional_expr(parser);
		}

		// If we're folding all future branches because we found a constant true
		// condition earlier, or this branch is constant false
		if (fold || cond_is_const_false(&condition)) {
			// Parse the block and get rid of its contents (including the
			// potentially emitted jump for the previous branch)
			parse_braced_block(parser);
			vec_len(fn->instructions) = saved_length;
		} else {
			if (previous_condition.type == OP_JUMP) {
				// Keep the previous branch's jump and patch its false case to after
				// this jump
				jmp_prepend(fn, &list, final_jump);
				jmp_false_case(fn, previous_condition.jump, final_jump + 1);
			}

			// Parse the contents of this branch
			parse_braced_block(parser);
			previous_condition = condition;

			// If the condition is constant true, fold all future branches
			if (cond_is_const_true(&condition)) {
				fold = true;
			}
		}

		// Stop after parsing an else branch
		if (is_else) {
			break;
		}
	}

	// Patch the false case of the last branch's condition to here
	if (previous_condition.type == OP_JUMP) {
		Index target = vec_len(fn->instructions);
		jmp_false_case(fn, previous_condition.jump, target);
	}

	// Point all end of branch jumps here
	jmp_target_all(fn, list, vec_len(fn->instructions));
}



//
//  While Loop
//

// Pushes a loop onto the parser's current function's linked list.
static void loop_push(Parser *parser, Loop *loop) {
	loop->head = NOT_FOUND;
	loop->parent = parser->scope->loop;
	parser->scope->loop = loop;
}


// Pops the top loop from the parser's current function's linked list.
static void loop_pop(Parser *parser) {
	parser->scope->loop = parser->scope->loop->parent;
}


// Parse a while loop.
static void parse_while(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	Function *fn = parser_fn(parser);

	// Skip `while` token
	lexer_next(lexer);

	// Save the instruction length in case we need to fold the while loop
	Index start = vec_len(fn->instructions);

	// Add a loop to the function's linked list
	Loop loop;
	loop_push(parser, &loop);

	// Parse expression and body
	Operand condition = parse_conditional_expr(parser);
	parse_braced_block(parser);

	// Remove the loop from the linked list
	loop_pop(parser);

	// Fold if condition is constant false
	if (cond_is_const_false(&condition)) {
		vec_len(fn->instructions) = start;
		return;
	}

	// Insert a jump back to the loop's start
	uint16_t offset = vec_len(fn->instructions) - start;
	fn_emit(fn, LOOP, offset, 0, 0);

	// Patch the condition's false case here
	if (condition.type == OP_JUMP) {
		jmp_false_case(fn, condition.jump, vec_len(fn->instructions));
	}

	// Patch break statement jumps here
	jmp_target_all(fn, loop.head, vec_len(fn->instructions));
}



//
//  Infinite Loops
//

// Parses an infinite loop.
static void parse_loop(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	Function *fn = parser_fn(parser);

	// Skip the `loop` token
	lexer_next(lexer);

	// Add loop to start of linked list
	Loop loop;
	loop_push(parser, &loop);

	// Parse contents
	Index start = vec_len(fn->instructions);
	parse_braced_block(parser);

	// Insert loop instruction
	uint16_t offset = vec_len(fn->instructions) - start;
	fn_emit(fn, LOOP, offset, 0, 0);

	// Remove loop from linked list
	loop_pop(parser);

	// Patch break statements
	jmp_target_all(fn, loop.head, vec_len(fn->instructions));
}



//
//  Break Statements
//

// Parses a break statement.
static void parse_break(Parser *parser) {
	Lexer *lexer = &parser->lexer;
	Function *fn = parser_fn(parser);

	// Ensure we're inside a loop
	if (parser->scope->loop == NULL) {
		err_fatal(parser, &lexer->token, "`break` statement not inside loop");
		return;
	}

	// Skip the break token
	lexer_next(lexer);

	// Insert an empty jump
	Index jump = fn_emit(fn, JMP, 0, 0, 0);

	// Append it to the loop's jump list
	jmp_prepend(fn, &parser->scope->loop->head, jump);
}



//
//  Blocks and Statements
//

// Parses a single statement, like an `if` or `while` construct.
static void parse_statement(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	switch (lexer->token.type) {
		// Local declaration
	case TOKEN_LET:
		parse_declaration(parser);
		break;

		// If statements
	case TOKEN_IF:
		parse_if(parser);
		break;

		// While loops
	case TOKEN_WHILE:
		parse_while(parser);
		break;

		// Infinite loops
	case TOKEN_LOOP:
		parse_loop(parser);
		break;

		// Break statement
	case TOKEN_BREAK:
		parse_break(parser);
		break;

		// Unconditional block
	case TOKEN_OPEN_BRACE:
		parse_braced_block(parser);
		break;

		// Assignment or function call
	default:
		parse_assignment_or_call(parser);
		break;
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
