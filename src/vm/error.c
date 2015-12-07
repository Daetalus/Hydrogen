
//
//  Error
//

#include <stdio.h>
#include <string.h>

#include "error.h"


// The maximum number of characters the description of an error can be.
#define MAX_ERROR_LENGTH 1024


// Returns the textual representation of a token as a heap allocated string.
char * token_string(Token token, TokenValue value);


// Returns a new, custom error.
HyError err_new(int line, char *fmt, ...) {
	HyError err;
	err.line = line;

	// Allocate space for a description
	err.description = malloc(sizeof(char) * MAX_ERROR_LENGTH);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(err.description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);
	return err;
}


// Returns an unexpected token error.
HyError err_unexpected(int line, Token token, TokenValue value, char *fmt, ...) {
	HyError err;
	err.line = line;

	// Create the description
	err.description = malloc(MAX_ERROR_LENGTH * sizeof(char));
	char *description = err.description;

	// `fmt` and arguments
	va_list args;
	va_start(args, fmt);
	int length = vsnprintf(description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);
	description = &description[length];

	// `, found ...`
	length = sprintf(description, ", found `");
	description = &description[length];

	// Token
	char *str_token = token_string(token, value);
	strcpy(description, str_token);
	free(str_token);
	description = &description[strlen(str_token)];

	// Final backtick
	sprintf(description, "`");
	return err;
}


// Frees an error.
void err_free(HyError *err) {
	if (err->description != NULL) {
		free(err->description);
	}
}



//
//  Token to String
//

// Converts a token with no associated data to `text`.
#define SYMBOL(token, text)                                 \
	case token: {                                           \
		char *result = malloc(strlen(text) * sizeof(char)); \
		strcpy(result, text);                               \
		return result;                                      \
	}


// Returns the textual representation of a token as a heap allocated string.
char * token_string(Token token, TokenValue value) {
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
	SYMBOL(TOKEN_LET, "let")
	SYMBOL(TOKEN_FN, "fn")
	SYMBOL(TOKEN_IMPORT, "import")

	case TOKEN_IDENTIFIER: {
		char *name = value.identifier.start;
		size_t length = value.identifier.length;
		char *result = malloc((length + 1) * sizeof(char));
		sprintf(result, "%.*s", (int) length, name);
		return result;
	}

	case TOKEN_STRING: {
		char *extracted = lexer_extract_string(value.identifier);
		char *result = malloc((strlen(extracted) + 3) * sizeof(char));
		sprintf(result, "'%s'", extracted);
		free(extracted);
		return result;
	}

	case TOKEN_INTEGER: {
		// Maximum int16_t is 32767 (5 characters), plus one for the sign (if
		// it's negative), plus one for the NULL terminator
		char *result = malloc(7 * sizeof(char));
		sprintf(result, "%d", value.integer);
		return result;
	}

	case TOKEN_NUMBER: {
		// Just assume the maximum size of a double is 20 characters long
		char *result = malloc(20 * sizeof(char));
		sprintf(result, "%.5f", value.number);
		return result;
	}

	// Unrecognised token
	default: {
		char *message = "Unrecognised token";
		char *result = malloc((strlen(message) + 1) * sizeof(char));
		strcpy(result, message);
		return result;
	}
	}
}
