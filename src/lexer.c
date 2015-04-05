
//
//  Lexer
//


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"


//
//  Classification
//

bool is_whitespace(char c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}


bool is_space_or_tab(char c) {
	return c == ' ' || c == '\t';
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


bool is_separator(char c) {
	return c == '.' || c == '/' || c == '\\' || c == '(' || c == ')' ||
		c == '"' || c == '\'' || c == '-' || c == ':' || c == ',' || c == '.' ||
		c == ';' || c == '<' || c == '>' || c == '~' || c == '!' || c == '@' ||
		c == '#' || c == '$' || c == '%' || c == '^' || c == '&' || c == '*' ||
		c == '|' || c == '+' || c == '=' || c == '[' || c == ']' || c == '{' ||
		c == '}' || c == '`' || c == '~' || c == '?';
}


bool is_quotation_mark(char c) {
	return c == '\'' || c == '"';
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


// Returns a pointer to the current position in the
// source string.
char * current(Parser *parser) {
	return &parser->source[parser->cursor];
}


// Returns the current character.
char current_char(Parser *parser) {
	return parser->source[parser->cursor];
}


// Restores the cursor to a particular location.
void restore_state(Parser *parser, char *location) {
	parser->cursor = location - parser->source;
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
	} else if (parser->cursor + amount > parser->length) {
		// Handle overflow
		parser->cursor = parser->length;
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
		return parser->source[0];
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


// Returns true if the parser starts with the given identifier
// (ie. starts with the given string and is followed by a non-
// identifier character).
bool starts_with_identifier(Parser *parser, char *string, int length) {
	return starts_with(parser, string, length) &&
		is_separator(peek_char(parser, length));
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

	if (is_identifier_start(current_char(parser))) {
		char *start = current(parser);
		while (!is_eof(parser) && is_identifier(current_char(parser))) {
			consume_char(parser);
			(*length)++;
		}
		return start;
	}

	return NULL;
}


// Consumes a number. Returns the length of the consumed number
// if successful, or 0 if unsuccessful. The number is returned
// through the provided argument. Nothing is consumed if the
// function fails.
int consume_number(Parser *parser, double *result) {
	*result = 0.0;
	char *saved = current(parser);

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
	char *start = current(parser);
	char *end;
	if (base != 10) {
		*result = (double) strtol(start, &end, base);
	} else {
		*result = strtod(start, &end);
	}

	if (start == end) {
		// Invalid number.
		restore_state(parser, saved);
		*result = 0.0;
		return 0;
	}

	int length = (end - start);
	parser->cursor = end - parser->source;

	return length;
}


// Consumes a string literal, returning a pointer to
// the start of string allocated on the heap.
// Returns NULL if it failed to find an ending quote.
char * consume_string_literal(Parser *parser, int *length) {
	char quote = current_char(parser);

	if (quote != '\'' && quote != '"') {
		// No opening quote.
		return NULL;
	}

	// Consume the opening quote.
	consume_char(parser);
	char *start = current(parser);

	// Continually consume characters until we encounter another
	// closing quote.
	bool was_escape = false;
	*length = 0;
	while (!is_eof(parser)) {
		// Check for the terminating quote
		if (!was_escape && current_char(parser) == quote) {
			break;
		}

		was_escape = false;
		if (consume_char(parser) == '\\') {
			was_escape = true;
		}

		(*length)++;
	}

	if (is_eof(parser)) {
		// Reached end of file without finding a closing quote.
		*length = 0;
		return NULL;
	}

	// Consume the closing quote
	consume_char(parser);

	return start;
}



//
//  Lexer
//

// Create a new lexer with the given source.
void lexer_new(Lexer *lexer, char *source) {
	parser_new(&lexer->parser, source);
	lexer->line = 0;
	lexer->cache_size = 0;
}



//
//  Parsing
//

// Sets the token pointer with the given values.
void set_token(Token *token, TokenType type, char *location, int length) {
	token->type = type;
	token->location = location;
	token->length = length;
}


// Defines a case which matches `character`, setting the token
// to the token `type` (with length 1) if the character matches.
#define SINGLE_TOKEN(character, type)                 \
	case character:                                   \
		set_token(token, (type), current(parser), 1); \
		consume_char(parser);                         \
		break;


// If the parser matches `ch1` followed by `ch2`, then `type2`
// is set, otherwise if just `ch1` is matched, then `type1`
// is set.
#define DOUBLE_TOKEN(ch1, type1, ch2, type2)               \
	case ch1:                                              \
		if (peek_char(parser, 1) == (ch2)) {               \
			set_token(token, (type2), current(parser), 2); \
			move_cursor(parser, 2);                        \
			break;                                         \
		} else {                                           \
			set_token(token, (type1), current(parser), 1); \
			consume_char(parser);                          \
			break;                                         \
		}

// If matches `ch1` followed by `ch2`, then `type2` is set.
// If matches `ch1` followed by `ch3`, then `type3` is set.
// If just matches `ch1`, then `type1` is set.
#define TRIPLE_TOKEN(ch1, type1, ch2, type2, ch3, type3)   \
	case ch1:                                              \
		if (peek_char(parser, 1) == (ch2)) {               \
			set_token(token, (type2), current(parser), 2); \
			move_cursor(parser, 2);                        \
			break;                                         \
		} else if (peek_char(parser, 1) == (ch3)) {        \
			set_token(token, (type3), current(parser), 2); \
			move_cursor(parser, 2);                        \
			break;                                         \
		} else {                                           \
			set_token(token, (type1), current(parser), 1); \
			consume_char(parser);                          \
			break;                                         \
		}


// If the next word matches `word`, then set the token
// to `type`.
#define KEYWORD(word, length, type)                          \
	if (starts_with_identifier(parser, (word), (length))) {  \
		set_token(token, (type), current(parser), (length)); \
		move_cursor(parser, (length));                       \
		break;                                               \
	}


// Matches two words separated by any amount of whitespace.
#define DOUBLE_KEYWORD(word1, length1, word2, length2, type)      \
	if (starts_with_identifier(parser, (word1), (length1))) {     \
		char *saved = current(parser);                            \
		move_cursor(parser, (length1));                           \
		consume_whitespace(parser);                               \
		if (starts_with_identifier(parser, (word2), (length2))) { \
			int length = current(parser) - saved;                 \
			set_token(token, (type), saved, length);              \
			move_cursor(parser, (length2));                       \
			break;                                                \
		} else {                                                  \
			restore_state(parser, saved);                         \
		}                                                         \
	}


// Parses the next token, populating the token pointer.
void next(Lexer *lexer, Token *token) {
	Parser *parser = &lexer->parser;

	// Check for end of file first.
	if (is_eof(parser)) {
		set_token(token, TOKEN_END_OF_FILE, current(parser), 0);
		return;
	}

	// Match the current character of the parser.
	consume_spaces_tabs(parser);
	char ch = current_char(parser);
	switch(ch) {
		// Mathematical operators
		DOUBLE_TOKEN('+', TOKEN_ADDITION,
			'=', TOKEN_ADDITION_ASSIGNMENT);
		DOUBLE_TOKEN('-', TOKEN_SUBTRACTION,
			'=', TOKEN_SUBTRACTION_ASSIGNMENT);
		DOUBLE_TOKEN('*', TOKEN_MULTIPLICATION,
			'=', TOKEN_MULTIPLICATION_ASSIGNMENT);
		DOUBLE_TOKEN('/', TOKEN_DIVISION,
			'=', TOKEN_DIVISION_ASSIGNMENT);
		DOUBLE_TOKEN('%', TOKEN_MODULO,
			'=', TOKEN_MODULO_ASSIGNMENT);

		// Boolean and bitwise operators
		SINGLE_TOKEN('~', TOKEN_BITWISE_NOT);
		SINGLE_TOKEN('^', TOKEN_BITWISE_XOR);
		DOUBLE_TOKEN('&', TOKEN_BITWISE_AND, '&', TOKEN_BOOLEAN_AND);
		DOUBLE_TOKEN('|', TOKEN_BITWISE_OR, '|', TOKEN_BOOLEAN_OR);
		DOUBLE_TOKEN('!', TOKEN_BOOLEAN_NOT, '=', TOKEN_NOT_EQUAL);
		DOUBLE_TOKEN('=', TOKEN_ASSIGNMENT, '=', TOKEN_EQUAL);
		TRIPLE_TOKEN('<', TOKEN_LESS_THAN,
			'=', TOKEN_LESS_THAN_EQUAL_TO,
			'<', TOKEN_LEFT_SHIFT);
		TRIPLE_TOKEN('>', TOKEN_GREATER_THAN,
			'=', TOKEN_GREATER_THAN_EQUAL_TO,
			'>', TOKEN_RIGHT_SHIFT);

		// Syntax
		SINGLE_TOKEN('(', TOKEN_OPEN_PARENTHESIS);
		SINGLE_TOKEN(')', TOKEN_CLOSE_PARENTHESIS);
		SINGLE_TOKEN('[', TOKEN_OPEN_BRACKET);
		SINGLE_TOKEN(']', TOKEN_CLOSE_BRACKET);
		SINGLE_TOKEN('{', TOKEN_OPEN_BRACE);
		SINGLE_TOKEN('}', TOKEN_CLOSE_BRACE);
		SINGLE_TOKEN('.', TOKEN_DOT);
		SINGLE_TOKEN(',', TOKEN_COMMA);
		SINGLE_TOKEN('\n', TOKEN_LINE);
		SINGLE_TOKEN('\r', TOKEN_LINE);

	default:
		// Keywords
		KEYWORD("let", 3, TOKEN_LET);
		KEYWORD("in", 2, TOKEN_IN);
		KEYWORD("for", 3, TOKEN_FOR);
		KEYWORD("while", 5, TOKEN_WHILE);
		KEYWORD("loop", 4, TOKEN_LOOP);
		KEYWORD("fn", 2, TOKEN_FUNCTION);
		KEYWORD("class", 5, TOKEN_CLASS);
		KEYWORD("if", 2, TOKEN_IF);
		DOUBLE_KEYWORD("else", 4, "if", 2, TOKEN_ELSE_IF);
		KEYWORD("else", 4, TOKEN_ELSE);

		// Number
		if (is_digit(ch)) {
			char *saved = current(parser);
			int length = consume_number(parser, &token->number);
			if (length > 0) {
				set_token(token, TOKEN_NUMBER, saved, length);
				break;
			}
		}

		// String literal
		if (is_quotation_mark(ch)) {
			int length;
			char *start = consume_string_literal(parser, &length);
			if (start != NULL) {
				set_token(token, TOKEN_STRING, start, length);
				break;
			}
		}

		// Identifiers
		if (is_identifier_start(ch)) {
			int length;
			char *start = consume_identifier(parser, &length);
			if (start != NULL) {
				set_token(token, TOKEN_IDENTIFIER, start, length);
				break;
			}
		}

		// Unrecognised token
		set_token(token, TOKEN_UNRECOGNISED, NULL, 0);
	}
}



//
//  Consuming
//

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



//
//  String Literal
//

#define ESCAPE_SEQUENCE(escape_ch, replacement) \
	if (ch == (escape_ch)) {                    \
		string->contents[i] = (replacement);    \
		continue;                               \
	}


// Extracts a string literal pointed to by the given token
// from the source code, populating `string` (assumed to be
// unallocated).
// If the string contains an invalid escape sequence, returns
// a pointer to the start of the escape sequence, else returns
// NULL.
char * extract_string_literal(Token *literal, String *string) {
	// Allocate space for the string, using the literal's
	// length as the capacity, as the string can't get
	// longer than the literal in the source code.
	string_new(string, literal->length);

	for (int i = 0; i < literal->length; i++) {
		char ch = literal->location[i];
		if (ch == '\\') {
			// Handle an escape character
			// Skip the backslash
			i++;
			ch = literal->location[i];

			// Insert the corresponding escape sequence
			ESCAPE_SEQUENCE('n', '\n')
			ESCAPE_SEQUENCE('r', '\r')
			ESCAPE_SEQUENCE('t', '\t')
			ESCAPE_SEQUENCE('\'', '\'')
			ESCAPE_SEQUENCE('"', '"')
			ESCAPE_SEQUENCE('\\', '\\')

			// No escape sequence was found
			return &literal->location[i - 1];
		} else {
			// It's a normal character, so just insert it.
			string->contents[i] = ch;
		}
	}

	return NULL;
}
