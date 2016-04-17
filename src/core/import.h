
//
//  Imports
//

#ifndef IMPORT_H
#define IMPORT_H

#include <stdbool.h>
#include <stdlib.h>

// Return the file path to a package that we want to import from the path to
// the parent package and the path given in the import statement.
char * import_pkg_path(char *parent, char *child);

// Return true if an import path (given in an import statement) is valid.
bool import_is_valid(char *path);

#endif
