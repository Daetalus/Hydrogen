
//
//  Expression
//


#include <stdio.h>

#include "lib/operator.h"
#include "lexer.h"
#include "value.h"
#include "compiler.h"
#include "bytecode.h"
#include "expression.h"
#include "vm.h"
#include "error.h"


// The precedence of an operator.
typedef enum {
	// No precision
	PREC_NONE,
	// Boolean or
	PREC_BOOLEAN_OR,
	// Boolean and
	PREC_BOOLEAN_AND,
	// Bitwise or
	PREC_BITWISE_OR,
	// Bitwise xor
	PREC_BITWISE_XOR,
	// Bitwise and
	PREC_BITWISE_AND,
	// Equal, not equal
	PREC_EQUALITY,
	// Less than, less than or equal to, greater than, greater
	// than or equal to
	PREC_ORDERING,
	// Shift left, shift right
	PREC_BITWISE_SHIFT,
	// Addition, subtraction
	PREC_ADDITION,
	// Multiplication, division, modulo
	PREC_MULTIPLICATION,
	// Bitwise not, boolean not, negation
	PREC_NOT,
} Precedence;


// The associativity of an operator.
typedef enum {
	// Left associative, like addition and multiplication.
	//
	// This means that the expression:
	// 3 + 4 + 5
	// Becomes:
	// ((3 + 4) + 5)
	ASSOC_LEFT,

	// Right associative, like exponentiation.
	//
	// This means that the expression:
	// 3 ^ 4 ^ 5
	// Becomes:
	// (3 ^ (4 ^ 5))
	ASSOC_RIGHT,
} Associativity;


// Information about a prefix operator.
typedef struct {
	Precedence precedence;
	NativeFunction fn;
} PrefixOperator;


// Information about an infix operator.
typedef struct {
	Precedence precedence;
	Associativity associativity;
	NativeFunction fn;
} InfixOperator;


// The callback function to compile an operand.
// The lexer's current token will be the start of the operand,
// so the function should consume the tokens it requires to
// parse the operand, and emit the corresponding bytecode.
typedef void (*OperandFunction)(Compiler *compiler);


// Information about an operand.
typedef struct {
	OperandFunction fn;
} Operand;


// The type of a rule.
typedef enum {
	RULE_PREFIX,
	RULE_INFIX,
	RULE_BOTH,
	RULE_OPERAND,
	RULE_UNUSED,
} RuleType;


// A rule instructing the expression parser on what to do when
// it encounters an operator.
typedef struct {
	RuleType type;

	union {
		PrefixOperator prefix;
		InfixOperator infix;
		Operand operand;

		// An operator that uses the same token for both prefix
		// and infix operators (eg. subtraction and negation).
		struct {
			PrefixOperator prefix;
			InfixOperator infix;
		} both;
	};
} Rule;


// Compile a number.
void operand_number(Compiler *compiler);

// Compile an identifier.
void operand_identifier(Compiler *compiler);

// Compile a string literal.
void operand_string_literal(Compiler *compiler);

// Compile a sub-expression (surrounded by parentheses).
void sub_expression(Compiler *compiler);

// Compile a true constant.
void operand_true(Compiler *compiler);

// Compile a false constant.
void operand_false(Compiler *compiler);

// Compile a nil constant.
void operand_nil(Compiler *compiler);

// Compile a function operand.
void operand_function(Compiler *compiler);


