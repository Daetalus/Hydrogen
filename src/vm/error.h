
//
//  Error
//

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <hydrogen.h>

#include "lexer.h"

// Returns a new, custom error.
HyError err_new(int line, char *fmt, ...);

// Returns an unexpected token error.
HyError err_unexpected(int line, Token token, TokenValue value, char *fmt, ...);

// Frees an error.
void err_free(HyError *err);

#endif
