
//
//  Lexer
//


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"


//
//  Classification
//

bool is_whitespace(char c) {
	return c == ' ' || c == '\n' || c == '\r' || c == '\r';
}


bool is_space_or_tab(char c) {
	return c == ' ' || c == '\r';
}


bool is_digit(char c) {
	return c >= '0' && c <= '9';
}


bool is_identifier_start(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


bool is_identifier(char c) {
	return is_identifier_start(c) || is_digit(c);
}



//
//  Parser
//

// Create a new parser from the given source string.
void parser_new(Parser *parser, char *source) {
	parser->source = source;
	parser->cursor = 0;
	parser->length = strlen(source);
}


// Returns true if the cursor is at the end of the
// source string.
int is_eof(Parser *parser) {
	return parser->source[parser->cursor] == '\0';
}


// Consume a character, returning it.
// A NULL terminator byte is returned if the cursor
// position is beyond the end of the file.
char consume_char(Parser *parser) {
	if (!is_eof(parser)) {
		parser->cursor++;
		return parser->source[parser->cursor - 1];
	} else {
		return '\0';
	}
}


// Move the cursor forward or backward (with a negative
// number) by an amount.
void move_cursor(Parser *parser, int amount) {
	if (parser->cursor + amount < 0) {
		// Handle underflow
		parser->cursor = 0;
	} else if (parser->cursor + amount >= parser->length) {
		// Handle overflow
		parser->cursor = parser->length - 1;
	} else {
		// Within range
		parser->cursor += amount;
	}
}


// Peek at a character further ahead in the source.
// A NULL terminator byte is returned if the cursor
// position is beyond the end of the file.
char peek_char(Parser *parser, int amount) {
	if (parser->cursor + amount < 0) {
		// Underflow
		return '\0';
	} else if (parser->cursor + amount >= parser->length) {
		// Overflow
		return '\0';
	} else {
		// Within range
		return parser->source[parser->cursor + amount];
	}
}


// Returns true if the parser starts with the given
// string.
bool starts_with(Parser *parser, char *string, int length) {
	for (int i = 0; i < length && string[i] != '\0'; i++) {
		if (string[i] != peek_char(parser, i)) {
			return false;
		}
	}

	return true;
}


// Continually consume characters until the given
// terminator string is found, or the end of file
// is reached.
void consume_until(Parser *parser, char *terminator, int length) {
	while (!is_eof(parser) && !starts_with(parser, terminator, length)) {
		consume_char(parser);
	}
}


// Continually consume characters until a
// non-whitespace character or end of file is
// reached.
void consume_whitespace(Parser *parser) {
	while (!is_eof(parser) && is_whitespace(peek_char(parser, 0))) {
		consume_char(parser);
	}
}


// Continually consume characters until a character
// that isn't a space or tab is found, or the end
// of the file is reached.
void consume_spaces_tabs(Parser *parser) {
	while (!is_eof(parser) && is_space_or_tab(peek_char(parser, 0))) {
		consume_char(parser);
	}
}


// Consumes an identifier. The identifier's length
// is returned, and its starting position within the
// source code is returned through the int pointer
// argument.
char * consume_identifier(Parser *parser, int *length) {
	*length = 0;

	if (is_identifier_start(peek_char(parser, 0))) {
		char *start = &parser->source[parser->cursor];
		while (!is_eof(parser) && is_identifier(peek_char(parser, 0))) {
			(*length)++;
		}

		return start;
	}

	return NULL;
}


// Consumes a number. Returns true if successful, or false
// if the parser does not contain a valid number under the
// cursor. The number is returned through the provided
// argument. Nothing is consumed if the function fails.
bool consume_number(Parser *parser, double *result) {
	*result = 0.0;

	int base = 10;
	if (starts_with(parser, "0x", 2)) {
		// Hexadecimal
		base = 16;
		move_cursor(parser, 2);
	} else if (starts_with(parser, "0o", 2)) {
		// Octal
		base = 8;
		move_cursor(parser, 2);
	}

	// Read in the value
	char *start = &parser->source[parser->cursor];
	char *end;
	if (base != 10) {
		*result = (double) strtol(start, &end, base);
	} else {
		*result = strtod(start, &end);
	}

	parser->cursor = end - start;

	if (start == end) {
		// Invalid number.
		*result = 0;
		return false;
	}

	return true;
}



//
//  Lexer
//

// Create a new lexer with the given source.
void lexer_new(Lexer *lexer, char *source) {
	parser_new(lexer->parser, source);
	lexer->line = 0;
	lexer->cache_size = 0;
}


// Parses the next token, populating the token pointer.
void next(Lexer *lexer, Token *token) {

}


// Consumes a token, moving to the next one.
Token * consume(Lexer *lexer) {
	if (lexer->cache_size == 0) {
		next(lexer, &lexer->cache[0]);
		lexer->cache_size++;
	}

	lexer->cache_size--;
	return &lexer->cache[lexer->cache_size];
}


// Peeks at a token further ahead in the source code, without
// advancing the cursor.
Token * peek(Lexer *lexer, int amount) {
	// Populate the cache with enough tokens to include
	// the one we're wanting to peek at.
	if (lexer->cache_size <= amount) {
		for (int i = 0; i <= amount - lexer->cache_size; i++) {
			next(lexer, &lexer->cache[lexer->cache_size]);
			lexer->cache_size++;
		}
	}

	// Return the token we're after.
	return &lexer->cache[amount];
}


// Returns true if the lexer starts with the given token type.
bool match(Lexer *lexer, TokenType token) {
	Token *current = peek(lexer, 0);
	return current->type == token;
}


// Returns true if the lexer starts with the two given tokens.
bool match2(Lexer *lexer, TokenType one, TokenType two) {
	Token *second = peek(lexer, 1);
	Token *first = peek(lexer, 0);
	return first->type == one && second->type == two;
}
