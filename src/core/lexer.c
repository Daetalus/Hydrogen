
//
//  Lexer
//

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lexer.h"
#include "state.h"
#include "pkg.h"
#include "err.h"


// Create a new lexer on an interpreter state in the package `pkg`.
Lexer lexer_new(HyState *state, Index src_index) {
	Source *src = &vec_at(state->sources, src_index);

	Lexer lexer;
	lexer.state = state;
	lexer.cursor = src->contents;
	lexer.line = 1;
	lexer.token.source = src_index;

	// Lex the first token
	lexer_next(&lexer);
	return lexer;
}


// Return the character under the cursor.
static inline char current(Lexer *lexer) {
	return *lexer->cursor;
}


// Return a character a certain number of characters in front of the cursor
// Does not protect against buffer overflow, so we must be certain before
// calling this function that cursor + amount does not extend past the end of
// the string.
static inline char peek(Lexer *lexer, int32_t amount) {
	return *(lexer->cursor + amount);
}


// Return true if the lexer is at the end of the file.
static inline bool eof(Lexer *lexer) {
	return current(lexer) == '\0';
}


// Move the cursor 1 character forward.
static void consume(Lexer *lexer) {
	// Don't do anything if we're at the end of the file
	if (eof(lexer)) {
		return;
	}

	// Treat \r\n as a single newline
	if (current(lexer) == '\r' && peek(lexer, 1) == '\n') {
		lexer->cursor++;
	}

	// Check for newlines
	if (current(lexer) == '\n' || current(lexer) == '\r') {
		lexer->line++;
	}

	lexer->cursor++;
}


// Move the cursor forward by an amount. Doesn't check for buffer overflow so
// the caller must be sure cursor + amount doesn't extend past the end of the
// source. Also doesn't check for newlines so the line number won't be updated
// if the characters we're skipping contain newlines.
static inline void forward(Lexer *lexer, int32_t amount) {
	lexer->cursor += amount;
}


// Return true if the string starting at the lexer's current cursor position
// matchines `string`.
static inline bool matches(Lexer *lexer, char *string) {
	return strncmp(lexer->cursor, string, strlen(string)) == 0;
}


// Return true if the string starting at the lexer's current cursor position
// matches `string`, and the character after this string separates identifiers.
static inline bool matches_identifier(Lexer *lexer, char *string) {
	uint32_t length = strlen(string);
	return strncmp(lexer->cursor, string, length) == 0 &&
		!is_identifier(peek(lexer, length));
}


// Consume characters until the end of the current line (excluding the newline
// character).
static inline void consume_line(Lexer *lexer) {
	while (!eof(lexer) && !is_newline(current(lexer))) {
		consume(lexer);
	}
}


// Consume all whitespace characters under the cursor.
static inline void consume_whitespace(Lexer *lexer) {
	while (is_whitespace(current(lexer))) {
		consume(lexer);
	}
}


// Parse a block comment. Assume the first opening delimeter has been consumed.
static void lex_block_comment(Lexer *lexer) {
	// Save the starting character of the initial block comment
	char *start = lexer->token.start;

	// Keep consuming until we reach a terminator, keeping track of nested
	// comments
	uint32_t nested = 1;
	while (!eof(lexer) && nested > 0) {
		if (matches(lexer, "*/")) {
			nested--;
		} else if (matches(lexer, "/*")) {
			nested++;
		}
		consume(lexer);
	}

	// Check if there were unterminated block comments
	if (nested > 0) {
		// Create a fake token for the start of the block comment
		Token token = lexer->token;
		token.type = TOKEN_COMMENT;
		token.start = start - 2;
		token.length = 2;

		Error err = err_new(lexer->state);
		err_print(&err, "Unterminated block comment");
		err_token(&err, &token);
		err_trigger(&err);
	}

	// We've already consumed the first character of the final terminator, so
	// only consume the second
	consume(lexer);
}


// Return true if we could lex a comment (block or single line).
static bool lex_comment(Lexer *lexer) {
	// Consume the first `/`
	consume(lexer);

	if (current(lexer) == '/') {
		// Single line comment
		consume_line(lexer);
	} else if (current(lexer) == '*') {
		// Block comment
		consume(lexer);
		lex_block_comment(lexer);
	} else {
		// Not a comment
		return false;
	}

	return true;
}


