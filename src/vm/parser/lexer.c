
//
//  Lexer
//

#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "lexer.h"
#include "../error.h"


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
	((ch) == '\n' || (ch) == '\r' || (ch) == '\t' || (ch) == ' ')

// Moves the cursor 1 place forward.
#define CONSUME() (lexer_consume(lexer))

// Moves the cursor an amount forward, guaranteeing no newlines are skipped
// over.
#define FORWARD(amount)      \
	lexer->cursor += amount; \
	lexer->token.column += amount;

// Evaluates to the character at the current cursor position.
#define CURRENT() (*lexer->cursor)

// Evaluates to the character a certain amount in front of the current cursor
// position.
#define PEEK(amount) (lexer->cursor[(amount)])

// Evaluates to true if the cursor is past the end of the file.
#define IS_EOF() (CURRENT() == '\0')


// Consumes a single character.
void lexer_consume(Lexer *lexer) {
	if (IS_EOF()) {
		return;
	}

	// Check for newline so we can increment the current line number
	if (CURRENT() == '\n' || CURRENT() == '\r') {
		// Treat \r\n as a single newline
		if (CURRENT() == '\r' && PEEK(1) == '\n') {
			lexer->cursor++;
		}

		lexer->token.line_start = lexer->cursor;
		lexer->token.line++;
		lexer->token.column = 1;
	} else {
		lexer->token.column++;
	}

	lexer->cursor++;
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


// Returns true if the lexer starts with the given string.
bool lexer_starts(Lexer *lexer, char *string) {
	for (int i = 0; string[i] != '\0'; i++) {
		if (PEEK(i) == '\0' || PEEK(i) != string[i]) {
			return false;
		}
	}
	return true;
}


// Consumes characters up until the next non-whitespace characters.
void lexer_consume_whitespace(Lexer *lexer) {
	while (IS_WHITESPACE(CURRENT())) {
		CONSUME();
	}
}


// Consumes characters up until the end of the line.
void lexer_consume_eol(Lexer *lexer) {
	while (!IS_EOF() && CURRENT() != '\r' && CURRENT() != '\n') {
		CONSUME();
	}
}


// Consumes characters until the start of the lexer matches the given string.
void lexer_consume_until(Lexer *lexer, char *terminator) {
	while (!IS_EOF() && !lexer_starts(lexer, terminator)) {
		CONSUME();
	}
}


// Creates a new lexer. The file name is used for error messages.
Lexer lexer_new(VirtualMachine *vm, char *file, char *package, char *source) {
	Lexer lexer;
	lexer.vm = vm;
	lexer.source = source;
	lexer.cursor = source;
	lexer.token.line = 1;
	lexer.token.column = 1;
	lexer.token.line_start = source;
	lexer.token.file = file;
	lexer.token.package = package;
	return lexer;
}


// Lexes the base prefix of a number.
int lexer_number_base(Lexer *lexer) {
	// Base prefixes begin with a 0
	if (CURRENT() != '0') {
		return 10;
	}

	switch (PEEK(1)) {
	case 'b':
		// Binary
		FORWARD(2);
		return 2;
	case 'o':
		// Octal
		FORWARD(2);
		return 8;
	case 'x':
		// Hexadecimal
		FORWARD(2);
		return 16;
	default:
		// Not a base prefix
		return 10;
	}
}


// Lexes an integer.
void lexer_integer(Lexer *lexer, int base) {
	Token *token = &lexer->token;

	// Consume the number
	char *end;
	uint64_t integer = strtol(lexer->cursor, &end, base);
	token->length = end - token->start;

	// Must have a length greater than 0
	if (token->length == 0) {
		token->type = TOKEN_UNRECOGNISED;
		return;
	}

	// Update the cursor position
	FORWARD(token->length);

	// Next character must not be an identifier
	if (IS_IDENTIFIER(CURRENT())) {
		token->type = TOKEN_UNRECOGNISED;
		token->length = 0;
		return;
	}

	// Convert to a double if the number is too large for an integer
	if (integer > SHRT_MAX) {
		token->type = TOKEN_NUMBER;
		token->number = (double) integer;
	} else {
		token->type = TOKEN_INTEGER;
		token->integer = (int16_t) integer;
	}
}


// Lexes a floating point number.
void lexer_decimal(Lexer *lexer) {
	Token *token = &lexer->token;

	// Consume the number
	char *end;
	token->type = TOKEN_NUMBER;
	token->number = strtod(token->start, &end);
	token->length = end - token->start;

	// Length must be greater than 0
	if (token->length == 0) {
		token->type = TOKEN_UNRECOGNISED;
		return;
	}

	// Update the cursor position
	FORWARD(token->length);

	// Next character must not be an identifier
	if (IS_IDENTIFIER(CURRENT())) {
		token->type = TOKEN_UNRECOGNISED;
		token->length = 0;
		return;
	}
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
void lexer_number(Lexer *lexer) {
	// Check for a base prefix
	int base = lexer_number_base(lexer);

	if (base != 10 || !lexer_number_is_decimal(lexer)) {
		// Consume an integer if we're not parsing a number that needs to be
		// stored as a double
		lexer_integer(lexer, base);
	} else {
		lexer_decimal(lexer);
	}
}


// Lexes a string literal.
void lexer_string(Lexer *lexer) {
	Token *token = &lexer->token;
	token->length = 1;
	token->type = TOKEN_STRING;

	// Save the opening quote
	char quote = CURRENT();
	CONSUME();

	// Consume characters until we reach the end of the string
	while (!IS_EOF() && (CURRENT() != quote || PEEK(-1) == '\\')) {
		token->length++;
		CONSUME();
	}

	// Check for an unterminated string
	if (IS_EOF()) {
		err_token(lexer->vm, token, "Unterminated string literal");
		return;
	}

	// Consume the final quote
	CONSUME();
	token->length++;
}


// Lexes an identifier.
void lexer_identifier(Lexer *lexer) {
	Token *token = &lexer->token;

	// Consume an identifier
	token->type = TOKEN_IDENTIFIER;
	token->length = 0;
	while (IS_IDENTIFIER(CURRENT())) {
		token->length++;
		CONSUME();
	}

	// We have an identifier if the token's length is greater than 0
	if (token->length == 0) {
		token->type = TOKEN_UNRECOGNISED;
		return;
	}
}


// Convenience method for defining a keyword.
#define KEYWORD(name, keyword)               \
	else if (lexer_matches(lexer, (name))) { \
		int length = strlen(name);           \
		FORWARD(length);                     \
		token->type = (keyword);             \
		token->length = length;              \
	}


// Lexes an identifier or language keyword.
void lexer_keyword_identifier(Lexer *lexer) {
	Token *token = &lexer->token;

	// Check for a keyword
	if (lexer_matches(lexer, "else")) {
		// Skip the `else`
		char *start = lexer->cursor;
		FORWARD(4);

		// Check for an `if`
		lexer_consume_whitespace(lexer);
		if (lexer_matches(lexer, "if")) {
			// Found an else if token
			FORWARD(2);
			token->type = TOKEN_ELSE_IF;
			token->length = lexer->cursor - start;
		} else {
			// Plain else token
			token->type = TOKEN_ELSE;
			token->length = 4;
		}
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

	else {
		// If we didn't match a keyword, try and parse an identifier
		lexer_identifier(lexer);
	}
}


// Defines the mappings:
//   ch1: token1
#define SINGLE(ch1, token1)     \
	case ch1:                   \
		CONSUME();              \
		token->type = (token1); \
		token->length = 1;      \
		break;


// Defines the mappings:
//   ch1 ch2: token2
//   ch1: token1
#define DOUBLE(ch1, token1, ch2, token2) \
	case ch1:                            \
		CONSUME();                       \
		if (CURRENT() == (ch2)) {        \
			CONSUME();                   \
			token->type = (token2);      \
			token->length = 2;           \
		} else {                         \
			token->type = (token1);      \
			token->length = 1;           \
		}                                \
		break;


// Defines the mappings:
//   ch1 ch2: token2
//   ch1 ch3: token3
//   ch1: token1
#define TRIPLE(ch1, token1, ch2, token2, ch3, token3) \
	case ch1:                                         \
		CONSUME();                                    \
		if (CURRENT() == (ch2)) {                     \
			CONSUME();                                \
			token->type = (token2);                   \
			token->length = 2;                        \
		} else if (CURRENT() == (ch3)) {              \
			CONSUME();                                \
			token->type = (token3);                   \
			token->length = 2;                        \
		} else {                                      \
			token->type = (token1);                   \
			token->length = 1;                        \
		}                                             \
		break;


// Parses the next token.
void lexer_next(Lexer *lexer) {
	Token *token = &lexer->token;
	token->start = lexer->cursor;

	switch (CURRENT()) {
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
		lexer_consume_whitespace(lexer);
		lexer_next(lexer);
		break;

	// Operators
	DOUBLE('+', TOKEN_ADD, '=', TOKEN_ADD_ASSIGN)
	DOUBLE('-', TOKEN_SUB, '=', TOKEN_SUB_ASSIGN)
	DOUBLE('*', TOKEN_MUL, '=', TOKEN_MUL_ASSIGN)
	SINGLE('%', TOKEN_MOD)
	DOUBLE('=', TOKEN_ASSIGN, '=', TOKEN_EQ)
	DOUBLE('!', TOKEN_NOT, '=', TOKEN_NEQ)
	TRIPLE('<', TOKEN_LT, '=', TOKEN_LE, '<', TOKEN_LEFT_SHIFT)
	TRIPLE('>', TOKEN_GT, '=', TOKEN_GE, '>', TOKEN_RIGHT_SHIFT)
	DOUBLE('&', TOKEN_BIT_AND, '&', TOKEN_AND)
	DOUBLE('|', TOKEN_BIT_OR, '|', TOKEN_OR)
	DOUBLE('.', TOKEN_DOT, '.', TOKEN_CONCAT)
	SINGLE('^', TOKEN_BIT_XOR)
	SINGLE('~', TOKEN_BIT_NOT)
	SINGLE('(', TOKEN_OPEN_PARENTHESIS)
	SINGLE(')', TOKEN_CLOSE_PARENTHESIS)
	SINGLE('[', TOKEN_OPEN_BRACKET)
	SINGLE(']', TOKEN_CLOSE_BRACKET)
	SINGLE('{', TOKEN_OPEN_BRACE)
	SINGLE('}', TOKEN_CLOSE_BRACE)
	SINGLE(',', TOKEN_COMMA)

	// Comments and division
	case '/':
		CONSUME();
		if (CURRENT() == '/') {
			// Single line comment
			// Consume until end of the next newline
			lexer_consume_eol(lexer);
			lexer_next(lexer);
		} else if (CURRENT() == '*') {
			// Block comment
			lexer_consume_until(lexer, "*/");
			FORWARD(2);
			lexer_next(lexer);
		} else if (CURRENT() == '=') {
			CONSUME();
			token->length = 2;
			token->type = TOKEN_DIV_ASSIGN;
		} else {
			token->length = 1;
			token->type = TOKEN_DIV;
		}
		break;

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
		lexer_number(lexer);
		break;

	// Strings
	case '"':
	case '\'':
		lexer_string(lexer);
		break;

	// Keywords and identifiers
	default:
		lexer_keyword_identifier(lexer);
		break;
	}
}


// Converts the character following a `\` into its corresponding escape
// sequence. Triggers an error if the escape sequence is invalid.
char escape_sequence(char ch) {
	switch (ch) {
	case 'n':  return '\n';
	case 'r':  return '\r';
	case 't':  return '\t';
	case '\'': return '\'';
	case '"':  return '"';
	case '\\': return '\\';

	// Invalid escape sequence
	default:
		return '\0';
	}
}


// Extracts a string from the given token. Returns a heap allocated string
// that needs to be freed. Triggers an error if the string contains an invalid
// escape sequence.
char * lexer_extract_string(Lexer *lexer, Token *token) {
	// Since the string with parsed escape sequences can't be longer than the
	// string in the source code, allocate the same amount of room for each.
	// The length of the token includes the opening and closing quote, so
	// subtract 2, but add 1 for the null terminator.
	size_t length = 0;
	char *result = malloc((token->length - 1) * sizeof(char));

	for (size_t i = 1; i < token->length - 1; i++) {
		// Check for an escape sequence
		if (token->start[i] == '\\') {
			i++;
			char ch = escape_sequence(token->start[i]);
			if (ch == '\0') {
				// Invalid escape sequence
				err_token(lexer->vm, token, "Invalid escape sequence `\\%c`",
					token->start[i]);
				return NULL;
			}

			result[length++] = ch;
		} else {
			// Normal character
			result[length++] = token->start[i];
		}
	}

	// Add the NULL terminator
	result[length++] = '\0';
	return result;
}
