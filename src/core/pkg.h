
//
//  Packages
//

#ifndef PKG_H
#define PKG_H

#include <hydrogen.h>

#include "parser.h"


// A package is a collection of variables (including functions, since function
// are variables), and struct definitions.
typedef struct {
	// The name of the package, used when the user wants to import the package
	// from somewhere.
	char *name;

	// A parser, to generate bytecode from source code. This is kept in the
	// package so we can save which variables we've defined, etc for each time
	// we parse some source code into bytecode on this package.
	Parser parser;

	// Variables declared at the top of a source file must be available to
	// external packages, and therefore can't be defined on the stack. They're
	// instead stored here, in the package itself. The name of each local is
	// stored in a separate array.
	Vec(Identifier) names;
	Vec(HyValue) locals;
} Package;


// Define a new package on the interpreter state. Return the index of the new
// package.
Index pkg_new(HyState *state);

// Release resources allocated by a package.
void pkg_free(Package *pkg);

// Parse some source code into bytecode, returning an error if one occurred,
// and setting `main_fn` to the index of the function that will execute the
// code at the top level of the package.
HyError * pkg_parse(Package *pkg, Index source, Index *main_fn);

// Find a package with the name `name`.
Index pkg_find(HyState *state, char *name, uint32_t length);

// Add a new top level local to a package with a default value of `value`.
Index pkg_local_add(Package *pkg, char *name, uint32_t length, HyValue value);

// Find the index of a local with the name `name`.
Index pkg_local_find(Package *pkg, char *name, uint32_t length);

#endif
