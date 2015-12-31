
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
	TOKEN_LEFT_SHIFT,
	TOKEN_RIGHT_SHIFT,

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
} TokenType;


// A token parsed by the lexer.
typedef struct {
	// The type of the token.
	TokenType type;

	// The starting location of the token in the source code.
	char *start;

	// The length of the token.
	size_t length;

	// The line the token was defined on.
	uint32_t line;

	// The column the token starts at.
	uint32_t column;

	// A pointer to the start of the line the token was defined on.
	char *line_start;

	// The name of the file the token was defined in, or NULL if the token was
	// lexed from a string.
	char *file;

	// The name of the package the token was defined in.
	char *package;

	// The value of the token if it's a number.
	union {
		double number;
		int16_t integer;
	};
} Token;


// The lexer, which separates source code into tokens.
typedef struct {
	// A pointer to the original source code.
	char *source;

	// The current cursor location in the source code.
	char *cursor;

	// The most recently lexed token.
	Token token;
} Lexer;


// Creates a new lexer. The file name is used for error messages.
Lexer lexer_new(char *file, char *package, char *source);

// Parses the next token.
void lexer_next(Lexer *lexer);

// Extracts a string from the given token. Returns a heap allocated string
// that needs to be freed. Triggers an error if the string contains an invalid
// escape sequence.
char * lexer_extract_string(Token *token);

#endif
