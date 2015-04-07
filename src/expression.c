
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


// Peeks at the next token, assuming its a binary operator.
Token peek_operator(Lexer *lexer, TokenType terminator) {
	Token operator = peek(lexer, 0);
	if (operator.type == TOKEN_END_OF_FILE ||
			(terminator != TOKEN_LINE && operator.type == terminator)) {
		// Stop the expression
		Token result;
		result.type = TOKEN_NONE;
		return result;
	} else if (operator.type == TOKEN_LINE) {
		Token token = peek(lexer, 1);
		if (is_binary_operator(token.type)) {
			consume(lexer);
			operator = token;
		} else {
			// Terminate the expression here
			Token result;
			result.type = TOKEN_NONE;
			return result;
		}
	}

	return operator;
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
	Token operator = peek_operator(lexer, terminator);

	// Keep compiling operators while their precedence is greater
	// than the precedence argument.
	while (operator.type != TOKEN_NONE &&
			precedence < operator_precedence(operator.type)) {
		if (is_binary_operator(operator.type)) {
			// Compile an infix operator.
			consume(lexer);
			infix(compiler, terminator, operator.type);
			operator = peek_operator(lexer, terminator);
		} else {
			// Expected binary operator.
			error(compiler, "Expected binary operator, found `%.*s`",
				operator.length, operator.location);
			return;
		}
	}
}


// Matches across newlines.
bool newline_match(Lexer *lexer, TokenType expected) {
	Token operator = peek(lexer, 0);
	if (operator.type == expected) {
		return true;
	} else if (operator.type == TOKEN_LINE) {
		Token token = peek(lexer, 1);
		if (token.type == expected) {
			consume(lexer);
			return true;
		}
	}

	return false;
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
	if (newline_match(lexer, TOKEN_SUBTRACTION)) {
		// Negation operator
		prefix(compiler, TOKEN_NEGATION);
	} else if (newline_match(lexer, TOKEN_BOOLEAN_NOT)) {
		// Boolean not operator
		prefix(compiler, TOKEN_BOOLEAN_NOT);
	} else if (newline_match(lexer, TOKEN_BITWISE_NOT)) {
		// Bitwise not operator
		prefix(compiler, TOKEN_BITWISE_NOT);
	} else if (newline_match(lexer, TOKEN_IDENTIFIER)) {
		// Variable
		Token variable = consume(lexer);
		push_local(compiler, variable.location, variable.length);
	} else if (newline_match(lexer, TOKEN_NUMBER)) {
		// Number literal
		Token number = consume(lexer);
		push_number(compiler, number.number);
	} else if (newline_match(lexer, TOKEN_STRING)) {
		// String literal
		Token literal = consume(lexer);
		String **string = push_string(compiler);
		char *sequence = extract_string_literal(&literal, string);

		if (sequence != NULL) {
			// Invalid escape sequence in string
			error(compiler, "Invalid escape sequence `%.*s`", 2, sequence);
		}
	} else if (newline_match(lexer, TOKEN_OPEN_PARENTHESIS)) {
		// Sub-expression
		consume(lexer);
		parse_precedence(compiler, TOKEN_CLOSE_PARENTHESIS, 0);
		expect(compiler, TOKEN_CLOSE_PARENTHESIS,
			"Expected `)` to close opening parenthesis");
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
	NativeFunction fn = operator_ptr(operator);
	emit_native(compiler, fn);
}


// Compiles an infix operator, leaving the result on the top
// of the stack.
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Compiler *compiler, TokenType terminator, TokenType operator) {
	// Determine the precedence level of the operator.
	int precedence = operator_precedence(operator);
	if (operator_associativity(operator) == ASSOCIATIVITY_RIGHT) {
		precedence--;
	}

	// Push the right hand side of the expression onto
	// the stack.
	parse_precedence(compiler, terminator, precedence);

	// Emit the native call for this operator.
	NativeFunction fn = operator_ptr(operator);
	emit_native(compiler, fn);
}
