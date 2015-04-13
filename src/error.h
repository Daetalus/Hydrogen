
//
//  Error
//


#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

#include "lexer.h"


// Color codes
#define NORMAL  "\x1B[0m"
#define BOLD    "\x1B[1m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"


// Triggers a fatal error.
//
// `line` can be -1, in order to not print a line number.
void error(int line, char *fmt, ...);

// Prints a warning message.
//
// Provide -1 as the line number to not print a line.
void warning(int line, char *fmt, ...);

// Consumes the next token, triggering an error with the given
// message if it isn't of the expected type.
//
// Returns the consumed token if successful, or a `TOKEN_NONE`
// if the token was of an unexpected type.
Token expect(Lexer *lexer, TokenType expected, char *fmt, ...);

#endif
