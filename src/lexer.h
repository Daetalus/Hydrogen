
//
//  Lexer
//


#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#include "value.h"


// The maximum number of tokens that the lexer can store in
// its token cache.
#define MAX_TOKEN_CACHE_SIZE 16


// All token types produced by the lexer.
typedef enum {
	// Mathematical Operators
	TOKEN_ADDITION,
	TOKEN_SUBTRACTION,
	TOKEN_MULTIPLICATION,
	TOKEN_DIVISION,
	TOKEN_MODULO,

	// Not actually produced by the lexer (all `-` are returned
	// as `TOKEN_SUBTRACTION`) but since the token type enum is
	// also used to represent operators, we need to include this
	// here.
	TOKEN_NEGATION,

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

	// Constants
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,

	// Used for string literals. The location and size of this
	// token is for the string literal itself, and doesn't
	// include the quotes enclosing the literal.
	TOKEN_STRING,

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
	TOKEN_IN,
	TOKEN_ELSE,
	TOKEN_ELSE_IF,
	TOKEN_FOR,
	TOKEN_WHILE,
	TOKEN_LOOP,
	TOKEN_FUNCTION,
	TOKEN_CLASS,

	// Other
	TOKEN_LINE,
	TOKEN_END_OF_FILE,
	TOKEN_NONE,
} TokenType;


// Operator associativity.
typedef enum {
	ASSOCIATIVITY_LEFT,
	ASSOCIATIVITY_RIGHT,
} Associativity;


// A token produced by the lexer.
typedef struct {
	// The type of the token.
	TokenType type;

	// A number used when the parsed token is a number.
	// Undefined if the token isn't a number.
	double number;

	// A pointer into the source code specifying the start
	// location of the token.
	char *location;

	// The length of the token in the source.
	int length;
} Token;


// A parser.
//
// Stores a cursor position within a source string, allowing
// navigation within the string.
typedef struct {
	// The source string being navigated.
	char *source;

	// The length of the source string.
	int length;

	// The cursor position within the source string.
	int cursor;
} Parser;


// The lexer.
typedef struct {
	// A parser providing cursor functionality within the source
	// code.
	Parser parser;

	// The line number of the current token.
	int line;

	// A cache of tokens built up when using the peek function
	// to look ahead.
	Token cache[MAX_TOKEN_CACHE_SIZE];

	// The number of items in the cache.
	int cache_size;

	// Whether to ignore newlines.
	// Defaults to false.
	bool should_ignore_newlines;
} Lexer;


// Create a new lexer with the given source.
void lexer_new(Lexer *lexer, char *source);

// Consumes a token, moving to the next one.
Token consume(Lexer *lexer);

// Peeks at a token further ahead in the source code, without
// advancing the cursor.
Token peek(Lexer *lexer, int amount);

// Returns true if the lexer starts with the given token type.
bool match(Lexer *lexer, TokenType token);

// Returns true if the lexer starts with the two given tokens.
bool match_double(Lexer *lexer, TokenType one, TokenType two);

// Extracts a string literal pointed to by the given token
// from the source code, populating `string` (assumed to be
// unallocated.
// If the string contains an invalid escape sequence, returns
// a pointer to the start of the escape sequence, else returns
// NULL.
char * extract_string_literal(Token *literal, String *string);

// Tells the lexer to not emit any newline tokens.
void ignore_newlines(Lexer *lexer);

// Tells the lexer to emit newline tokens.
void obey_newlines(Lexer *lexer);

#endif
