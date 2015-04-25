
//
//  Parser
//


#include <string.h>
#include <stdio.h>

#include "parser.h"


// Returns true if the character is whitespace.
bool is_whitespace(char c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}


// Returns true if the character is a newline.
bool is_newline(char c) {
	return c == '\n' || c == '\r';
}


// Returns true if the character is a space or tab.
bool is_space_or_tab(char c) {
	return c == ' ' || c == '\t';
}


// Returns true if the character is a digit.
bool is_digit(char c) {
	return c >= '0' && c <= '9';
}


// Returns true if the character can act as the first character
// in an identifier.
bool is_identifier_start(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


// Returns true if the character can exist as part of an
// identifier.
bool is_identifier(char c) {
	return is_identifier_start(c) || is_digit(c);
}


// Returns true if the character can act as a quotation mark,
// starting and ending a string literal.
bool is_quotation_mark(char c) {
	return c == '\'' || c == '"';
}



//
//  Parser
//

// Create a new parser with `source`.
Parser parser_new(char *source) {
	Parser parser;
	parser.source = source;
	parser.length = strlen(source);
	parser.cursor = 0;
	return parser;
}


// Returns true if the parser has reached the end of the source
// code file.
bool parser_is_eof(Parser *parser) {
	return parser->source[parser->cursor] == '\0';
}


// Returns a pointer to the character at the current cursor
// position.
char * parser_ptr(Parser *parser) {
	return &parser->source[parser->cursor];
}


// Returns the character at the current cursor position.
char parser_current(Parser *parser) {
	return parser->source[parser->cursor];
}


// Saves the current cursor location for restoration at a later
// point.
void parser_save(Parser *parser) {
	parser->saved = parser->cursor;
}


// Restores the cursor position to the saved location, or to 0
// if `parser_save` hasn't been called.
void parser_restore(Parser *parser) {
	parser->cursor = parser->saved;
}


// Consumes a character and increments the cursor position.
void parser_consume(Parser *parser) {
	if (!parser_is_eof(parser)) {
		parser->cursor++;
	}
}


// Returns the character `amount` characters in front of the
// current cursor position.
//
// Returns a NULL terminator if the requested character is past
// the end of the source string.
// Returns the first character in the source if the requested
// character is before the start of the source string.
char parser_peek(Parser *parser, int amount) {
	if (parser->cursor + amount >= parser->length) {
		// Overflow
		return '\0';
	} else {
		return parser->source[parser->cursor + amount];
	}
}



// Moves the cursor to `position`.
void parser_move_to(Parser *parser, int position) {
	if (position >= parser->length) {
		// Overflow
		position = parser->length - 1;
	}

	parser->cursor = position;
}


// Moves the cursor forward or backward by `amount`.
void parser_move(Parser *parser, int32_t amount) {
	if (parser->cursor + amount > parser->length) {
		// Overflow
		parser->cursor = parser->length;
	} else {
		parser->cursor += amount;
	}
}


// Returns true if the parser starts with `string`.
bool parser_starts_with(Parser *parser, char *str, int length) {
	for (int i = 0; i < length && str[i] != '\0'; i++) {
		if (str[i] != parser_peek(parser, i)) {
			return false;
		}
	}

	return true;
}


// Returns true if the parser starts with `str` followed by a
// non-identifier character.
bool parser_starts_with_identifier(Parser *parser, char *str, int length) {
	return parser_starts_with(parser, str, length) &&
		!is_identifier(parser_peek(parser, length));
}


// Consumes characters until a non-whitespace character is
// encountered.
// Returns the number of whitespace characters consumed.
int parser_consume_whitespace(Parser *parser) {
	int count = 0;
	while (!parser_is_eof(parser) && is_whitespace(parser_current(parser))) {
		parser_consume(parser);
		count++;
	}
	return count;
}


// Consumes characters until a character that isn't a space or
// tab is encountered.
void parser_consume_spaces_tabs(Parser *parser) {
	while (!parser_is_eof(parser) && is_space_or_tab(parser_current(parser))) {
		parser_consume(parser);
	}
}


// Consumes an identifier, returning a pointer to the start of
// the identifier in the source code.
// Returns the length of the identifier through the `length`
// pointer.
char * parser_consume_identifier(Parser *parser, int *length) {
	*length = 0;

	if (!is_identifier_start(parser_current(parser))) {
		return NULL;
	}

	char *start = parser_ptr(parser);
	while (!parser_is_eof(parser) && is_identifier(parser_current(parser))) {
		parser_consume(parser);
		(*length)++;
	}

	return start;
}


// Consumes a number, returning it.
// The length of the number is returned through the `length`
// argument.
double parser_consume_number(Parser *parser, int *length) {
	double result = 0.0;

	int base = 10;
	if (parser_starts_with(parser, "0x", 2)) {
		// Hexadecimal
		base = 16;
		parser_move(parser, 2);
	} else if (parser_starts_with(parser, "0o", 2)) {
		// Octal
		base = 8;
		parser_move(parser, 2);
	}

	char *end;
	char *start = parser_ptr(parser);
	if (base != 10) {
		// Octal or hexadecimal can only exist as integers, so
		// parse as a long.
		result = (double) strtol(start, &end, base);
	} else {
		result = strtod(start, &end);
	}

	if (start == end) {
		// Not a number
		*length = 0;
		return 0.0;
	}

	*length = end - start;
	parser->cursor = end - parser->source;
	return result;
}


// Consumes a string literal, returning a pointer to the
// character after the opening quote.
// Returns the length of the literal through the `length`
// argument.
//
// This function returns the string literal as it is in the
// source code. This will contain improper escape sequences,
// eg. `\n` will exist as a `\` followed by an `n`. Extract the
// actual string using `parser_extract_literal`.
char * parser_consume_literal(Parser *parser, int *length) {
	*length = 0;

	// Opening quote
	char quote = parser_current(parser);
	if (quote != '\'' && quote != '"') {
		// No opening quote
		return NULL;
	}

	parser_consume(parser);
	char *start = parser_ptr(parser);

	// Consume characters until we encounter the closing quote.
	bool was_escape = false;
	while (!parser_is_eof(parser)) {
		if (!was_escape && parser_current(parser) == quote) {
			// Found the closing quote
			break;
		}

		// Check for an escaping backslash.
		if (parser_current(parser) == '\\') {
			was_escape = true;
		} else {
			was_escape = false;
		}
		parser_consume(parser);

		(*length)++;
	}

	if (parser_is_eof(parser)) {
		// Reached end of file without finding a closing quote.
		*length = 0;
		return NULL;
	}

	// Consume the closing quote.
	parser_consume(parser);

	return start;
}


// Extracts a string literal returned by
// `parser_consume_literal`.
//
// If the literal contains an invalid escape sequence, then its
// position is returned through `invalid_escape_ptr`.
String * parser_extract_literal(char *str, int length,
		char **invalid_escape_ptr) {
	// Allocate the literal's length as the capacity for the new
	// string, as it can't get any longer.
	*invalid_escape_ptr = NULL;
	String *result = string_new(length + 1);

	for (int i = 0; i < length; i++) {
		char ch = str[i];
		if (ch == '\\') {
			// Skip the backslash.
			i++;
			ch = str[i];

			// Insert the escape sequence.
			if (ch == 'n') {
				string_append_char(result, '\n');
			} else if (ch == 'r') {
				string_append_char(result, '\r');
			} else if (ch == 't') {
				string_append_char(result, '\t');
			} else if (ch == '\'') {
				string_append_char(result, '\'');
			} else if (ch == '"') {
				string_append_char(result, '"');
			} else if (ch == '\\') {
				string_append_char(result, '\\');
			} else {
				// No escape sequence was found, so return an
				// error.
				*invalid_escape_ptr = &str[i - 1];
				return NULL;
			}
		} else {
			// It's a normal character, so just insert it.
			string_append_char(result, ch);
		}
	}

	return result;
}