// Lex a string. Assume the character under the cursor is an opening quote.
static void lex_string(Lexer *lexer) {
	Token *token = &lexer->token;
	token->type = TOKEN_STRING;

	// Start the token's length at 2 to include the opening and closing quotes
	token->length = 2;

	// Save the opening quote and skip over it
	char quote = current(lexer);
	consume(lexer);

	// Consume characters until we reach the end of the string
	while (!eof(lexer) &&
			!(current(lexer) == quote && peek(lexer, -1) != '\\')) {
		token->length++;
		consume(lexer);
	}

	// Check the string has a terminating quote
	if (eof(lexer)) {
		Error err = err_new(lexer->state);
		err_print(&err, "Unterminated string literal");
		err_token(&err, token);
		err_trigger(&err);
		return;
	}

	// Consume the closing quote
	consume(lexer);
}


// Lex a number prefix (doesn't consume it). Return the number base.
static int lex_number_prefix(Lexer *lexer) {
	// Must start with 0
	if (current(lexer) != '0' || !is_base_prefix(peek(lexer, 1))) {
		return 10;
	}

	// Depending on the following character
	switch (peek(lexer, 1)) {
	case 'b':
		// Binary
		return 2;
	case 'o':
		// Octal
		return 8;
	case 'x':
		// Hexadecimal
		return 16;
	default:
		// Unrecognised
		return -1;
	}
}


// Return true if a hexadecimal number is floating point.
static bool hex_is_float(Lexer *lexer) {
	// Skip the first sequence of hexadecimal digits
	// Start the position at 2 to skip the base prefix
	uint32_t pos = 2;
	while (is_hex(peek(lexer, pos))) {
		pos++;
	}

	// If the following character is a `p` (for power of 2 exponent) or a
	// `.` followed by a decimal digit, then we're dealing with a float
	char ch = peek(lexer, pos);
	return ch == 'p' || ch == 'P' ||
		(ch == '.' && is_decimal(peek(lexer, pos + 1)));
}


// Return true if a decimal number is floating point.
static bool decimal_is_float(Lexer *lexer) {
	// Skip the first sequence of decimal digits
	uint32_t pos = 0;
	while (is_decimal(peek(lexer, pos))) {
		pos++;
	}

	// If the following character is a `.` followed by a digit, or we have
	// a decimal exponent, then the number is a float
	// Unfortunately, C doesn't allow you to parse integers with exponents, so
	// we have to treat integers with decimal exponents as floats
	char ch = peek(lexer, pos);
	return ch == 'e' || ch == 'E' ||
		(ch == '.' && is_decimal(peek(lexer, pos + 1)));
}


// Return true if the number under the lexer's cursor is floating point.
static bool number_is_float(Lexer *lexer, int base) {
	if (base == 16) {
		return hex_is_float(lexer);
	} else if (base == 10) {
		return decimal_is_float(lexer);
	} else {
		return false;
	}
}


// Ensure the current character is not an identifier.
static void ensure_not_identifier(Lexer *lexer) {
	Token *token = &lexer->token;

	// Trigger an error if the current character is part of an identifier
	if (is_identifier(current(lexer))) {
		Error err = err_new(lexer->state);
		err_print(&err, "Unexpected identifier after number ");
		err_print_token(&err, token);
		err_token(&err, token);
		err_trigger(&err);
	}
}


// Lex a floating point number.
static void lex_floating_point(Lexer *lexer) {
	Token *token = &lexer->token;

	// Parse a floating point number
	// TODO: 0x0xef01 is parsed as a number because strtold accepts an optional
	// hex prefix. Fix it
	char *end;
	token->type = TOKEN_NUMBER;
	token->number = strtold(lexer->cursor, &end);
	token->length = end - lexer->cursor;
	lexer->cursor = end;

	// Next character cannot be an identifier
	ensure_not_identifier(lexer);
}


