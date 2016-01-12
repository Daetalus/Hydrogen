
//
//  Hydrogen
//


#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdlib.h>
#include <stdbool.h>


// The interpreter state.
typedef struct vm HyVM;


// A runtime or compile error.
typedef struct {
	// A description of the error.
	char *description;

	// The line of source code the error occurred on. -1 if there's no line
	// number associated with the error.
	int line;

	// The column of the line in the source code the error occurred on. -1 if
	// there is no column associated with the error.
	int column;

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

// Runs a file, returning an error if one occurred, or NULL otherwise.
HyError * hy_run_file(HyVM *vm, char *path);

// Frees an error.
void hy_err_free(HyError *err);



//
//  API
//

// A value of a variable.
typedef uint64_t HyValue;

// A native package.
typedef struct native_package HyNativePackage;

// Arguments to a native function
typedef struct fn_args HyArgs;

// A native function.
typedef HyValue (* HyNativeFn)(HyVM *, HyArgs *);

// The type of a variable.
typedef enum {
	HY_NUMBER,
	HY_STRING,
	HY_STRUCT,
	HY_FN,
	HY_BOOLEAN,
	HY_NIL,
} HyType;


// Define a native package on an interpreter with the given name.
HyNativePackage * hy_package_new(HyVM *vm, char *name);

// Defines a native function on a package with the given name and number of
// arguments. If `arity` is -1, then the function can accept an arbitrary
// number of arguments.
void hy_fn_new(HyNativePackage *package, char *name, int arity, HyNativeFn fn);


// Directly triggers an error.
// TODO: Make varargs
void hy_trigger_error(HyVM *vm, char *message);

// Run the garbage collector.
void hy_collect_garbage(HyVM *vm);


// Returns the number of arguments supplied to a function.
uint32_t hy_args_count(HyArgs *args);

// Returns the `n`th argument supplied to a native function. Returns nil if the
// requested argument's index is greater than the number of arguments supplied
// to the function.
HyValue hy_arg(HyArgs *args, uint32_t n);

// Implicitly converts a value to a boolean, not triggering an error.
bool hy_to_bool(HyValue value);

// Expects a boolean value.
bool hy_expect_bool(HyValue value);

// Expects a value to be a number, triggreing an error if it isn't.
double hy_expect_number(HyValue value);

// Expects a value to be a string, triggering an error if it isn't. Do not
// attempt to free the returned string! It will be garbage collected at a later
// point.
char * hy_expect_string(HyValue value);


// Returns a nil value.
HyValue hy_nil(void);

// Converts a boolean into a value.
HyValue hy_bool(bool value);

// Converts a string into a value.
HyValue hy_string(char *value);

// Converts a number into a value.
HyValue hy_number(double value);

// Returns the type of a variable.
HyType hy_type(HyValue value);


// TODO: Native structs, struct reflection

#endif
