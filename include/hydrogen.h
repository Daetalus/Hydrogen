
//
//  Hydrogen
//

#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <stdint.h>
#include <stdbool.h>

// Used to specify a variable argument function.
#define HY_VAR_ARG (~((uint32_t) 0))


// The interpreter state, used to execute Hydrogen source code. Variables,
// functions, etc. are preserved by the state across calls to `hy_run`.
typedef struct hy_state HyState;

// Stores package specific data like functions and structs defined in the
// package.
typedef uint32_t HyPackage;

// Represents a native struct.
typedef uint32_t HyStruct;

// A type that represents all possible values a variable can hold.
typedef uint64_t HyValue;

// A list of arguments passed to a native function.
typedef struct hy_args HyArgs;


// The prototype for a native function.
typedef HyValue (* HyNativeFn)(HyState *state, HyArgs *args);

// The prototype for a method on a native struct.
typedef HyValue (* HyNativeMethod)(HyState *state, void *data, HyArgs *args);

// The prototype for a destructor on a native struct.
typedef void (* HyDestructor)(HyState *state, void *data);


// Contains data describing an error.
typedef struct {
	// A description of the error that occurred.
	char *description;

	// The path to the file the error occurred in, or NULL if the error didn't
	// occur in a file.
	char *file;

	// The line number and column in the file the error occurred on, or 0 if
	// the error has no associated source code.
	uint32_t line;
	uint32_t column;

	// The contents of the line in the file the error occurred on, or NULL if
	// the error has no associated source code. Does not include the newline
	// character at the end of the line.
	char *code;

	// The length of the token that triggered the error, or 0 if the error has
	// no associated source code.
	uint32_t length;
} HyError;


// Create a new interpreter state.
HyState * hy_new(void);

// Execute a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Act as a wrapper around other API
// functions. Return an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`.
HyError * hy_run_file(HyState *state, char *path);

// Execute some source code from a string. Return an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`.
HyError * hy_run_string(HyState *state, char *source);

// Release all resources allocated by an interpreter state.
void hy_free(HyState *state);

// Release resources allocated by an error object.
void hy_err_free(HyError *err);

// Create a new package on the interpreter state. The name of the package is
// used to import it from other packages. It can only consist of ASCII letters
// (lowercase and uppercase), numbers, and underscores.
HyPackage hy_add_pkg(HyState *state, char *name);

// Return a heap allocated string (that needs to be freed) containing the name
// of a package based off its file path.
char * hy_pkg_name(char *path);


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned.
HyError * hy_pkg_run_file(HyState *state, HyPackage pkg, char *path);

// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned.
HyError * hy_pkg_run_string(HyState *state, HyPackage pkg, char *source);


// Read source code from a file and parse it into bytecode, printing it to
// the standard output.
HyError * hy_print_bytecode_file(HyState *state, HyPackage pkg, char *path);

// Parse source code into bytecode and print it to the standard output. An
// error object is returned if one occurred during parsing, otherwise NULL
// is returned.
HyError * hy_print_bytecode_string(HyState *state, HyPackage pkg, char *source);


// Add a native function to a package. `arity` is the number of arguments the
// function accepts. If it is set to HY_VAR_ARG, then the function can accept
// any number of arguments.
void hy_add_fn(HyState *state, HyPackage pkg, char *name, uint32_t arity,
	HyNativeFn fn);


// Add a native struct to a package. `size` specifies how much memory (in bytes)
// needed in each instance of the struct. `constructor` is called every time an
// instance of the struct is instantiated.
HyStruct hy_add_struct(HyState *state, HyPackage pkg, char *name, uint32_t size,
	HyNativeMethod constructor, uint32_t constructor_arity);

// Set the destructor on a native struct, called every time an instance of the
// struct is garbage collected, to allow you to free any associated resources.
void hy_set_destructor(HyState *state, HyStruct def, HyDestructor destructor);

// Add a method on a native struct.
void hy_add_method(HyState *state, HyStruct def, char *name, uint32_t arity,
	HyNativeMethod method);



//
//  Values
//

// The possible types of a value.
typedef enum {
	HY_NIL,
	HY_BOOL,
	HY_NUMBER,
	HY_STRING,
	HY_STRUCT,
	HY_METHOD,
	HY_ARRAY,
	HY_FUNCTION,
} HyType;


// A Hydrogen array.
typedef struct hy_array HyArray;


// Return a nil value.
HyValue hy_nil(void);

// Convert a boolean into a value.
HyValue hy_bool(bool boolean);

// Convert a number into a value.
HyValue hy_number(double number);

// Copy a string into a garbage collected value.
HyValue hy_string(HyState *state, char *string);

// Turn an array into a value.
HyValue hy_array(HyArray *array);


// Return the type of a value.
HyType hy_type(HyValue value);

// Return true if a value is nil or not.
bool hy_is_nil(HyValue value);

// Convert a value to a boolean, ignoring the type of the value.
bool hy_to_bool(HyValue value);

// Convert a value into a boolean, triggering an error if the value is not a
// boolean in type.
bool hy_expect_bool(HyValue value);

// Convert a value into a number, triggering an error if the value isn't a
// number.
double hy_expect_number(HyValue value);

// Convert a value into a string, triggering an error if it isn't a string
//
// Do not try and free the returned string. It will be garbage collected later.
//
// Don't modify the returned string. It should be treated as read only, and a
// copy should be made if you want to modify it.
char * hy_expect_string(HyValue value);

// Convert a value into an array, triggering an error if it isn't one.
HyArray * hy_expect_array(HyValue value);


// Return the number of arguments passed to a native function.
uint32_t hy_args_count(HyArgs *args);

// Return the argument at `index` passed to a native function.
HyValue hy_arg(HyArgs *args, uint32_t index);


// Create an empty array with the suggested capacity.
HyArray * hy_array_new(uint32_t capacity);

// Return the length of an array.
uint32_t hy_array_len(HyArray *array);

// Fetch a value at an index in an array.
HyValue hy_array_get(HyArray *array, uint32_t index);

// Append a value to the end of an array.
void hy_array_append(HyArray *array, HyValue value);

// Insert a value into an array at the specified index.
void hy_array_insert(HyArray *array, uint32_t index, HyValue value);

#endif