// Lex an integer.
static void lex_integer(Lexer *lexer, int base) {
	Token *token = &lexer->token;

	// Parse integer
	// TODO: 0x0xef01 is parsed as a number because strtoull accepts an optional
	// hex prefix. Fix it
	char *end;
	uint64_t value = strtoull(lexer->cursor, &end, base);
	token->length = end - lexer->cursor;
	lexer->cursor = end;

	// Next character cannot be an identifier
	// Returns via longjmp if the error is triggered
	ensure_not_identifier(lexer);

	// Convert to a double if the number is too large to fit into a signed 16
	// bit integer
	if (value > SHRT_MAX) {
		token->type = TOKEN_NUMBER;
		token->number = (double) value;
	} else {
		token->type = TOKEN_INTEGER;
		token->integer = (int16_t) value;
	}
}


// Print an invalid base prefix error.
static void invalid_base_prefix(Lexer *lexer) {
	// Create a fake identifier for the base prefix
	Token *token = &lexer->token;
	token->type = TOKEN_IDENTIFIER;
	token->length = 2;

	// Trigger the error
	Error err = err_new(lexer->state);
	err_print(&err, "Invalid base prefix ");
	err_print_token(&err, token);
	err_token(&err, token);
	err_trigger(&err);
}


// Lex a number. Return true if successful.
static bool lex_number(Lexer *lexer) {
	// Ensure we start with a decimal digit
	if (!is_decimal(current(lexer))) {
		return false;
	}

	int base = lex_number_prefix(lexer);
	if (base == -1) {
		invalid_base_prefix(lexer);
		return false;
	}

	if (number_is_float(lexer, base)) {
		lex_floating_point(lexer);
	} else {
		// Skip the base prefix
		if (base != 10) {
			forward(lexer, 2);
		}

		lex_integer(lexer, base);
	}

	return true;
}


// Convenience method for associating a token with a keyword.
#define KEYWORD(name, keyword)                    \
	else if (matches_identifier(lexer, (name))) { \
		token->type = (keyword);                  \
		token->length = strlen((name));           \
		forward(lexer, token->length);            \
		return true;                              \
	}


// Lex a keyword. Return true if successful.
static bool lex_keyword(Lexer *lexer) {
	Token *token = &lexer->token;

	// Since an `else if` token can have an unknown amount of whitespace between
	// the `else` and `if`, we need to handle it separately
	if (matches_identifier(lexer, "else")) {
		// Skip the `else`
		char *start = lexer->cursor;
		forward(lexer, 4);

		// Check for a following `if`
		consume_whitespace(lexer);
		if (matches_identifier(lexer, "if")) {
			// Found a following `if`, so this is an else if token
			forward(lexer, 2);
			token->type = TOKEN_ELSE_IF;
			token->length = lexer->cursor - start;
		} else {
			// No `if`, so just an else token
			token->type = TOKEN_ELSE;
			token->length = 4;
		}
		return true;
	}

	KEYWORD("if", TOKEN_IF)
	KEYWORD("while", TOKEN_WHILE)
	KEYWORD("loop", TOKEN_LOOP)
	KEYWORD("for", TOKEN_FOR)
	KEYWORD("break", TOKEN_BREAK)
	KEYWORD("let", TOKEN_LET)
	KEYWORD("fn", TOKEN_FN)
	KEYWORD("return", TOKEN_RETURN)
	KEYWORD("import", TOKEN_IMPORT)
	KEYWORD("true", TOKEN_TRUE)
	KEYWORD("false", TOKEN_FALSE)
	KEYWORD("nil", TOKEN_NIL)
	KEYWORD("struct", TOKEN_STRUCT)
	KEYWORD("new", TOKEN_NEW)
	KEYWORD("self", TOKEN_SELF)

	return false;
}


// Lex an identifier. Return true if successful.
static bool lex_identifier(Lexer *lexer) {
	// Ensure we start with an identifier character
	if (!is_identifier_start(current(lexer))) {
		return false;
	}

	// Lex an identifier
	Token *token = &lexer->token;
	token->type = TOKEN_IDENTIFIER;
	token->length = 0;
	while (is_identifier(current(lexer))) {
		token->length++;
		consume(lexer);
	}

	return true;
}


