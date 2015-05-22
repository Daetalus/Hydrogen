
//
//  Lexer
//


#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"


// Token types emitted by the lexer.
typedef enum {
	// Mathematical Operators
	TOKEN_ADDITION,
	TOKEN_SUBTRACTION,
	TOKEN_MULTIPLICATION,
	TOKEN_DIVISION,
	TOKEN_MODULO,

	// Boolean Operators
	TOKEN_BOOLEAN_AND,
	TOKEN_BOOLEAN_OR,
	TOKEN_BOOLEAN_NOT,
	TOKEN_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_LESS_THAN,
	TOKEN_LESS_THAN_EQUAL_TO,
	TOKEN_GREATER_THAN,
	TOKEN_GREATER_THAN_EQUAL_TO,

	// Bitwise Operators
	TOKEN_LEFT_SHIFT,
	TOKEN_RIGHT_SHIFT,
	TOKEN_BITWISE_AND,
	TOKEN_BITWISE_OR,
	TOKEN_BITWISE_NOT,
	TOKEN_BITWISE_XOR,

	// Assignment
	TOKEN_ASSIGNMENT,
	TOKEN_ADDITION_ASSIGNMENT,
	TOKEN_SUBTRACTION_ASSIGNMENT,
	TOKEN_MULTIPLICATION_ASSIGNMENT,
	TOKEN_DIVISION_ASSIGNMENT,
	TOKEN_MODULO_ASSIGNMENT,

	// Syntax
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_OPEN_BRACKET,
	TOKEN_CLOSE_BRACKET,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_DOT,
	TOKEN_COMMA,

	// Keywords
	TOKEN_LET,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_ELSE_IF,
	TOKEN_WHILE,
	TOKEN_LOOP,
	TOKEN_BREAK,
	TOKEN_FOR,
	TOKEN_IN,
	TOKEN_CLASS,
	TOKEN_NEW,
	TOKEN_FUNCTION,
	TOKEN_RETURN,
	TOKEN_SELF,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NIL,

	// Other
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_STRING,
	TOKEN_LINE,
	TOKEN_END_OF_FILE,
	TOKEN_NONE,
} TokenType;


// A token emitted by the lexer.
typedef struct {
	// The type of the token.
	TokenType type;

	// The numerical value of a number token (`TOKEN_NUMBER`).
	double number;

	// A pointer into the source code specifying the start
	// location of the token.
	char *location;

	// The length of the token in the source.
	int length;
} Token;


// The maximum number of tokens that the lexer can store in its
// cache.
#define MAX_TOKEN_CACHE_SIZE 16


// Emits a sequence of tokens from a source string.
typedef struct {
	// The source code parser. See `parser.h`.
	Parser parser;

	// The line number of the current token.
	int line;

	// The lexer cannot arbitrarily pick out tokens to return,
	// we must parse them sequentially. So if we want to look
	// ahead (with peek), we need to cache the tokens before the
	// one we're looking for.
	Token cache[MAX_TOKEN_CACHE_SIZE];

	// The number of items in the cache.
	int cache_size;

	// Whether to emit newlines. Defaults to true.
	bool emit_newlines;
} Lexer;


// Create a new lexer with `source`.
Lexer lexer_new(char *source);

// Consumes a token, returning it.
Token lexer_consume(Lexer *lexer);

// Returns the token `amount` tokens in front of the current
// one.
Token lexer_peek(Lexer *lexer, int amount);

// Returns the current token without consuming anything.
Token lexer_current(Lexer *lexer);

// Returns true if the lexer starts with `token`.
bool lexer_match(Lexer *lexer, TokenType token);

// Returns true if the next two tokens are `one` and `two`.
bool lexer_match_two(Lexer *lexer, TokenType one, TokenType two);

// Tells the lexer to not emit any newline tokens.
void lexer_disable_newlines(Lexer *lexer);

// Tells the lexer to emit newline tokens.
void lexer_enable_newlines(Lexer *lexer);

#endif
