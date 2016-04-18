
//
//  Interpreter State
//

#include <string.h>
#include <stdio.h>

#include "state.h"
#include "err.h"
#include "exec.h"


// The maximum stack size.
#define MAX_STACK_SIZE 2048

// The maximum call stack size storing data for function calls.
#define MAX_CALL_STACK_SIZE 2048


// Execute a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`.
HyError * hy_run_file(HyState *state, char *path) {
	char *name = hy_pkg_name(path);
	HyPackage pkg = hy_add_pkg(state, name);
	HyError *err = hy_pkg_run_file(state, pkg, path);
	free(name);
	return err;
}


// Execute some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`.
HyError * hy_run_string(HyState *state, char *source) {
	HyPackage pkg = hy_add_pkg(state, NULL);
	HyError *err = hy_pkg_run_string(state, pkg, source);
	return err;
}


// Create a new interpreter state.
HyState * hy_new(void) {
	HyState *state = malloc(sizeof(HyState));

	vec_new(state->sources, Source, 4);
	vec_new(state->packages, Package, 4);

	vec_new(state->functions, Function, 8);
	vec_new(state->native_fns, NativeFunction, 8);

	vec_new(state->structs, StructDefinition, 4);
	vec_new(state->native_structs, NativeStructDefinition, 4);

	vec_new(state->constants, HyValue, 32);
	vec_new(state->strings, String *, 16);
	vec_new(state->fields, Identifier, 16);

	state->stack = malloc(sizeof(HyValue) * MAX_STACK_SIZE);
	state->call_stack = malloc(sizeof(Frame) * MAX_CALL_STACK_SIZE);
	state->call_stack_count = 0;

	state->error = NULL;
	return state;
}


// Release all resources allocated by an interpreter state.
void hy_free(HyState *state) {
	// Source files
	for (uint32_t i = 0; i < vec_len(state->sources); i++) {
		Source *src = &vec_at(state->sources, i);
		free(src->file);
		free(src->contents);
	}

	// Packages
	for (uint32_t i = 0; i < vec_len(state->packages); i++) {
		pkg_free(&vec_at(state->packages, i));
	}

	// Functions
	for (uint32_t i = 0; i < vec_len(state->functions); i++) {
		fn_free(&vec_at(state->functions, i));
	}

	// Native functions
	for (uint32_t i = 0; i < vec_len(state->native_fns); i++) {
		native_free(&vec_at(state->native_fns, i));
	}

	// Struct definitions
	for (uint32_t i = 0; i < vec_len(state->structs); i++) {
		struct_free(&vec_at(state->structs, i));
	}

	// Native struct definitions
	for (uint32_t i = 0; i < vec_len(state->native_structs); i++) {
		native_struct_free(&vec_at(state->native_structs, i));
	}

	// Strings
	for (uint32_t i = 0; i < vec_len(state->strings); i++) {
		free(vec_at(state->strings, i));
	}

	// Arrays
	vec_free(state->sources);
	vec_free(state->packages);
	vec_free(state->functions);
	vec_free(state->native_fns);
	vec_free(state->structs);
	vec_free(state->native_structs);
	vec_free(state->constants);
	vec_free(state->strings);
	vec_free(state->fields);

	// Runtime stacks
	free(state->stack);
	free(state->call_stack);

	// The interpreter state itself
	free(state);
}


// Parse and run some source code.
HyError * vm_parse_and_run(HyState *state, HyPackage pkg_index, Index source) {
	Package *pkg = &vec_at(state->packages, pkg_index);

	// Parse the source code
	Index main_fn = 0;
	HyError *err = pkg_parse(pkg, source, &main_fn);

	// Execute the main function if no error occurred
	if (err == NULL) {
		err = exec_fn(state, main_fn);
	}
	return err;
}


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned.
HyError * hy_pkg_run_file(HyState *state, HyPackage pkg, char *path) {
	Index source = state_add_source_file(state, path);

	// Check we could find the file
	if (source == NOT_FOUND) {
		Error err = err_new(state);
		err_print(&err, "Failed to open file");
		err_file(&err, path);
		return err_make(&err);
	}

	return vm_parse_and_run(state, pkg, source);
}


// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned.
HyError * hy_pkg_run_string(HyState *state, HyPackage pkg, char *source) {
	Index source_index = state_add_source_string(state, source);
	return vm_parse_and_run(state, pkg, source_index);
}


// Add a constant to the interpreter state, returning its index.
Index state_add_constant(HyState *state, HyValue constant) {
	vec_inc(state->constants);
	vec_last(state->constants) = constant;
	return vec_len(state->constants) - 1;
}


// Create a new string constant that is `length` bytes long.
Index state_add_literal(HyState *state, uint32_t length) {
	vec_inc(state->strings);
	vec_last(state->strings) = malloc(sizeof(String) + length + 1);
	String *string = vec_last(state->strings);
	string->type = OBJ_STRING;
	string->length = length;
	string->contents[0] = '\0';
	return vec_len(state->strings) - 1;
}


// Add a field name to the interpreter state's fields list. If a field matching
// `ident` already exists, then it returns the index of the existing field.
Index state_add_field(HyState *state, Identifier ident) {
	// Check for an existing field first (reverse order)
	for (int i = vec_len(state->fields) - 1; i >= 0; i--) {
		Identifier *match = &vec_at(state->fields, (uint32_t) i);
		if (ident.length == match->length &&
				strncmp(ident.name, match->name, ident.length) == 0) {
			return i;
		}
	}

	// No existing field, so add a new one
	vec_inc(state->fields);
	Identifier *last = &vec_last(state->fields);
	last->name = ident.name;
	last->length = ident.length;
	return vec_len(state->fields) - 1;
}


// Return the contents of a file.
static char * file_contents(char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		return NULL;
	}

	// Get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);

	// Read its contents
	char *contents = malloc(length + 1);
	fread(contents, sizeof(char), length, f);
	fclose(f);
	contents[length] = '\0';
	return contents;
}


// Add a file as a source code object on the interpreter.
Index state_add_source_file(HyState *state, char *path) {
	// Read the contents of the file
	char *contents = file_contents(path);
	if (contents == NULL) {
		return NOT_FOUND;
	}

	vec_inc(state->sources);
	Source *src = &vec_last(state->sources);
	src->contents = contents;

	// Copy the file path into our own heap allocated string
	src->file = malloc(strlen(path) + 1);
	strcpy(src->file, path);
	return vec_len(state->sources) - 1;
}


// Add a string as a source code object on the interpreter.
Index state_add_source_string(HyState *state, char *source) {
	vec_inc(state->sources);
	Source *src = &vec_last(state->sources);
	src->file = NULL;

	// Copy the source code into our own heap allocated string
	src->contents = malloc(strlen(source) + 1);
	strcpy(src->contents, source);
	return vec_len(state->sources) - 1;
}