// Set the token's type.
static void set(Lexer *lexer, TokenType type) {
	Token *token = &lexer->token;
	consume(lexer);
	token->type = type;
	token->length = 1;
}


// If the character after the cursor matches `ch`, then set the token's type to
// `type2`, otherwise set the type to `type1`.
static void set_2(Lexer *lexer, TokenType type, char ch2,
		TokenType type2) {
	Token *token = &lexer->token;
	consume(lexer);
	if (current(lexer) == ch2) {
		consume(lexer);
		token->type = type2;
		token->length = 2;
	} else {
		token->type = type;
		token->length = 1;
	}
}


// If the character after the cursor matches `ch2`, then set the token's type
// to `type2`, else if it matches `ch3`, set the token type to `type3`, else
// set the type to `type`.
static void set_3(Lexer *lexer, TokenType type, char ch2, TokenType type2,
		char ch3, TokenType type3) {
	Token *token = &lexer->token;
	consume(lexer);
	if (current(lexer) == ch2) {
		consume(lexer);
		token->type = type2;
		token->length = 2;
	} else if (current(lexer) == ch3) {
		consume(lexer);
		token->type = type3;
		token->length = 2;
	} else {
		token->type = type;
		token->length = 1;
	}
}


// Lex the next token in the source code.
void lexer_next(Lexer *lexer) {
	Token *token = &lexer->token;
	token->start = lexer->cursor;

	switch (current(lexer)) {
		// End of file
	case '\0':
		token->type = TOKEN_EOF;
		token->length = 0;
		break;

		// Whitespace
	case ' ':
	case '\t':
	case '\n':
	case '\r':
		consume_whitespace(lexer);
		lexer_next(lexer);
		break;

		// Syntax
	case '^': set(lexer, TOKEN_BIT_XOR); break;
	case '~': set(lexer, TOKEN_BIT_NOT); break;
	case '(': set(lexer, TOKEN_OPEN_PARENTHESIS); break;
	case ')': set(lexer, TOKEN_CLOSE_PARENTHESIS); break;
	case '[': set(lexer, TOKEN_OPEN_BRACKET); break;
	case ']': set(lexer, TOKEN_CLOSE_BRACKET); break;
	case '{': set(lexer, TOKEN_OPEN_BRACE); break;
	case '}': set(lexer, TOKEN_CLOSE_BRACE); break;
	case ',': set(lexer, TOKEN_COMMA); break;
	case '+': set_2(lexer, TOKEN_ADD, '=', TOKEN_ADD_ASSIGN); break;
	case '-': set_2(lexer, TOKEN_SUB, '=', TOKEN_SUB_ASSIGN); break;
	case '*': set_2(lexer, TOKEN_MUL, '=', TOKEN_MUL_ASSIGN); break;
	case '%': set_2(lexer, TOKEN_MOD, '=', TOKEN_MOD_ASSIGN); break;
	case '=': set_2(lexer, TOKEN_ASSIGN, '=', TOKEN_EQ); break;
	case '!': set_2(lexer, TOKEN_NOT, '=', TOKEN_NEQ); break;
	case '&': set_2(lexer, TOKEN_BIT_AND, '&', TOKEN_AND); break;
	case '|': set_2(lexer, TOKEN_BIT_OR, '|', TOKEN_OR); break;
	case '.': set_2(lexer, TOKEN_DOT, '.', TOKEN_CONCAT); break;
	case '<': set_3(lexer, TOKEN_LT, '=', TOKEN_LE, '<', TOKEN_LSHIFT); break;
	case '>': set_3(lexer, TOKEN_GT, '=', TOKEN_GE, '>', TOKEN_RSHIFT); break;

		// Comment or division
	case '/':
		if (lex_comment(lexer)) {
			lexer_next(lexer);
		} else if (current(lexer) == '=') {
			consume(lexer);
			token->type = TOKEN_DIV_ASSIGN;
			token->length = 2;
		} else {
			token->type = TOKEN_DIV;
			token->length = 1;
		}
		break;

		// String
	case '\'':
	case '"':
		lex_string(lexer);
		break;

	default:
		// Number
		if (lex_number(lexer)) {
			break;
		}

		// Keyword
		if (lex_keyword(lexer)) {
			break;
		}

		// Identifier
		if (lex_identifier(lexer)) {
			break;
		}

		// Unrecognised
		token->type = TOKEN_UNRECOGNISED;
		token->length = 0;
		break;
	}
}