// Expression rules array. The entries in the array are in order
// of the tokens as defined in the lexer, so we can simply
// index it with a token to find the rule for any token in
// constant time.
//
// Note: Must be kept in order of the `Token` enum in `lexer.h`
Rule rules[] = {
	// Addition
	{RULE_INFIX,
	{.infix = {PREC_ADDITION, ASSOC_LEFT, &operator_addition}}},
	// Subtraction and negation
	{RULE_BOTH, {.both = {
		{PREC_NOT, &operator_negation},
		{PREC_ADDITION, ASSOC_LEFT, &operator_subtraction}
	}}},
	// Multiplication
	{RULE_INFIX,
	{.infix = {PREC_MULTIPLICATION, ASSOC_LEFT, &operator_multiplication}}},
	// Division
	{RULE_INFIX,
	{.infix = {PREC_MULTIPLICATION, ASSOC_LEFT, &operator_division}}},
	// Modulo
	{RULE_INFIX,
	{.infix = {PREC_MULTIPLICATION, ASSOC_LEFT, &operator_modulo}}},

	// Boolean and
	{RULE_INFIX,
	{.infix = {PREC_BOOLEAN_AND, ASSOC_LEFT, &operator_boolean_and}}},
	// Boolean or
	{RULE_INFIX,
	{.infix = {PREC_BOOLEAN_OR, ASSOC_LEFT, &operator_boolean_or}}},
	// Boolean not
	{RULE_PREFIX,
	{.prefix = {PREC_NOT, &operator_boolean_not}}},
	// Equal
	{RULE_INFIX,
	{.infix = {PREC_EQUALITY, ASSOC_LEFT, &operator_equal}}},
	// Not equal
	{RULE_INFIX,
	{.infix = {PREC_EQUALITY, ASSOC_LEFT, &operator_not_equal}}},
	// Less than
	{RULE_INFIX,
	{.infix = {PREC_ORDERING, ASSOC_LEFT, &operator_less_than}}},
	// Less than equal to
	{RULE_INFIX,
	{.infix = {PREC_ORDERING, ASSOC_LEFT, &operator_less_than_equal_to}}},
	// Greater than
	{RULE_INFIX,
	{.infix = {PREC_ORDERING, ASSOC_LEFT, &operator_greater_than}}},
	// Greater than equal to
	{RULE_INFIX,
	{.infix = {PREC_ORDERING, ASSOC_LEFT, &operator_greater_than_equal_to}}},

	// Left shift
	{RULE_INFIX,
	{.infix = {PREC_BITWISE_SHIFT, ASSOC_LEFT, &operator_left_shift}}},
	// Right shift
	{RULE_INFIX,
	{.infix = {PREC_BITWISE_SHIFT, ASSOC_LEFT, &operator_right_shift}}},
	// Bitwise and
	{RULE_INFIX,
	{.infix = {PREC_BITWISE_AND, ASSOC_LEFT, &operator_bitwise_and}}},
	// Bitwise or
	{RULE_INFIX,
	{.infix = {PREC_BITWISE_OR, ASSOC_LEFT, &operator_bitwise_or}}},
	// Bitwise not
	{RULE_PREFIX,
	{.prefix = {PREC_NOT, &operator_bitwise_not}}},
	// Bitwise xor
	{RULE_INFIX,
	{.infix = {PREC_BITWISE_XOR, ASSOC_LEFT, &operator_bitwise_xor}}},

	// Assignment
	{RULE_UNUSED},
	// Addition assignment
	{RULE_UNUSED},
	// Subtraction assignment
	{RULE_UNUSED},
	// Multiplication assignment
	{RULE_UNUSED},
	// Division assignment
	{RULE_UNUSED},
	// Modulo assignment
	{RULE_UNUSED},

	// Open parenthesis
	{RULE_OPERAND, {.operand = {&sub_expression}}},
	// Close parenthesis
	{RULE_UNUSED},
	// Open bracket
	{RULE_UNUSED},
	// Close bracket
	{RULE_UNUSED},
	// Open brace
	{RULE_UNUSED},
	// Close brace
	{RULE_UNUSED},
	// Dot
	{RULE_UNUSED},
	// Comma
	{RULE_UNUSED},

	// Let
	{RULE_UNUSED},
	// If
	{RULE_UNUSED},
	// Else
	{RULE_UNUSED},
	// Else if
	{RULE_UNUSED},
	// While
	{RULE_UNUSED},
	// Loop
	{RULE_UNUSED},
	// Break
	{RULE_UNUSED},
	// For
	{RULE_UNUSED},
	// In
	{RULE_UNUSED},
	// Function
	{RULE_OPERAND, {.operand = {&operand_function}}},
	// Return
	{RULE_UNUSED},

	// True
	{RULE_OPERAND, {.operand = {&operand_true}}},
	// False
	{RULE_OPERAND, {.operand = {&operand_false}}},
	// Nil
	{RULE_OPERAND, {.operand = {&operand_nil}}},

	// Identifier
	{RULE_OPERAND, {.operand = {&operand_identifier}}},
	// Number
	{RULE_OPERAND, {.operand = {&operand_number}}},
	// String
	{RULE_OPERAND, {.operand = {&operand_string_literal}}},
	// Line
	{RULE_UNUSED},
	// End of file
	{RULE_UNUSED},
	// None
	{RULE_UNUSED},
};


