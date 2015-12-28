
//
//  Hydrogen
//


#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdlib.h>


// The interpreter state.
typedef struct vm HyVM;


// A runtime or compile error.
typedef struct {
	// A description of the error.
	char *description;

	// The line of source code the error occurred on.
	uint32_t line;

	// The column of the line in the source code the error occurred on.
	uint32_t column;

	// The name of the package the error occurred in, or NULL if the error
	// occurred in the main package.
	char *package;

	// The path to the file the error occurred in, or NULL if the source code
	// the error occurred in is not located in a file.
	char *file;
} HyError;


// Create a new interpreter state.
HyVM * hy_new(void);

// Free an interpreter's state.
void hy_free(HyVM *vm);

// Runs the given source code string, returning a pointer to an error object
// if an error occurred, or NULL otherwise. The returned error object must be
// freed.
HyError * hy_run(HyVM *vm, char *source);

// Frees an error.
void hy_err_free(HyError *err);

#endif
