
//
//  Error
//

#include <stdio.h>
#include <string.h>

#include "error.h"


// The maximum number of characters an error message can
// be.
#define MAX_ERROR_LENGTH 1024


// Sets the error on the VM.
static void set_error(VirtualMachine *vm, Lexer *lexer, char *message) {
	vm_free_error(vm);
	vm->error.description = message;
	vm->error.line = lexer_line(lexer);
	vm->error.package = NULL;
	vm->error.file = NULL;
}


// Triggers a custom error on the VM.
void err_fatal(VirtualMachine *vm, Lexer *lexer, char *fmt, ...) {
	// Format the error message
	char *message = malloc(sizeof(char) * MAX_ERROR_LENGTH);
	va_list args;
	va_start(args, fmt);
	vsnprintf(message, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);

	set_error(vm, lexer, message);
}


// Shortcut for defining the contents of a token.
#define SYMBOL(token, content)           \
	case token:                          \
		strcpy(string, content);         \
		return &string[strlen(content)]; \


// Appends the textual representation of a token to a
// string.
static char * print_token(char *string, Token token, TokenValue value) {
	switch (token) {
	// Mathematical operators
	SYMBOL(TOKEN_ADD, "+")
	SYMBOL(TOKEN_SUB, "-")
	SYMBOL(TOKEN_MUL, "*")
	SYMBOL(TOKEN_DIV, "/")
	SYMBOL(TOKEN_MOD, "%%")

	// Comparison operators
	SYMBOL(TOKEN_EQ, "==")
	SYMBOL(TOKEN_NEQ, "!=")
	SYMBOL(TOKEN_LT, "<")
	SYMBOL(TOKEN_LE, "<=")
	SYMBOL(TOKEN_GT, ">")
	SYMBOL(TOKEN_GE, ">=")

	// Assignment operators
	SYMBOL(TOKEN_ASSIGN, "=")
	SYMBOL(TOKEN_ADD_ASSIGN, "+=")
	SYMBOL(TOKEN_SUB_ASSIGN, "-=")
	SYMBOL(TOKEN_MUL_ASSIGN, "*=")
	SYMBOL(TOKEN_DIV_ASSIGN, "/=")
	SYMBOL(TOKEN_MOD_ASSIGN, "%=")

	// Boolean operators
	SYMBOL(TOKEN_AND, "&&")
	SYMBOL(TOKEN_OR, "||")
	SYMBOL(TOKEN_NOT, "!")

	// Bitwise operators
	SYMBOL(TOKEN_BIT_AND, "&")
	SYMBOL(TOKEN_BIT_OR, "|")
	SYMBOL(TOKEN_BIT_XOR, "^")
	SYMBOL(TOKEN_BIT_NOT, "~")

	// Syntax
	SYMBOL(TOKEN_OPEN_PARENTHESIS, "(")
	SYMBOL(TOKEN_CLOSE_PARENTHESIS, ")")
	SYMBOL(TOKEN_OPEN_BRACKET, "[")
	SYMBOL(TOKEN_CLOSE_BRACKET, "]")
	SYMBOL(TOKEN_OPEN_BRACE, "{")
	SYMBOL(TOKEN_CLOSE_BRACE, "}")
	SYMBOL(TOKEN_COMMA, ",")
	SYMBOL(TOKEN_DOT, ".")

	// Values
	case TOKEN_IDENTIFIER: {
		int written = snprintf(string, value.identifier.length, "%s",
			value.identifier.start);
		return &string[written];
	}

	case TOKEN_STRING: {
		int written = snprintf(string, value.identifier.length, "\"%s\"",
			value.identifier.start);
		return &string[written];
	}

	case TOKEN_INTEGER: {
		int written = sprintf(string, "%d", value.integer);
		return &string[written];
	}

	case TOKEN_NUMBER: {
		int written = sprintf(string, "%f", value.number);
		return &string[written];
	}

	SYMBOL(TOKEN_TRUE, "true")
	SYMBOL(TOKEN_FALSE, "false")
	SYMBOL(TOKEN_NIL, "nil")

	// Keywords
	SYMBOL(TOKEN_IF, "if")
	SYMBOL(TOKEN_ELSE_IF, "else if")
	SYMBOL(TOKEN_ELSE, "else")
	SYMBOL(TOKEN_WHILE, "while")
	SYMBOL(TOKEN_LOOP, "loop")
	SYMBOL(TOKEN_FOR, "for")
	SYMBOL(TOKEN_FN, "fn")

	// Invalid token
	default:
		return string;
	}
}


// Triggers an unexpected token error on the VM.
void err_unexpected(VirtualMachine *vm, Lexer *lexer, char *fmt, ...) {
	// Format the message
	char *message = malloc(sizeof(char) * MAX_ERROR_LENGTH);
	char *start = message;
	int next;
	va_list args;

	// Add the given message
	va_start(args, fmt);
	next = vsprintf(message, fmt, args);
	message = &message[next];
	va_end(args);

	// Add the static text
	next = sprintf(message, ", found `");
	message = &message[next];

	// Add the token
	// TODO: Prevent overflow of max error length when
	// writing a token
	message = print_token(message, lexer->token, lexer->value);

	// Finish the static text
	sprintf(message, "`");

	set_error(vm, lexer, start);
}