// Compiles an expression, stopping once we reach an operator
// with a higher precedence than the given precedence level.
void parse_precedence(Compiler *compiler, ExpressionTerminator terminator,
	Precedence precedence);


// Compile the left hand side of an infix operator. This could
// be a number or variable name, or a prefix operator (like
// negation).
//
// Leaves the result on the top of the stack.
void left(Compiler *compiler, ExpressionTerminator terminator);


// Compiles a prefix operator, leaving the result on the top of
// the stack.
void prefix(Compiler *compiler, ExpressionTerminator terminator,
	Precedence precedence, NativeFunction fn);


// Compiles an infix operator, leaving the result on the top
// of the stack.
//
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Compiler *compiler, ExpressionTerminator terminator,
	InfixOperator operator);


// Peeks at the next token, assuming its a binary operator.
//
// Returns true if the next token is an operator, populating the
// infix operator argument.
bool next_operator(Lexer *lexer, ExpressionTerminator terminator,
	InfixOperator *infix);


// Generates bytecode to evaluate an expression parsed from the
// compiler's lexer. Leaves the result of the expression on the
// top of the stack. Stops parsing when `terminator` returns
// true.
//
// Triggers an error on the compiler if the expression fails to
// parse.
//
// The terminator token is not consumed.
void expression(Compiler *compiler, ExpressionTerminator terminator) {
	parse_precedence(compiler, terminator, PREC_NONE);
}


// Compiles an expression, stopping once we reach an operator
// with a higher precedence than the given precedence level.
void parse_precedence(Compiler *compiler, ExpressionTerminator terminator,
		Precedence precedence) {
	Lexer *lexer = &compiler->vm->lexer;
	InfixOperator operator;

	// Compile the left hand side of an infix operator.
	left(compiler, terminator);

	// Get the infix operator after the left argument.
	if (!next_operator(lexer, terminator, &operator)) {
		return;
	}

	// Keep compiling operators until we reach the end of the
	// expression, or an operator of higher precedence than the
	// one we're allowed.
	while (precedence < operator.precedence) {
		// Compile an infix operator.
		infix(compiler, terminator, operator);

		// Fetch the next operator
		if (!next_operator(lexer, terminator, &operator)) {
			return;
		}
	}
}


// Compile the left hand side of an infix operator. This could
// be a number or variable name, or a prefix operator (like
// negation).
//
// Leaves the result on the top of the stack.
void left(Compiler *compiler, ExpressionTerminator terminator) {
	Lexer *lexer = &compiler->vm->lexer;

	// Fetch the token we're using as the operand or prefix
	// operator.
	Token token = lexer_current(lexer);
	if (token.type == TOKEN_LINE) {
		// There might be a continuation of the expression on
		// next line.
		Token after = lexer_peek(lexer, 1);
		RuleType type = rules[after.type].type;
		if (type == RULE_OPERAND || type == RULE_PREFIX) {
			// The expression is continued onto the next line.
			lexer_consume(lexer);
			token = after;
		}
	}

	// Fetch the rule for the token.
	Rule rule = rules[token.type];

	if (rule.type == RULE_OPERAND) {
		// An operand, like a number or string literal
		rule.operand.fn(compiler);
	} else if (rule.type == RULE_PREFIX) {
		// A prefix operator, like negation or bitwise not
		prefix(compiler, terminator, rule.prefix.precedence, rule.prefix.fn);
	} else if (rule.type == RULE_BOTH) {
		// A prefix operator, but with for a token that also acts
		// as an infix operator.
		// Handle it the same way a prefix operator is.
		prefix(compiler, terminator, rule.both.prefix.precedence,
			rule.both.prefix.fn);
	} else {
		// Unrecognised left hand expression.
		error(lexer->line, "Expected operand in expression, found `%.*s`",
			token.length, token.location);
	}
}


