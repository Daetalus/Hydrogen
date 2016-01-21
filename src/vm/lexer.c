
//
//  Lexer
//

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lexer.h"


// Create a new lexer on an interpreter state in the package `pkg`, lexing the
// source code at `source`.
Lexer lexer_new(HyState *state, Index pkg, Index source) {
	Package *pkg = &vec_at(state->packages, pkg);
	Source *src = &vec_at(pkg->sources, source);

	Lexer lexer;
	lexer.state = state;
	lexer.source = src->contents;
	lexer.cursor = src->contents;
	lexer.token.package = pkg;
	lexer.token.source = source;
	return lexer;
}


// Returns true if a character is a newline.
static inline bool is_newline(char ch) {
	return ch == '\n' || ch == '\r';
}


// Returns true if a character is whitespace.
static inline bool is_whitespace(char ch) {
	return is_newline(ch) || ch == ' ' || ch == '\t';
}


// Returns true if a character is a decimal digit.
static inline bool is_decimal(char ch) {
	return ch >= '0' && ch <= '9';
}


// Returns true if a character is a hexadecimal digit.
static inline bool is_hex(char ch) {
	return is_decimal(ch) || (ch >= 'a' && ch <= 'f');
}


// Returns true if a character can start an identifier.
static inline bool is_identifier_start(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}


// Returns true if a character can be part of an identifier.
static inline bool is_identifier(char ch) {
	return is_identifier_start(ch) || is_decimal(ch);
}


// Returns the character under the cursor.
static inline char current(Lexer *lexer) {
	return *lexer->cursor;
}


// Returns a character a certain number of characters in front of the cursor.
// Does not protect against buffer overflow, so we must be certain before
// calling this function that cursor + amount does not extend past the end of
// the string.
static inline char peek(Lexer *lexer, uint32_t amount) {
	return *(lexer->cursor + amount);
}


// Returns true if the lexer is at the end of the file.
static inline bool eof(Lexer *lexer) {
	return current(lexer) == '\0';
}


// Moves the cursor 1 character forward.
static inline void consume(Lexer *lexer) {
	// Don't do anything if we're at the end of the file
	if (!eof(lexer)) {
		lexer->cursor++;
	}
}


// Moves the cursor forward by an amount. Doesn't check for buffer overflow so
// the caller must be sure cursor + amount doesn't extend past the end of the
// source.
static inline void forward(Lexer *lexer, uint32_t amount) {
	lexer->cursor += amount;
}


// Returns true if the string starting at the lexer's current cursor position
// matchines `string`.
static inline bool matches(Lexer *lexer, char *string) {
	return strncmp(lexer->cursor, string, strlen(string));
}


// Returns true if the string starting at the lexer's current cursor position
// matches `string`, and the character after this string separates identifiers.
static inline bool matches_identifier(Lexer *lexer, char *string) {
	uint32_t length = strlen(string);
	return strncmp(lexer->cursor, string, length) == 0 &&
		!is_identifier(lexer_peek(lexer, length));
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


// Consume characters until the string under the cursor matches `terminator`.
static inline void consume_until(Lexer *lexer, char *terminator) {
	while (!eof(lexer) && !matches(lexer, terminator)) {
		consume(lexer);
	}
}


// Returns true if we could lex a comment (block or single line).
static bool comment(Lexer *lexer) {
	// Consume first `/`
	consume(lexer);

	if (current(lexer) == '/') {
		// Single line
		consume_line(lexer);
	} else if (current(lexer) == '*') {
		// Block
		consume(lexer);
		consume_until(lexer, "*/");
		forward(lexer, 2);
	} else {
		return false;
	}
	return true;
}


// Lexes a string. Assumes the character under the cursor is an opening quote.
static void string(Lexer *lexer) {
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
	}

	// Check the string isn't unterminated
	if (eof(lexer)) {
		// TODO: Unterminated string error
		return;
	}

	// Consume closing quote
	consume(lexer);
}


