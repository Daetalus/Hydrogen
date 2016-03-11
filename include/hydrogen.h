
//
//  Hydrogen
//

#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdint.h>
#include <stdbool.h>


// The interpreter state, used to execute Hydrogen source code. Variables,
// functions, etc. are preserved by the state across calls to `hy_run`
typedef struct hy_state HyState;

// Stores package specific data like functions and structs defined in the
// package
typedef uint32_t HyPackage;

// A type that represents all possible values a variable can hold
typedef uint64_t HyValue;

// A list of arguments passed to a native function
typedef struct hy_args HyArgs;


// The prototype for a native function
typedef HyValue (* HyNativeFn)(HyState *state, HyArgs *args);


// Contains data describing an error
typedef struct {
	// A description of the error that occurred
	char *description;

	// The path to the file the error occurred in, or NULL if the error didn't
	// occur in a file
	char *file;

	// The line number and column in the file the error occurred on, or 0 if
	// the error has no associated source code
	uint32_t line;
	uint32_t column;

	// The contents of the line in the file the error occurred on, or NULL if
	// the error has no associated source code. Does not include the newline
	// character at the end of the line
	char *code;

	// The length of the token that triggered the error, or 0 if the error has
	// no associated source code
	uint32_t length;
} HyError;


// Create a new interpreter state
HyState * hy_new(void);

// Executes a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`
HyError * hy_run_file(HyState *state, char *path);

// Executes some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`
HyError * hy_run_string(HyState *state, char *source);

// Release all resources allocated by an interpreter state
void hy_free(HyState *state);

// Release resources allocated by an error object
void hy_err_free(HyError *err);

// Create a new package on the interpreter state. The name of the package is
// used when other packages want to import it. It can only consist of ASCII
// letters (lowercase and uppercase), numbers, and underscores
HyPackage hy_add_pkg(HyState *state, char *name);

// Returns a heap allocated string (that needs to be freed) containing the name
// of a package based off its file path
char * hy_pkg_name(char *path);


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned
HyError * hy_pkg_run_file(HyState *state, HyPackage pkg, char *path);

// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned
HyError * hy_pkg_run_string(HyState *state, HyPackage pkg, char *source);


// Read source code from a file and parse it into bytecode, printing it to
// the standard output
HyError * hy_print_bytecode_file(HyState *state, HyPackage pkg, char *path);

// Parse source code into bytecode and print it to the standard output. An
// error object is returned if one occurred during parsing, otherwise NULL
// is returned
HyError * hy_print_bytecode_string(HyState *state, HyPackage pkg, char *source);


// Add a native function to a package. `arity` is the number of arguments the
// function accepts. If it is set to -1, then the function can accept any number
// of arguments
void hy_add_native(HyState *state, HyPackage pkg, char *name, int32_t arity,
	HyNativeFn fn);



//
//  Values
//

// The possible types of a value
typedef enum {
	HY_NIL,
	HY_BOOL,
	HY_NUMBER,
	HY_STRING,
	HY_STRUCT,
	HY_FUNCTION,
} HyType;


// Returns a nil value
HyValue hy_nil(void);

// Converts a boolean into a value
HyValue hy_bool(bool boolean);

// Converts a number into a value
HyValue hy_number(double number);

// Copies a string into a garbage collected value
HyValue hy_string(HyState *state, char *string);

// Returns the type of a value
HyType hy_type(HyValue value);

// Returns true if a value is nil or not
bool hy_is_nil(HyValue value);

// Converts a value to a boolean, ignoring the type of the value
bool hy_to_bool(HyValue value);

// Converts a value into a boolean, triggering an error if the value is not a
// boolean in type
bool hy_expect_bool(HyValue value);

// Converts a value into a number, triggering an error if the value isn't a
// number
double hy_expect_number(HyValue value);

// Converts a value into a string, triggering an error if it isn't a string
// Do not try and free the returned string. It will be garbage collected later
char * hy_expect_string(HyValue value);

// Returns the number of arguments passed to a native function
uint32_t hy_args_count(HyArgs *args);

// Returns the `index`th argument passed to a native function
HyValue hy_arg(HyArgs *args, uint32_t index);

#endif
