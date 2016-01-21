
//
//  Packages
//

#ifndef PKG_H
#define PKG_H

#include <hydrogen.h>


// Some source code, either from a file or string.
typedef struct {
	// The path to the file the source code came from, or NULL if the source
	// code didn't come from a file.
	char *file;

	// The source code itself.
	char *contents;
} Source;


// A package is a collection of variables (including functions, since function
// are variables), and struct definitions.
typedef struct {
	// The name of the package, used when the user wants to import the package
	// from somewhere.
	char *name;

	// A package can have multiple source code locations associated with it,
	// from files, strings, or definitions made using the API. So we need an
	// array of source locations.
	Vec(Source) sources;

	// A parser, to generate bytecode from source code. This is kept in the
	// package so we can save which variables we've defined, etc for each time
	// we compile some source code into this package.
	Parser parser;

	// Variables declared at the top of a source file must be available to
	// external packages, and therefore can't be defined on the stack. They're
	// instead stored here, in the package itself. The name of each local is
	// stored in a separate array.
	Vec(Identifier) names;
	Vec(HyValue) locals;
} Package;


// Defines a new package on the interpreter state. Returns the index of the
// package.
Index pkg_new(HyState *state);

// Releases resources allocated by a package.
void pkg_free(Package *pkg);

// Executes a source object on a package by compiling into bytecode and
// executing the result.
HyError * pkg_run(Package *pkg, Index source);

// Adds a file as a source on the package.
Index pkg_add_file(Package *pkg, char *path);

// Adds a string as a source on the package.
Index pkg_add_string(Package *pkg, char *source);

// Finds a package with the name `name`.
Index pkg_find(HyState *state, char *name, uint32_t length);

// Adds a new top level local to a package with a default value of `value`.
Index pkg_local_add(Package *pkg, char *name, uint32_t length, HyValue value);

// Finds the index of a local with the name `name`.
Index pkg_local_find(Package *pkg, char *name, uint32_t length);

#endif
