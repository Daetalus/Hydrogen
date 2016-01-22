
//
//  Virtual Machine
//

#include <stdio.h>

#include "vm.h"
#include "err.h"


// Executes a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`.
HyError * hy_run_file(char *path) {
	HyState *state = hy_new();
	char *name = hy_package_name(path);
	HyPackage pkg = hy_package_new(state, name);
	HyError *err = hy_package_run_file(state, pkg, path);
	free(name);
	hy_free(state);
	return err;
}


// Executes some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`.
HyError * hy_run_string(char *source) {
	HyState *state = hy_new();
	HyPackage pkg = hy_package_new(state, NULL);
	HyError *err = hy_package_run_string(state, pkg, source);
	hy_free(state);
	return err;
}


// Create a new interpreter state.
HyState * hy_new(void) {
	HyState *state = malloc(sizeof(HyState));
	vec_new(state->packages, Package, 4);
	vec_new(state->functions, Function, 8);
	vec_new(state->natives, NativeFunction, 8);
	vec_new(state->structs, StructDefinition, 8);
	vec_new(state->constants, HyValue, 32);
	vec_new(state->strings, String *, 16);
	vec_new(state->fields, Identifier, 16);
	state->error = NULL;
	return state;
}


// Release all resources allocated by an interpreter state.
void hy_free(HyState *state) {
	for (uint32_t i = 0; i < vec_len(state->packages); i++) {
		pkg_free(&vec_at(state->packages, i));
	}
	for (uint32_t i = 0; i < vec_len(state->functions); i++) {
		fn_free(&vec_at(state->functions, i));
	}
	for (uint32_t i = 0; i < vec_len(state->natives); i++) {
		native_free(&vec_at(state->natives, i));
	}
	for (uint32_t i = 0; i < vec_len(state->structs); i++) {
		struct_free(&vec_at(state->structs, i));
	}
	for (uint32_t i = 0; i < vec_len(state->strings); i++) {
		free(vec_at(state->strings, i));
	}

	vec_free(state->packages);
	vec_free(state->functions);
	vec_free(state->natives);
	vec_free(state->structs);
	vec_free(state->constants);
	vec_free(state->strings);
	vec_free(state->fields);
}


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned.
HyError * hy_package_run_file(HyState *state, HyPackage index, char *path) {
	Package *pkg = &vec_at(state->packages, index);
	Index source = pkg_add_file(pkg, path);

	// Check we could find the file
	if (source == NOT_FOUND) {
		// Failed to open file
		HyError *err = err_new();
		err_print(err, "Failed to open file `%s`", path);
		return err;
	} else {
		return pkg_run(pkg, source);
	}
}


// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned.
HyError * hy_package_run_string(HyState *state, HyPackage index, char *source) {
	Package *pkg = &vec_at(state->packages, index);
	Index source_index = pkg_add_string(pkg, source);
	return pkg_run(pkg, source_index);
}



//
//  Execution
//

// Executes a function on the interpreter state.
HyError * vm_run_fn(HyState *state, Index fn_index) {
	Function *fn = &vec_at(state->functions, fn_index);
	printf("running fn %d, named %.*s\n", fn_index, fn->length, fn->name);
	return NULL;
}
