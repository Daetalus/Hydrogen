
//
//  Error
//

#include <stdio.h>
#include <string.h>

#include "error.h"


// The maximum number of characters an error message can be.
#define MAX_ERROR_LENGTH 1024


// Returns the textual representation of the lexer's most recent token as a
// heap allocated string. Returns NULL for an invalid token.
char * token_string(Lexer *lexer);


// Create a new error.
void err_new(HyError *err, int line, char *fmt, ...) {
	err->line = line;

	// Allocate space for a new description
	err->description = malloc(sizeof(char) * MAX_ERROR_LENGTH);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(err->description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);
}


// Free an error.
void err_free(HyError *err) {
	if (err->description != NULL) {
		free(err->description);
	}
}


// Create an unexpected token error for the most recent token on the lexer.
void err_unexpected(HyError *err, Lexer *lexer, char *fmt, ...) {
	err->line = lexer_line(lexer);

	// Create the description
	err->description = malloc(MAX_ERROR_LENGTH * sizeof(char));
	char *description = err->description;

	// Add the given message
	va_list args;
	va_start(args, fmt);
	int length = vsnprintf(description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);
	description = &description[length];

	// Add the `, found ...` part
	length = sprintf(description, ", found `");
	description = &description[length];

	// Print the token
	char *token = token_string(lexer);
	strcpy(description, token);
	free(token);
	description = &description[strlen(token)];

	// Add the final closing backtick
	sprintf(description, "`");
}



//
//  Token to String Conversion
//

// Handles a case in converting a token to a string.
#define SYMBOL(token, text)                                 \
	case token: {                                           \
		char *result = malloc(strlen(text) * sizeof(char)); \
		strcpy(result, text);                               \
		return result;                                      \
	}


// Returns the textual representation of the lexer's most recent token as a
// heap allocated string. Returns NULL for an invalid token.
char * token_string(Lexer *lexer) {
	switch (lexer->token) {
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

	// Values
	case TOKEN_IDENTIFIER: {
		Identifier *ident = &lexer->value.identifier;
		char *result = malloc((ident->length + 1) * sizeof(char));
		snprintf(result, ident->length, "%.*s", (int) ident->length,
			ident->start);
		return result;
	}

	case TOKEN_STRING: {
		Identifier *ident = &lexer->value.identifier;
		char *result = malloc((ident->length + 3) * sizeof(char));
		snprintf(result, ident->length, "'%.*s'", (int) ident->length,
			ident->start);
		return result;
	}

	case TOKEN_INTEGER: {
		// 7 characters as maximum int16_t is 32767 (5 characters), plus one
		// for the sign (if it's negative), plus one for the NULL terminator
		char *result = malloc(7 * sizeof(char));
		sprintf(result, "%d", lexer->value.integer);
		return result;
	}

	case TOKEN_NUMBER: {
		char *result = malloc(20 * sizeof(char));
		sprintf(result, "%.5f", lexer->value.number);
		return result;
	}

	// Unrecognised token
	default:
		return NULL;
	}
}
