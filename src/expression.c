
//
//  Expression
//


#include <stdio.h>

#include "compiler.h"
#include "lexer.h"
#include "value.h"
#include "operators.h"


// Forward declarations.
void parse_precedence(Compiler *compiler, TokenType terminator,
	int precedence);
void left(Compiler *compiler);
void prefix(Compiler *compiler, TokenType operator);
void infix(Compiler *compiler, TokenType terminator, TokenType operator);

bool is_binary_operator(TokenType operator);
int operator_precedence(TokenType operator);
Associativity operator_associativity(TokenType operator);


// Generates bytecode for evaluating an expression, leaving the
// resulting value on the top of the stack.
//
// Uses a Pratt parser to compile the expression. Triggers an
// error on the compiler if the expression fails to parse.
//
// The expression stops parsing when it reaches the terminator
// token. If the terminator is a new line token, and we could
// continue successfully parsing the expression on the next line,
// then parsing continues.
//
// The terminator token is not consumed.
void expression(Compiler *compiler, TokenType terminator) {
	parse_precedence(compiler, terminator, 0);
}


// Compiles an expression, stopping once we reach an operator
// with a higher precedence than the given precedence level.
void parse_precedence(Compiler *compiler, TokenType terminator,
		int precedence) {
	Lexer *lexer = &compiler->vm->lexer;

	// Compile the left hand side of an infix operator. Could be
	// a prefix operator, variable, function call, or string
	// literal. Leaves the result on the top of the stack.
	left(compiler);

	// Peek at the next token.
	Token operator = peek(lexer, 0);

	// Keep compiling operators while their precedence is greater
	// than the precedence argument.
	while (operator.type != TOKEN_END_OF_FILE && operator.type != terminator &&
			precedence < operator_precedence(operator.type)) {
		if (is_binary_operator(operator.type)) {
			// Consume the operator token.
			consume(lexer);

			// Compile an infix operator.
			infix(compiler, terminator, operator.type);

			// Update the operator
			operator = peek(lexer, 0);
		} else {
			// Expected binary operator.
			error(compiler, "Expected binary operator, found `%.*s`.",
				operator.length, operator.location);
			return;
		}
	}
}


// Compile the left hand side to an infix operator. This could
// simply be a number or variable name, or it could be a prefix
// operator (like negation).
//
// Leaves the left hand side of the expression on the top of the
// stack.
void left(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Match a prefix operator, variable, function call, or
	// constant.
	if (match(lexer, TOKEN_SUBTRACTION)) {
		// Negation operator
		prefix(compiler, TOKEN_NEGATION);
	} else if (match(lexer, TOKEN_BOOLEAN_NOT)) {
		// Boolean not operator
		prefix(compiler, TOKEN_BOOLEAN_NOT);
	} else if (match(lexer, TOKEN_BITWISE_NOT)) {
		// Bitwise not operator
		prefix(compiler, TOKEN_BITWISE_NOT);
	} else if (match2(lexer, TOKEN_IDENTIFIER, TOKEN_OPEN_PARENTHESIS)) {
		// Function call
		Token function = consume(lexer);
		emit_function_call(compiler, function.location, function.length);
	} else if (match(lexer, TOKEN_IDENTIFIER)) {
		// Variable
		Token variable = consume(lexer);
		push_local(compiler, variable.location, variable.length);
	} else if (match(lexer, TOKEN_NUMBER)) {
		// Number literal
		Token number = consume(lexer);
		push_number(compiler, number.number);
	} else if (match(lexer, TOKEN_STRING)) {
		// String literal
		Token literal = consume(lexer);
		String *string = push_string(compiler);
		char *sequence = extract_string_literal(&literal, string);

		if (sequence != NULL) {
			// Invalid escape sequence in string
			error(compiler, "Invalid escape sequence `%.*s`", 2, sequence);
		}
	} else if (match(lexer, TOKEN_OPEN_PARENTHESIS)) {
		// Sub-expression
		// Consume the parenthesis
		consume(lexer);

		// Parse the expression
		parse_precedence(compiler, TOKEN_CLOSE_PARENTHESIS, 0);

		// Consume the closing parenthesis
		expect(compiler, TOKEN_CLOSE_PARENTHESIS,
			"Expected `)` to close opening parenthesis.");
	} else {
		// Unrecognised operand, so trigger an error.
		Token token = peek(lexer, 0);
		error(compiler, "Expected operand in expression, found `%.*s`",
			token.length, token.location);
	}
}


// Compiles a prefix operator, leaving the result on the top of
// the stack.
void prefix(Compiler *compiler, TokenType operator) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the prefix operator token.
	consume(lexer);

	// Compile the argument to this operator, leaving it on the
	// top of the stack.
	//
	// The terminator token here doesn't matter, because we're
	// only compiling the left hand side of an expression, and it
	// would be invalid for the terminator token to appear here
	// anyway.
	parse_precedence(compiler, TOKEN_END_OF_FILE, 0);

	// Push a native call to the operator's corresponding
	// handling function.
	emit_native_operator_call(compiler, operator);
}


// Compiles an infix operator, leaving the result on the top
// of the stack.
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Compiler *compiler, TokenType terminator, TokenType operator) {
	Lexer *lexer = &compiler->vm->lexer;

	// Determine the precedence level of the operator.
	int precedence = operator_precedence(operator);
	if (operator_associativity(operator) == ASSOCIATIVITY_RIGHT) {
		precedence--;
	}

	// Push the right hand side of the expression onto
	// the stack.
	parse_precedence(compiler, terminator, precedence);

	// Emit the native call for this operator.
	emit_native_operator_call(compiler, operator);
}
