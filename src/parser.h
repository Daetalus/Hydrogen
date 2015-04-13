
//
//  Parser
//


#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdbool.h>

#include "lib/str.h"


// Sequentially extracts parts of the source code to help the
// lexer generate tokens.
typedef struct {
	// The source string being navigated.
	char *source;

	// The length of the source string.
	int length;

	// The current cursor position in the source string.
	int cursor;

	// The saved cursor position.
	int saved;
} Parser;


// Returns true if the character is whitespace.
bool is_whitespace(char c);

// Returns true if the character is a newline.
bool is_newline(char c);

// Returns true if the character is a space or tab.
bool is_space_or_tab(char c);

// Returns true if the character is a digit.
bool is_digit(char c);

// Returns true if the character can act as the first character
// in an identifier.
bool is_identifier_start(char c);

// Returns true if the character can exist as part of an
// identifier.
bool is_identifier(char c);

// Returns true if the character can act as a quotation mark,
// starting and ending a string literal.
bool is_quotation_mark(char c);


// Create a new parser with `source`.
Parser parser_new(char *source);

// Returns true if the parser has reached the end of the source
// code file.
bool parser_is_eof(Parser *parser);

// Returns a pointer to the character at the current cursor
// position.
char * parser_ptr(Parser *parser);

// Returns the character at the current cursor position.
char parser_current(Parser *parser);

// Saves the current cursor location for restoration at a later
// point.
void parser_save(Parser *parser);

// Restores the cursor position to the saved location, or to 0
// if `parser_save` hasn't been called.
void parser_restore(Parser *parser);

// Consumes a character and increments the cursor position.
void parser_consume(Parser *parser);

// Returns the character `amount` characters in front of the
// current cursor position.
//
// Returns a NULL terminator if the requested character is past
// the end of the source string.
// Returns the first character in the source if the requested
// character is before the start of the source string.
char parser_peek(Parser *parser, int amount);

// Moves the cursor to `position`.
void parser_move_to(Parser *parser, int position);

// Moves the cursor forward or backward by `amount`.
void parser_move(Parser *parser, int32_t amount);

// Returns true if the parser starts with `str`.
bool parser_starts_with(Parser *parser, char *str, int length);

// Returns true if the parser starts with `str` followed by a
// non-identifier character.
bool parser_starts_with_identifier(Parser *parser, char *str, int length);

// Consumes characters until a non-whitespace character is
// encountered.
// Returns the number of whitespace characters consumed.
int parser_consume_whitespace(Parser *parser);

// Consumes characters until a character that isn't a space or
// tab is encountered.
void parser_consume_spaces_tabs(Parser *parser);

// Consumes an identifier, returning a pointer to the start of
// the identifier in the source code.
// Returns the length of the identifier through the `length`
// pointer.
char * parser_consume_identifier(Parser *parser, int *length);

// Consumes a number, returning it.
// The length of the number is returned through the `length`
// argument.
double parser_consume_number(Parser *parser, int *length);

// Consumes a string literal, returning a pointer to the
// character after the opening quote.
// Returns the length of the literal through the `length`
// argument.
//
// This function returns the string literal as it is in the
// source code. This will contain improper escape sequences,
// eg. `\n` will exist as a `\` followed by an `n`. Extract the
// actual string using `parser_extract_literal`.
char * parser_consume_literal(Parser *parser, int *length);

// Extracts a string literal returned by
// `parser_consume_literal`.
//
// If the literal contains an invalid escape sequence, then its
// position is returned through `invalid_escape_ptr`.
String * parser_extract_literal(char *str, int length,
	char **invalid_escape_ptr);

#endif
