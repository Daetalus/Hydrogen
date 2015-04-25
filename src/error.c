
//
//  Error
//


#include <stdio.h>

#include "error.h"


// Prints an error, using a variable argument struct, rather
// than creating one.
void print_error(int line, char *fmt, va_list args) {
	fprintf(stderr, RED BOLD "error: " WHITE);
	if (line > 0) {
		fprintf(stderr, "line %d: ", line);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, NORMAL "\n");
}


// Triggers a fatal error.
//
// `line` can be -1, in order to prevent the line number from
// being printed.
void error(int line, char *fmt, ...) {
	// Print the error
	va_list args;
	va_start(args, fmt);
	print_error(line, fmt, args);
	va_end(args);

	// Halt the program
	exit(1);
}


// Prints a warning message.
//
// `line` can be -1, in order to prevent the line number from
// being printed.
void warning(int line, char *fmt, ...) {
	// Print the warning
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED BOLD "warning: " WHITE);
	if (line > 0) {
		fprintf(stderr, "line %d: ", line);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, NORMAL "\n");

	va_end(args);
}


// Consumes the next token, triggering an error with the given
// message if it isn't of the expected type.
//
// Returns the consumed token if successful, or a `TOKEN_NONE`
// if the token was of an unexpected type.
Token expect(Lexer *lexer, TokenType expected, char *fmt, ...) {
	Token token = lexer_consume(lexer);

	// If the consumed token is of an unexpected type.
	if (token.type != expected) {
		// Print the error
		va_list args;
		va_start(args, fmt);
		print_error(lexer->line, fmt, args);
		va_end(args);

		// Halt the program
		exit(1);
	} else {
		// The token was what we expected, so return it.
		return token;
	}
}
