
//
//  Hydrogen
//


#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdlib.h>


// The interpreter state.
typedef struct vm HyVM;


// Possible results for the execution of source code.
typedef enum {
	HY_SUCCESS,
	HY_COMPILE_ERROR,
	HY_RUNTIME_ERROR,
} HyResult;


// A runtime or compile error.
typedef struct {
	// A description of the error.
	char *description;

	// The line of source code the error occurred on.
	uint32_t line;

	// The column of the line in the source code the error occurred on.
	uint32_t column;

	// The name of the package the error occurred in, or NULL if the package is
	// anonymous.
	char *package;

	// The path to the file the error occurred in, or NULL if the source code
	// the error occurred in is not located in a file.
	char *file;
} HyError;


// Create a new interpreter state.
HyVM * hy_new(void);

// Free an interpreter's state.
void hy_free(HyVM *vm);

// Runs the given source code string.
HyResult hy_exec_string(HyVM *vm, char *source);

// Returns the most recent error that has occurred. Do not attempt to free the
// returned pointer!
HyError * hy_error(HyVM *vm);

#endif
