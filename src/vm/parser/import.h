
//
//  Import Parsing
//

#ifndef IMPORT_H
#define IMPORT_H

#include "parser.h"

// Searches for an imported package in the parser with the given name,
// returning NULL if the package couldn't be found.
int import_package_find(Parser *parser, char *name, size_t length);

// Imports a package with the given path and name.
void import(Parser *parser, char *path);

// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser);

#endif
