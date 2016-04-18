
//
//  Errors
//

#ifndef ERR_H
#define ERR_H

#include <hydrogen.h>
#include <stdarg.h>

#include "lexer.h"
#include "state.h"


// An error struct containing more information that we don't want to expose to
// the user through the C API.
typedef struct {
	// The underlying error object.
	HyError *native;

	// The interpreter state the error was be triggered on.
	HyState *state;

	// The description of the error as it's being constructed. Use a vector so
	// we can expand the length of the description string as it's constructed.
	Vec(char) description;
} Error;


// Create a new error object without any associated information.
Error err_new(HyState *state);

// Print a string to an error's description using a `va_list` as arguments to
// the format string.
void err_print_va(Error *err, char *fmt, va_list args);

// Print a string to an error's description.
void err_print(Error *err, char *fmt, ...);

// Print a token to an error's description, surrounded in grave accents.
void err_print_token(Error *err, Token *token);

// Associate a token with the error.
void err_token(Error *err, Token *token);

// Associate a file with an error object.
void err_file(Error *err, char *file);

// Construct the native error object from the parent error. Free resources
// allocated by the parent at the same time.
HyError * err_make(Error *err);

// Stop execution of the current code and jump back to the error guard. Free
// any resources allocated during error construction.
void err_trigger(Error *err);

#endif