//
//  String Extraction
//

// Convert a hexadecimal character into a number.
static uint8_t hex_to_number(char ch) {
	if (ch >= 'a' && ch <= 'f') {
		return 10 + ch - 'a';
	} else if (ch >= 'A' && ch <= 'F') {
		return 10 + ch - 'A';
	} else {
		return ch - '0';
	}
}


// Convert 2 hexadecimal digits into a number.
static char hex_sequence_to_number(char *string) {
	return hex_to_number(*string) << 4 | hex_to_number(*(string + 1));
}


// Lex a hexadecimal escape sequence.
static char lex_hex_escape_sequence(char **cursor) {
	// Skip the starting `x`
	(*cursor)++;

	// Expect 2 hexadecimal characters
	if (!is_hex(**cursor) || !is_hex(*(*cursor + 1))) {
		return '\0';
	} else {
		// Convert both characters to numbers
		char result = hex_sequence_to_number(*cursor);
		*cursor += 2;
		return result;
	}
}


// Returns the correct escape sequence for the character following the `\`.
static char lex_escape_sequence(char **cursor) {
	switch (**cursor) {
	case 'a':  return '\a';
	case 'b':  return '\b';
	case 'f':  return '\f';
	case 'n':  return '\n';
	case 'r':  return '\r';
	case 't':  return '\t';
	case 'v':  return '\v';
	case '\\': return '\\';
	case '\'': return '\'';
	case '"':  return '"';
	case '?':  return '?';
	case 'x':  return lex_hex_escape_sequence(cursor);
	default:   return '\0';
	}
}


// Triggers an invalid escape sequence error.
static void invalid_escape_sequence(Lexer *lexer, Token *string, char *start) {
	// Create a token for the escape sequence
	Token token = *string;
	token.type = TOKEN_IDENTIFIER;
	token.start = start;

	// Set the length based off if the sequence is normal or a hex character
	token.length = (*(start + 1) == 'x') ? 4 : 2;

	// Check if we can display the sequence in the error message (ensure we
	// don't have a newline in the sequence)
	bool display_sequence = true;
	for (uint32_t i = 0; i < token.length; i++) {
		char ch = *(start + i);
		if (ch == '\n' || ch == '\r' || ch == '\0') {
			display_sequence = false;
			break;
		}
	}

	// Trigger error
	Error err = err_new(lexer->state);
	err_print(&err, "Invalid escape sequence");
	if (display_sequence) {
		err_print(&err, " `%.*s`", token.length, token.start);
	}
	err_token(&err, &token);
	err_trigger(&err);
}


// String literals need to be extracted from a token separately because escape
// sequences need to be parsed into their proper values. Stores the extracted
// string directly into `buffer`. Ensure that `buffer` is at least as long as
// token->length - 1 (since the token length includes the two surrounding
// quotes). Returns the length of the parsed string.
uint32_t lexer_extract_string(Lexer *lexer, Token *token, char *buffer) {
	uint32_t length = 0;

	// Since the token's starting position and length take into account the two
	// surrounding quotes, start at 1 and finish before the end of the token's
	// length
	char *end = token->start + token->length - 1;
	for (char *cursor = &token->start[1]; cursor < end; cursor++) {
		// Check for an escape sequence
		if (*cursor == '\\') {
			char *start = cursor;
			cursor++;

			// Parse the escape sequence
			char sequence = lex_escape_sequence(&cursor);
			if (sequence == '\0') {
				// Invalid escape sequence
				invalid_escape_sequence(lexer, token, start);
				return 0;
			}

			buffer[length++] = sequence;
		} else {
			buffer[length++] = *cursor;
		}
	}

	// Add the NULL terminator
	buffer[length] = '\0';
	return length;
}
