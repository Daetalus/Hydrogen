
//
//  Import Parsing
//

#ifndef IMPORT_H
#define IMPORT_H

#include "parser.h"

// The type of an imported package.
typedef enum {
	IMPORT_USER,
	IMPORT_NATIVE,
} ImportType;


// An imported package.
typedef struct import {
	// The type of an import.
	ImportType type;

	// The index of the package (native or user) in the corresponding VM's
	// package list.
	uint32_t index;
} Import;


// All imported packages.
typedef struct imports {
	ARRAY(Import, imports);
} Imports;


// Searches for an imported package in the parser with the given name,
// returning NULL if the package couldn't be found.
Import * import_package_find(Parser *parser, char *name, size_t length);

// Extracts a package's actual name from its provided path, returning NULL if
// the path is invalid.
char * import_package_name(char *path);

// Imports a package with the given path and name.
void import(Parser *parser, char *path);

// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser);

#endif
