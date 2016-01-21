
//
//  Hydrogen
//


#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdint.h>
#include <stdbool.h>


// The interpreter state, used to execute Hydrogen source code. Variables,
// functions, etc. are preserved by the state across calls to `hy_run`.
typedef struct hy_state HyState;

// Stores package specific data like functions and structs defined in the
// package.
typedef uint32_t HyPackage;

// A type that represents all possible values a variable can hold.
typedef uint64_t HyValue;

// A list of arguments passed to a native function.
typedef struct hy_args HyArgs;


// The prototype for a native function.
typedef HyValue (* HyNativeFn)(HyState *state, HyArgs *args);


// The origin of an element in a stack trace.
typedef enum {
	HY_TRACE_FUNCTION,
	HY_TRACE_METHOD,
	HY_TRACE_PACKAGE,
	HY_TRACE_ANONYMOUS_PACKAGE,
} HyStackTraceType;


// An element in a stack trace. Describes the location of a function call.
typedef struct {
	// The type of the stack trace element.
	HyStackTraceType type;

	// The path to the file containing the function call.
	char *file;

	// The line number in the file the function call is on.
	uint32_t line;

	// The name of the function being called. NULL if the type is an anonymous
	// package (which doesn't have a name).
	char *name;
} HyStackTrace;


// Contains data describing an error.
typedef struct {
	// A description of the error that occurred.
	char *description;

	// The path to the file the error occurred in, or NULL if the error didn't
	// occur in a file.
	char *file;

	// The contents of the line in the file the error occurred on, or NULL if
	// the error has no associated source code. Does not include the newline
	// character at the end of the line.
	char *line_contents;

	// The line number and column in the file the error occurred on, or -1 if
	// the error has no associated source code.
	int32_t line;
	int32_t column;

	// The length of the token that triggered the error, or -1 if the error has
	// no associated source code.
	int32_t length;

	// The state of the call stack at the point during runtime the error
	// occurred. If the error was during compilation, this is set to NULL, and
	// the length is set to 0.
	HyStackTrace *stack_trace;
	uint32_t stack_trace_length;
} HyError;


// Executes a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`.
HyError * hy_run_file(char *path);

// Executes some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`.
HyError * hy_run_string(char *source);


// Create a new interpreter state.
HyState * hy_new(void);

// Release all resources allocated by an interpreter state.
void hy_free(HyState *state);

// Release resources allocated by an error object.
void hy_err_free(HyError *err);

// Create a new package on the interpreter state. The name of the package is
// used when other packages want to import it. It can only consist of ASCII
// letters (lowercase and uppercase), numbers, and underscores.
HyPackage hy_package_new(HyState *state, char *name);

// Returns a heap allocated string (that needs to be freed) containing the name
// of a package based off its file path.
char * hy_package_name(char *path);


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned.
HyError * hy_package_run_file(HyState *state, HyPackage pkg, char *path);

// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned.
HyError * hy_package_run_string(HyState *state, HyPackage pkg, char *source);


// Read source code from a file and compile it into bytecode, printing it to
// the standard output.
HyError * hy_print_bytecode_file(HyState *state, HyPackage pkg, char *path);

// Compile source code into bytecode and print it to the standard output. An
// error object is returned if one occurred during compilation, otherwise NULL
// is returned.
HyError * hy_print_bytecode_string(HyState *state, HyPackage pkg, char *source);


// Add a native function to a package. `arity` is the number of arguments the
// function accepts. If it is set to -1, then the function can accept any number
// of arguments.
void hy_add_native(HyState *state, HyPackage pkg, char *name, int32_t arity,
	HyNativeFn fn);

#endif
