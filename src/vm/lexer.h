
//
// Lexer
//

#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>


// All possible token types.
typedef enum {
	// Mathematical operators
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_MOD,
	TOKEN_CONCAT,

	// Comparison operators
	TOKEN_EQ,
	TOKEN_NEQ,
	TOKEN_LT,
	TOKEN_LE,
	TOKEN_GT,
	TOKEN_GE,

	// Assignment operators
	TOKEN_ASSIGN,
	TOKEN_ADD_ASSIGN,
	TOKEN_SUB_ASSIGN,
	TOKEN_MUL_ASSIGN,
	TOKEN_DIV_ASSIGN,

	// Boolean operators
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_NOT,

	// Bitwise operators
	TOKEN_BIT_AND,
	TOKEN_BIT_OR,
	TOKEN_BIT_XOR,
	TOKEN_BIT_NOT,

	// Syntax
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_OPEN_BRACKET,
	TOKEN_CLOSE_BRACKET,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_COMMA,
	TOKEN_DOT,

	// Values
	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_INTEGER,
	TOKEN_NUMBER,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NIL,

	// Keywords
	TOKEN_IF,
	TOKEN_ELSE_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_LOOP,
	TOKEN_BREAK,
	TOKEN_FOR,
	TOKEN_LET,
	TOKEN_FN,
	TOKEN_RETURN,
	TOKEN_IMPORT,
	TOKEN_STRUCT,
	TOKEN_NEW,

	// Other
	TOKEN_EOF,
	TOKEN_UNRECOGNISED,
} Token;


// An identifier within the source code.
typedef struct {
	// A pointer to the start of the identifier.
	char *start;

	// The length of the identifier.
	size_t length;
} Identifier;


// The value of a token.
typedef union {
	// A number.
	double number;

	// An integer.
	int16_t integer;

	// An identifier or string literal.
	Identifier identifier;
} TokenValue;


// The lexer, which separates source code into tokens.
typedef struct {
	// A pointer to the original source code.
	char *source;

	// The current cursor location in the source code.
	uint32_t cursor;

	// The most recently parsed token.
	Token token;
	TokenValue value;
} Lexer;


// Creates a new lexer.
Lexer lexer_new(char *source);

// Parses the next token.
void lexer_next(Lexer *lexer);

// Extracts a string from the given identifier. Returns a heap allocated string
// that needs to be freed.
//
// Returns NULL if the string contains an invalid escape sequence.
char * lexer_extract_string(Identifier identifier);

// Returns the current source code line of the lexer.
uint32_t lexer_line(Lexer *lexer);

#endif
