
//
//  Package
//

#include <string.h>
#include <stdio.h>

#include "pkg.h"
#include "vm.h"


// Create a new package on the interpreter state. The name of the package is
// used when other packages want to import it. It can only consist of ASCII
// letters (lowercase and uppercase), numbers, and underscores
HyPackage hy_add_pkg(HyState *state, char *name) {
	// Create a new package
	Index index = pkg_new(state);
	Package *pkg = &vec_at(state->packages, index);

	// Copy the name of the package across into a new heap allocated string
	if (name != NULL) {
		pkg->name = malloc(strlen(name) + 1);
		strcpy(pkg->name, name);
	}

	return index;
}


// Defines a new package on the interpreter state. Returns the index of the
// package
Index pkg_new(HyState *state) {
	vec_inc(state->packages);
	Package *pkg = &vec_last(state->packages);
	Index index = vec_len(state->packages) - 1;
	pkg->name = NULL;
	pkg->parser = parser_new(state, index);
	vec_new(pkg->names, Identifier, 8);
	vec_new(pkg->locals, HyValue, 8);
	return index;
}


// Releases resources allocated by a package
void pkg_free(Package *pkg) {
	free(pkg->name);
	parser_free(&pkg->parser);
	vec_free(pkg->names);
	vec_free(pkg->locals);
}


// Parses some source code into bytecode, returning an error if one occurred,
// and setting `main_fn` to the index of the function that will execute the
// code at the top level of the provided source code
HyError * pkg_parse(Package *pkg, Index source, Index *main_fn) {
	HyState *state = pkg->parser.state;

	// Catch errors
	Index index = NOT_FOUND;
	if (setjmp(state->error_jmp) == 0) {
		// Parse the source
		index = parser_parse(&pkg->parser, source);
	}

	// Check for error
	if (state->error != NULL) {
		// Reset the error
		return state_reset_error(state);
	}

	// Set the main function's index
	if (main_fn != NULL) {
		*main_fn = index;
	}
	return NULL;
}


// Finds a package with the name `name`
Index pkg_find(HyState *state, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(state->packages); i++) {
		Package *pkg = &vec_at(state->packages, i);
		if (pkg->name != NULL && length == strlen(pkg->name) &&
				strncmp(name, pkg->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}


// Adds a new top level local to a package with a default value of `value`
Index pkg_local_add(Package *pkg, char *name, uint32_t length, HyValue value) {
	vec_inc(pkg->locals);
	vec_inc(pkg->names);

	Identifier *ident = &vec_last(pkg->names);
	ident->name = name;
	ident->length = length;
	vec_at(pkg->locals, vec_len(pkg->locals) - 1) = value;
	return vec_len(pkg->names) - 1;
}


// Finds the index of a local with the name `name`
Index pkg_local_find(Package *pkg, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(pkg->names); i++) {
		Identifier *ident = &vec_at(pkg->names, i);
		if (length == ident->length &&
				strncmp(name, ident->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}
