
//
//  Errors
//

#ifndef ERR_H
#define ERR_H

#include <hydrogen.h>
#include <stdarg.h>

#include "lexer.h"

// Creates a new error object without any associated details yet. The error can
// be constructed using the building functions below.
HyError * err_new(void);

// Prints a string to an error's description.
void err_print(HyError *err, char *fmt, ...);

// Prints a token to an error's description, surrounded in grave accents.
void err_print_token(HyError *err, Token *token);

// Associate a token with the error.
void err_token(HyState *state, HyError *err, Token *token);

// Triggers a jump back to the error guard with the built error.
void err_trigger(HyState *state, HyError *err);

#endif
