
//
//  Import Parsing
//

#ifndef IMPORT_H
#define IMPORT_H

#include "parser.h"

// Imports a package with the given path and name.
void import(Parser *parser, char *path);

// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser);

#endif