// Compiles a prefix operator, leaving the result on the top of
// the stack.
void prefix(Compiler *compiler, ExpressionTerminator terminator,
		Precedence precedence, NativeFunction fn) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the prefix operator token.
	lexer_consume(lexer);

	// Compile the argument to the prefix operator.
	parse_precedence(compiler, terminator, 0);

	// Emit the native call
	emit_native(bytecode, fn);
}


// Compiles an infix operator, leaving the result on the top
// of the stack.
//
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Compiler *compiler, ExpressionTerminator terminator,
		InfixOperator operator) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Determine precedence level.
	Precedence precedence = operator.precedence;
	if (operator.associativity == ASSOC_RIGHT) {
		precedence--;
	}

	// Consume the operator token.
	lexer_consume(lexer);

	// Evaluate the right hand side of the expression, leaving
	// the result on the top of the stack.
	parse_precedence(compiler, terminator, precedence);

	// Emit the native call for this operator.
	emit_native(bytecode, operator.fn);
}


// Peeks at the next token, assuming its a binary operator.
//
// Returns true if the next token is an operator, populating the
// infix operator argument.
bool next_operator(Lexer *lexer, ExpressionTerminator terminator,
		InfixOperator *infix) {
	Token token = lexer_current(lexer);
	Rule rule;

	if (token.type == TOKEN_END_OF_FILE ||
			(terminator != NULL && terminator(token))) {
		// Reached the terminating token.
		return false;
	} else if (token.type == TOKEN_LINE) {
		token = lexer_peek(lexer, 1);
		rule = rules[token.type];

		if (rule.type != RULE_INFIX && rule.type != RULE_BOTH) {
			// We reached a new line token and there was no
			// continuation of the expression over the line.
			return false;
		}

		// Consume the newline token
		lexer_consume(lexer);
	} else {
		// Plain token, so set the rule.
		rule = rules[token.type];
	}

	if (rule.type == RULE_INFIX) {
		*infix = rule.infix;
	} else if (rule.type == RULE_BOTH) {
		*infix = rule.both.infix;
	} else {
		// Not an infix operator.
		error(lexer->line, "Expected binary operator, found `%.*s`",
			token.length, token.location);
		return false;
	}

	return true;
}



//
//  Operands
//

// Compile a number.
void operand_number(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Token number = lexer_consume(lexer);
	emit_push_number(&compiler->fn->bytecode, number.number);
}


// Compile an identifier.
void operand_identifier(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	if (match_function_call(lexer)) {
		// The operand is a function call instead of a variable
		function_call(compiler);
	} else {
		// The operand is a variable
		Token identifier = lexer_consume(lexer);
		push_local(compiler, identifier.location, identifier.length);
	}
}


// Compile a string literal.
void operand_string_literal(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	Token literal = lexer_consume(lexer);
	String **string = push_string(compiler);
	char *sequence = NULL;
	*string = parser_extract_literal(literal.location, literal.length,
		&sequence);

	if (sequence != NULL) {
		// Invalid escape sequence in string
		error(lexer->line, "Invalid escape sequence `%.*s` in string",
			2, sequence);
	}
}


// Returns true when a sub-expression should be terminated (at
// a close parenthesis).
bool should_terminate_sub_expression(Token token) {
	return token.type == TOKEN_CLOSE_PARENTHESIS;
}


// Compile a sub-expression (surrounded by parentheses).
void sub_expression(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	lexer_consume(lexer);
	parse_precedence(compiler, &should_terminate_sub_expression, PREC_NONE);
	expect(lexer, TOKEN_CLOSE_PARENTHESIS,
		"Expected `)` to close `(` in expression");
}


// Compile a true constant.
void operand_true(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	Lexer *lexer = &compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_TRUE);
}


// Compile a false constant.
void operand_false(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	Lexer *lexer = &compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_FALSE);
}


// Compile a nil constant.
void operand_nil(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	Lexer *lexer = &compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_NIL);
}


// Compile a function operand.
void operand_function(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function keyword
	lexer_consume(lexer);

	// Define a function
	Function *fn;
	fn->name = NULL;
	fn->
	vm_new_function(compiler->vm, &fn);
}
