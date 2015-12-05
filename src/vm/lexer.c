
//
//  Lexer
//

#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "lexer.h"


// Evaluates to true if the given character is a digit.
#define IS_NUMBER(ch) (ch >= '0' && ch <= '9')


// Evaluates to true if the given character is an identifier (letter, number,
// underscore).
#define IS_IDENTIFIER(ch)                                     \
	((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||  \
	 IS_NUMBER(ch) || ch == '_')


// Evaluates to true if the given character is a newline.
#define IS_NEWLINE(ch) (ch == '\n' || ch == '\r')


// Evaluates to true if the given character is whitespace.
#define IS_WHITESPACE(ch)  \
	(ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ')


// Moves the cursor 1 place forward.
#define CONSUME() (lexer->cursor++)


// Evaluates to the character at the current cursor position.
#define CURRENT() (lexer->source[lexer->cursor])


// Evaluates to a pointer to the current character.
#define CURRENT_PTR() (&CURRENT())


// Evaluates to the character a certain amount in front of the current cursor
// position.
#define PEEK(amount) (lexer->source[lexer->cursor + amount])


// Evaluates to true if the cursor is past the end of the file.
#define IS_EOF() (CURRENT() == '\0')


// Consumes a character and returns the given token.
#define SINGLE(single)     \
	CONSUME();             \
	lexer->token = single; \
	break;


// Consumes a character, and if the next character matches the given character,
// returns the second option, else returns the first.
#define DOUBLE(first, condition, second)                                   \
	CONSUME();                                                             \
	lexer->token = CURRENT() == (condition) ? (CONSUME(), second) : first; \
	break;


// Convenience method for defining a keyword.
#define KEYWORD(name, token)                 \
	else if (lexer_matches(lexer, (name))) { \
		lexer->cursor += strlen(name);       \
		return (token);                      \
	}


// Returns true if the lexer matches the given string.
bool lexer_matches(Lexer *lexer, char *string) {
	int i;
	for (i = 0; string[i] != '\0'; i++) {
		if (PEEK(i) == '\0' || PEEK(i) != string[i]) {
			return false;
		}
	}

	// Ensure the character after the match is not an identifier
	return !IS_IDENTIFIER(PEEK(i));
}


// Consumes characters up until the next non-whitespace characters.
void consume_whitespace(Lexer *lexer) {
	while (IS_WHITESPACE(CURRENT())) {
		CONSUME();
	}
}


// Creates a new lexer.
Lexer lexer_new(char *source) {
	Lexer lexer;
	lexer.source = source;
	lexer.cursor = 0;
	return lexer;
}


// Lexes the base prefix of a number.
int lexer_number_base(Lexer *lexer) {
	// Base prefixes begin with a 0
	if (CURRENT() == '0') {
		// Skip over the 0
		CONSUME();

		switch (CURRENT()) {
		// Binary
		case 'b': return 2;
		// Octal
		case 'o': return 8;
		// Hexadecimal
		case 'x': return 16;
		// Invalid
		default: return -1;
		}
	}
	return 10;
}


// Lexes an integer.
Token lexer_integer(Lexer *lexer, int base) {
	// Consume the number
	char *end;
	char *start = CURRENT_PTR();
	uint64_t integer = strtol(start, &end, base);
	size_t length = end - start;

	// Must have a length greater than 0
	if (length == 0) {
		return TOKEN_UNRECOGNISED;
	}

	// Update the cursor position
	lexer->cursor += length;

	// Next character must not be an identifier
	if (IS_IDENTIFIER(CURRENT())) {
		return TOKEN_UNRECOGNISED;
	}

	// Convert to a double if the number is too large for an integer
	if (integer > SHRT_MAX) {
		lexer->value.number = (double) integer;
		return TOKEN_NUMBER;
	} else {
		lexer->value.integer = (int16_t) integer;
		return TOKEN_INTEGER;
	}
}


// Lexes a floating point number.
Token lexer_decimal(Lexer *lexer) {
	// Consume the number
	char *end;
	char *start = CURRENT_PTR();
	lexer->value.number = strtod(start, &end);
	size_t length = end - start;

	// Length must be greater than 0
	if (length == 0) {
		return TOKEN_UNRECOGNISED;
	}

	// Update the cursor position
	lexer->cursor += length;

	// Next character must not be an identifier
	if (IS_IDENTIFIER(CURRENT())) {
		return TOKEN_UNRECOGNISED;
	}

	return TOKEN_NUMBER;
}


// Returns true if there's a decimal place in the number.
bool lexer_number_is_decimal(Lexer *lexer) {
	// Consume up until the first non-number character
	int amount = 0;
	while (IS_NUMBER(PEEK(amount))) {
		amount++;
	}

	// The number is a decimal if we have a dot followed by another number
	return PEEK(amount) == '.' && IS_NUMBER(PEEK(amount + 1));
}


// Lexes a number.
Token lexer_number(Lexer *lexer) {
	// Check for a base prefix
	int base = lexer_number_base(lexer);
	if (base == -1) {
		// Invalid base specifier
		return TOKEN_UNRECOGNISED;
	}

	if (base != 10 || !lexer_number_is_decimal(lexer)) {
		// Consume an integer if we're not parsing a number that needs to be
		// stored as a double
		return lexer_integer(lexer, base);
	} else {
		return lexer_decimal(lexer);
	}
}


// Lexes a string literal.
Token lexer_string(Lexer *lexer) {
	Identifier *identifier = &lexer->value.identifier;

	// Save the opening quote and starting location
	char quote = CURRENT();
	CONSUME();
	identifier->start = CURRENT_PTR();
	identifier->length = 0;

	// Consume characters until we reach the end of the string
	while (!IS_EOF() && (CURRENT() != quote || PEEK(-1) == '\\')) {
		identifier->length++;
		CONSUME();
	}

	// Check for an unterminated string
	if (IS_EOF()) {
		// TODO: Handle unterminated strings better
		return TOKEN_UNRECOGNISED;
	}

	// Consume the final quote
	CONSUME();
	return TOKEN_STRING;
}


// Lexes an identifier.
Token lexer_identifier(Lexer *lexer) {
	Identifier *identifier = &lexer->value.identifier;

	// Consume an identifier
	identifier->start = CURRENT_PTR();
	identifier->length = 0;
	while (IS_IDENTIFIER(CURRENT())) {
		CONSUME();
		identifier->length++;
	}

	// We have an identifier if the length is greater than 0
	return identifier->length > 0 ? TOKEN_IDENTIFIER : TOKEN_UNRECOGNISED;
}


// Lexes an identifier or language keyword.
Token lexer_keyword_identifier(Lexer *lexer) {
	// Check for a keyword
	if (lexer_matches(lexer, "else")) {
		lexer->cursor += 4;

		// Check for an `if`
		int saved = lexer->cursor;
		consume_whitespace(lexer);
		if (lexer_matches(lexer, "if")) {
			lexer->cursor += 2;
			return TOKEN_ELSE_IF;
		}

		lexer->cursor = saved;
		return TOKEN_ELSE;
	}

	KEYWORD("if", TOKEN_IF)
	KEYWORD("while", TOKEN_WHILE)
	KEYWORD("loop", TOKEN_LOOP)
	KEYWORD("for", TOKEN_FOR)
	KEYWORD("break", TOKEN_BREAK)
	KEYWORD("let", TOKEN_LET)
	KEYWORD("fn", TOKEN_FN)
	KEYWORD("import", TOKEN_IMPORT)
	KEYWORD("true", TOKEN_TRUE)
	KEYWORD("false", TOKEN_FALSE)
	KEYWORD("nil", TOKEN_NIL)

	// If we didn't match a keyword, try and parse an identifier
	return lexer_identifier(lexer);
}


// Parses the next token.
void lexer_next(Lexer *lexer) {
	switch (CURRENT()) {
	// End of file
	case '\0':
		lexer->token = TOKEN_EOF;
		break;

	// Whitespace
	case ' ':
	case '\t':
	case '\n':
	case '\r':
		consume_whitespace(lexer);
		lexer_next(lexer);
		break;

	// Operators
	case '+': DOUBLE(TOKEN_ADD, '=', TOKEN_ADD_ASSIGN);
	case '-': DOUBLE(TOKEN_SUB, '=', TOKEN_SUB_ASSIGN);
	case '*': DOUBLE(TOKEN_MUL, '=', TOKEN_MUL_ASSIGN);
	case '/': DOUBLE(TOKEN_DIV, '=', TOKEN_DIV_ASSIGN);
	case '%': DOUBLE(TOKEN_MOD, '=', TOKEN_MOD_ASSIGN);
	case '=': DOUBLE(TOKEN_ASSIGN, '=', TOKEN_EQ);
	case '!': DOUBLE(TOKEN_NOT, '=', TOKEN_NEQ);
	case '<': DOUBLE(TOKEN_LT, '=', TOKEN_LE);
	case '>': DOUBLE(TOKEN_GT, '=', TOKEN_GE);
	case '&': DOUBLE(TOKEN_BIT_AND, '&', TOKEN_AND);
	case '|': DOUBLE(TOKEN_BIT_OR, '|', TOKEN_OR);
	case '.': DOUBLE(TOKEN_DOT, '.', TOKEN_CONCAT);
	case '^': SINGLE(TOKEN_BIT_XOR);
	case '~': SINGLE(TOKEN_BIT_NOT);
	case '(': SINGLE(TOKEN_OPEN_PARENTHESIS);
	case ')': SINGLE(TOKEN_CLOSE_PARENTHESIS);
	case '[': SINGLE(TOKEN_OPEN_BRACKET);
	case ']': SINGLE(TOKEN_CLOSE_BRACKET);
	case '{': SINGLE(TOKEN_OPEN_BRACE);
	case '}': SINGLE(TOKEN_CLOSE_BRACE);
	case ',': SINGLE(TOKEN_COMMA);

	// Numbers
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		lexer->token = lexer_number(lexer);
		break;

	// Strings
	case '"':
	case '\'':
		lexer->token = lexer_string(lexer);
		break;

	// Keywords and identifiers
	default:
		lexer->token = lexer_keyword_identifier(lexer);
		break;
	}
}


// Converts the character following a `\` into its corresponding escape
// sequence.
char escape_sequence(char ch) {
	switch (ch) {
	case 'n':  return '\n';
	case 'r':  return '\r';
	case 't':  return '\t';
	case '\'': return '\'';
	case '"':  return '"';
	case '\\': return '\\';

	// Invalid escape sequence
	default: return '\0';
	}
}


// Extracts a string from the given identifier. Returns a heap allocated string
// that needs to be freed.
//
// Returns NULL if the string contains an invalid escape sequence.
char * lexer_extract_string(Identifier identifier) {
	// Since the string with parsed escape sequences can't be longer than the
	// string in the source code, allocate the same amount of room for each.
	// Add 1 for the NULL terminator
	size_t length = 0;
	char *result = malloc((identifier.length + 1) * sizeof(char));

	for (size_t i = 0; i < identifier.length; i++) {
		// Check for an escape sequence
		if (identifier.start[i] == '\\') {
			result[length++] = escape_sequence(identifier.start[++i]);

			// Invalid escape sequence
			if (result[length - 1] == '\0') {
				return NULL;
			}
		} else {
			// Normal character
			result[length++] = identifier.start[i];
		}
	}

	// Add the NULL terminator
	result[length++] = '\0';
	return result;
}


// Returns the current source code line of the lexer.
uint32_t lexer_line(Lexer *lexer) {
	uint32_t line = 0;

	// Iterate over every character in the source code
	for (char *current = CURRENT_PTR(); current >= lexer->source; current--) {
		// Skip the current character if it isn't a newline
		if (*current != '\n' && *current != '\r') {
			continue;
		}

		// Increment the line number
		line++;

		// Treat \n\r or \r\n as a single newline
		char prev = *(current - 1);
		if ((prev == '\n' || prev == '\r') && prev != *current) {
			current--;
		}
	}

	return line;
}