// Lexes a number prefix (doesn't consume it), returning the base.
static int number_prefix(Lexer *lexer) {
	// Must start with 0
	if (current(lexer) != '0') {
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


// Returns true if the number under the lexer's cursor is floating point.
static bool number_is_float(Lexer *lexer, int base) {
	// If we're starting with a hexadecimal prefix
	if (base == 16) {
		// Skip the first sequence of hexadecimal digits
		// Start the position at 2 to skip the base prefix
		uint32_t position = 2;
		while (is_hex(peek(lexer, position))) {
			position++;
		}

		// If the following character is a `p` (for power of 2 exponent) or a
		// `.` followed by a hex digit, then we're dealing with a float
		char ch = peek(lexer, position);
		return ch == 'p' || ch == 'P' ||
			(ch == '.' && is_hex(peek(lexer, position + 1)));
	} else if (base == 10) {
		// Skip the first sequence of decimal digits
		uint32_t position = 2;
		while (is_decimal(peek(lexer, position))) {
			position++;
		}

		// If the following character is a `.` followed by a digit, then we have
		// a float
		return ch == '.' && is_decimal(peek(lexer, position + 1));
	}

	return false;
}


// Ensures the current character is not an identifier.
static void ensure_not_identifier(Lexer *lexer) {
	if (is_identifier(current(lexer))) {
		// TODO: Number followed by letter error
	}
}


// Lexes a floating point number.
static void floating_point(Lexer *lexer) {
	Token *token = &lexer->token;

	// Parse a floating point number
	char *end;
	token->type = TOKEN_NUMBER;
	token->number = strtold(lexer->cursor, &end);
	token->length = end - lexer->cursor;
	lexer->cursor = end;

	// Next character cannot be an identifier
	ensure_not_identifier(lexer);
}


// Lexes an integer.
static void integer(Lexer *lexer) {
	Token *token = &lexer->token;

	// Parse integer
	char *end;
	uint64_t value = strtoull(lexer->cursor, &end);
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


// Lexes a number, returning true if possible.
static bool number(Lexer *lexer) {
	// Ensure we start with a decimal digit
	if (!is_decimal(current(lexer))) {
		return false;
	}

	int base = number_prefix(lexer);
	if (base == -1) {
		// TODO: Invalid base error
		return false;
	}

	// Lex an integer if we're in octal or binary, or if we fit the conditions
	// of an integer
	if (number_is_float(lexer, base)) {
		floating_point(lexer);
	} else {
		// Skip the base prefix
		forward(2);
		integer(lexer, base);
	}

	return true;
}


// Convenience method for defining a keyword.
#define KEYWORD(name, keyword)          \
	else if (matches(lexer, (name))) {  \
		token->type = (keyword);        \
		token->length = strlen((name)); \
		forward(lexer, token->length);  \
		return true;                    \
	}


// Lexes a keyword, returning true if successful.
static bool keyword(Lexer *lexer) {
	Token *token = &lexer->token;

	// Since an `else if` token can have an unknown amount of whitespace between
	// the `else` and `if`, we need to handle it separately
	if (matches(lexer, "else")) {
		// Skip the `else`
		char *start = lexer->cursor;
		forward(lexer, 4);

		// Check for a following `if`
		consume_whitespace(lexer);
		if (matches(lexer, "if")) {
			forward(lexer, 2);
			token->type = TOKEN_ELSE_IF;
			token->length = lexer->cursor - start;
		} else {
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

	return false;
}


// Lexes an identifier, returning true if successful.
static bool identifier(Lexer *lexer) {
	// Ensure we start with an identifier
	if (!is_identifier_start(current(lexer))) {
		return false;
	}

	// Lex an identifier
	Token *token = &lexer->token;
	token->type = TOKEN_IDENTIFIER;
	token->length = 0;
	while (is_identifier(current(lexer))) {
		token->length++;
	}

	return true;
}


// Sets the token's type.
static void set1(Lexer *lexer, TokenType type) {
	Token *token = &lexer->token;
	consume(lexer);
	token->type = type;
	token->length = 1;
}


// If the character after the cursor matches `ch`, then set the token's type to
// `type2`, otherwise set the type to `type1`.
static void set2(Lexer *lexer, TokenType type, char ch2, TokenType type2) {
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
static void set3(Lexer *lexer, TokenType type, char ch2, TokenType type2,
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
	case '+': set2(lexer, TOKEN_ADD, '=', TOKEN_ADD_ASSIGN); break;
	case '-': set2(lexer, TOKEN_SUB, '=', TOKEN_SUB_ASSIGN); break;
	case '*': set2(lexer, TOKEN_MUL, '=', TOKEN_MUL_ASSIGN); break;
	case '%': set1(lexer, TOKEN_MOD); break;
	case '=': set2(lexer, TOKEN_ASSIGN, '=', TOKEN_EQ); break;
	case '!': set2(lexer, TOKEN_NOT, '=', TOKEN_NEQ); break;
	case '&': set2(lexer, TOKEN_BIT_AND, '&', TOKEN_AND); break;
	case '|': set2(lexer, TOKEN_BIT_OR, '|', TOKEN_OR); break;
	case '.': set2(lexer, TOKEN_DOT, '.', TOKEN_CONCAT); break;
	case '^': set1(lexer, TOKEN_BIT_XOR); break;
	case '~': set1(lexer, TOKEN_BIT_NOT); break;
	case '(': set1(lexer, TOKEN_OPEN_PARENTHESIS); break;
	case ')': set1(lexer, TOKEN_CLOSE_PARENTHESIS); break;
	case '[': set1(lexer, TOKEN_OPEN_BRACKET); break;
	case ']': set1(lexer, TOKEN_CLOSE_BRACKET); break;
	case '{': set1(lexer, TOKEN_OPEN_BRACE); break;
	case '}': set1(lexer, TOKEN_CLOSE_BRACE); break;
	case ',': set1(lexer, TOKEN_COMMA); break;
	case '<': set3(lexer, TOKEN_LT, '=', TOKEN_LE, '<', TOKEN_LEFT_SHIFT); break;
	case '>': set3(lexer, TOKEN_GT, '=', TOKEN_GE, '>', TOKEN_RIGHT_SHIFT); break;

		// Comment or division
	case '/':
		if (comment(lexer)) {
			lexer_next(lexer);
		} else {
			token->type = TOKEN_DIV;
			token->length = 1;
		}
		break;

		// String
	case '\'':
	case '"':
		string(lexer);
		break;

	default:
		// Number
		if (number(lexer)) {
			break;
		}

		// Keyword
		if (keyword(lexer)) {
			break;
		}

		// Identifier
		if (identifier(lexer)) {
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

// Converts a hexadecimal character into a number.
static uint8_t hex_to_number(char ch) {
	if (ch >= 'a' && ch <= 'f') {
		return 10 + ch - 'a';
	} else if (ch >= 'A' && ch <= 'F') {
		return 10 + ch - 'A';
	} else {
		return ch - '0';
	}
}


// Converts 2 hexadecimal digits into a number.
static char hex_sequence_to_number(char *string) {
	return hex_to_number(*string) << 4 | hex_to_number(*(string + 1));
}


// Lexes a hexadecimal escape sequence.
static char hex_escape_sequence(char **cursor) {
	// Skip the starting `x`
	(*cursor)++;

	// Expect 2 hexadecimal characters
	if (!is_hex(**cursor) || !is_hex(*(*cursor + 1))) {
		return '\0';
	} else {
		// Convert both characters to numbers
		char result = hex_sequence_to_number(*cursor);
		(*cursor) += 2;
		return result;
	}
}


// Returns the correct escape sequence for the character following the `\`.
static char escape_sequence(char **cursor) {
	switch (**cursor) {
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'v': return '\v';
	case '\\': return '\\';
	case '\'': return '\'';
	case '"': return '"';
	case '?': return '?';
	case 'x': return hex_escape_sequence(cursor);
	default: return '\0';
	}
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
	char *end = token->string + token->length - 1;
	for (char *cursor = &token->string[1]; cursor < end; cursor++) {
		// Check for an escape sequence
		if (*cursor == '\\') {
			cursor++;

			// Parse the escape sequence
			char sequence = escape_sequence(&cursor);
			if (sequence == '\0') {
				// TODO: Invalid escape sequence error
				return 0;
			}

			buffer[length++] = sequence;
		} else {
			buffer[length++] = *cursor;
		}
	}

	// Add NULL terminator
	buffer[length] = '\0';
	return length;
}
