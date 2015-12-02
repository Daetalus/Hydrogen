
//
//  Error
//


#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <hydrogen.h>

#include "lexer.h"

// Create a new error.
void err_new(HyError *err, int line, char *fmt, ...);

// Free an error.
void err_free(HyError *err);

// Create an unexpected token error for the most recent token on the lexer.
void err_unexpected(HyError *err, Lexer *lexer, char *fmt, ...);

#endif
