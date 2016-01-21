
//
//  Package
//

#include <string.h>
#include <stdio.h>

#include "pkg.h"
#include "vm.h"


// Create a new package on the interpreter state. The name of the package is
// used when other packages want to import it. It can only consist of ASCII
// letters (lowercase and uppercase), numbers, and underscores.
HyPackage hy_package_new(HyState *state, char *name) {
	// Create a new package
	Index index = pkg_new(state);
	Package *pkg = &vec_at(state->packages, index);

	// Copy the name of the package across into a new heap allocated string
	pkg->name = malloc(strlen(name) + 1);
	strcpy(pkg->name, name);
	return index;
}


// Returns a heap allocated string (that needs to be freed) containing the name
// of a package based off its file path.
char * hy_package_name(char *path) {
	// TODO
	return NULL;
}


// Add a folder to search through when resolving the file paths of imported
// packages.
void hy_search(HyState *state, HyPackage pkg, char *path) {
	// TODO
}


// Defines a new package on the interpreter state. Returns the index of the
// package.
Index pkg_new(HyState *state) {
	vec_add(state->packages);
	Package *pkg = vec_last(state->packages);
	pkg->name = NULL;
	vec_new(pkg->sources, Source, 4);
	pkg->parser = parser_new(state, vec_len(state->packages) - 1);
	vec_new(pkg->names, Identifier, 8);
	vec_new(pkg->locals, HyValue, 8);
	return vec_len(state->packages) - 1;
}


// Releases resources allocated by a package.
void pkg_free(Package *pkg) {
	for (uint32_t i = 0; i < vec_len(pkg->sources); i++) {
		Source *src = &vec_at(pkg->sources, i);
		free(src->file);
		free(src->contents);
	}

	free(pkg->name);
	vec_free(pkg->sources);
	vec_free(pkg->names);
	vec_free(pkg->locals);
}


// Executes a source object on a package by compiling into bytecode and
// executing the result.
HyError * pkg_run(Package *pkg, Index source) {
	HyState *state = pkg->parser.state;

	// Catch errors
	Index main_fn = NOT_FOUND;
	if (setjmp(state->error_jmp) == 0) {
		// Compile source code
		main_fn = parser_parse(&pkg->parser, source);
	}

	// Check for compilation error
	if (state->error != NULL) {
		return state->error;
	}

	// Catch errors
	if (setjmp(state->error_jmp) == 0) {
		// Execute compiled bytecode
		vm_run_fn(state, main_fn);
	}

	// Return runtime error
	return state->error;
}


// Returns the contents of a file.
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
	char *contents = malloc(sizeof(char) * (length + 1));
	fread(contents, sizeof(char), length, f);
	fclose(f);
	contents[length] = '\0';
	return contents;
}


// Adds a file as a source on the package.
Index pkg_add_file(Package *pkg, char *path) {
	// Read the contents of the file
	char *contents = file_contents(path);
	if (contents == NULL) {
		return NOT_FOUND;
	}

	vec_add(pkg->sources);
	Source *src = vec_last(pkg->sources);
	src->contents = contents;

	// Copy the file path into our own heap allocated string
	src->file = malloc(strlen(path) + 1);
	strcpy(src->file, path);
	return vec_len(pkg->sources) - 1;
}


// Adds a string as a source on the package.
Index pkg_add_string(Package *pkg, char *source) {
	vec_add(pkg->sources);
	Source *src = vec_last(pkg->sources);
	src->file = NULL;

	// Copy the source code into our own heap allocated string
	src->contents = malloc(strlen(source) + 1);
	strcpy(src->contents, source);
	return vec_len(pkg->sources) - 1;
}


// Finds a package with the name `name`.
Index pkg_find(HyState *state, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(state->packages); i++) {
		Package *pkg = &vec_at(state->packages, i);
		if (length == strlen(pkg->name) &&
				strncmp(name, pkg->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}


// Adds a new top level local to a package with a default value of `value`.
Index pkg_local_add(Package *pkg, char *name, uint32_t length, HyValue value) {
	vec_add(pkg->locals);
	vec_add(pkg->names);

	Identifier *ident = vec_last(pkg->names);
	ident->name = name;
	ident->length = length;
	vec_at(pkg->locals, vec_len(pkg->locals) - 1) = value;
	return vec_len(pkg->names) - 1;
}


// Finds the index of a local with the name `name`.
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
