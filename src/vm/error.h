
//
//  Error
//

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <hydrogen.h>

#include "lexer.h"

// Returns a new, custom error on the given token.
HyError err_new(Token *token, char *fmt, ...);

// Returns an unexpected token error.
HyError err_unexpected(Token *token, char *fmt, ...);

// Frees an error.
void err_free(HyError *err);

#endif
