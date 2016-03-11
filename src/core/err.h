
//
//  Errors
//

#ifndef ERR_H
#define ERR_H

#include <hydrogen.h>
#include <stdarg.h>

#include "lexer.h"
#include "vm.h"


// An error struct containing more information that we don't want to expose to
// the user through the C API
typedef struct {
	// The underlying error object
	HyError *native;

	// The interpreter state the error will be triggered on
	HyState *state;

	// The description of the error as it's being constructed
	Vec(char) description;
} Error;


// Creates a new error object without any associated details yet. The error can
// be constructed using the building functions below
Error err_new(HyState *state);

// Prints a string to an error's description using a `va_list` as arguments to
// the format string
void err_print_va(Error *err, char *fmt, va_list args);

// Prints a string to an error's description
void err_print(Error *err, char *fmt, ...);

// Prints a token to an error's description, surrounded in grave accents
void err_print_token(Error *err, Token *token);

// Associates a line of source code with the error
void err_code(Error *err, Token *token);

// Associates a file with an error object
void err_file(Error *err, char *file);

// Constructs the native error object from the parent error, freeing resources
// allocated by the parent at the same time
HyError * err_make(Error *err);

// Stops execution of the current code and jumps back to the error guard. Frees
// any resources allocated by the error construction
void err_trigger(Error *err);

#endif
