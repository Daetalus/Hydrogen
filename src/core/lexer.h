
//
//  Lexer
//

#ifndef LEXER_H
#define LEXER_H

#include <hydrogen.h>
#include <stdint.h>

#include <vec.h>


// All possible token types
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
	TOKEN_MOD_ASSIGN,

	// Boolean operators
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_NOT,

	// Bitwise operators
	TOKEN_BIT_AND,
	TOKEN_BIT_OR,
	TOKEN_BIT_XOR,
	TOKEN_BIT_NOT,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,

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
	TOKEN_SELF,

	// Comments
	// Not actually emitted by the lexer, but used in generating error messages
	// that relate to comments (like unterminated block comment error)
	TOKEN_COMMENT,

	// Other
	TOKEN_EOF,
	TOKEN_UNRECOGNISED,
} TokenType;


// A token emitted by the lexer
typedef struct {
	// The type of the token
	TokenType type;

	// The location and length of the token in the source code
	char *start;
	uint32_t length;

	// The package and source code index in the package the token is located in
	Index package;
	Index source;

	// The value of a token if it's a number or integer
	union {
		double number;
		int16_t integer;
	};
} Token;


// A lexer, which converts source code into a stream of tokens
typedef struct {
	// The interpreter state the lexer was created on
	HyState *state;

	// A pointer to the start of the source code we are lexing
	char *source;

	// The current cursor position and line number in the source code
	char *cursor;
	uint32_t line;

	// The most recently lexed token, which is updated every time the
	// `lexer_next` function is called
	Token token;
} Lexer;


// Create a new lexer on an interpreter state in the package `pkg`, lexing the
// source code at `source`
Lexer lexer_new(HyState *state, Index pkg_index, Index src_index);

// Lex the next token in the source code
void lexer_next(Lexer *lexer);

// String literals need to be extracted from a token separately because escape
// sequences need to be parsed into their proper values. Stores the extracted
// string directly into `buffer`. Ensure that `buffer` is at least as long as
// token->length - 1 (since the token length includes the two surrounding
// quotes). Returns the length of the parsed string
uint32_t lexer_extract_string(Lexer *lexer, Token *token, char *buffer);



//
//  Character Groupings
//

// Returns true if a character is a newline
static inline bool is_newline(char ch) {
	return ch == '\n' || ch == '\r';
}


// Returns true if a character is whitespace
static inline bool is_whitespace(char ch) {
	return is_newline(ch) || ch == ' ' || ch == '\t';
}


// Returns true if a character is a decimal digit
static inline bool is_decimal(char ch) {
	return ch >= '0' && ch <= '9';
}


// Returns true if a character is a hexadecimal digit
static inline bool is_hex(char ch) {
	return is_decimal(ch) || (ch >= 'a' && ch <= 'f');
}


// Returns true if a character can start an identifier
static inline bool is_identifier_start(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}


// Returns true if a character can be part of an identifier
static inline bool is_identifier(char ch) {
	return is_identifier_start(ch) || is_decimal(ch);
}

#endif
