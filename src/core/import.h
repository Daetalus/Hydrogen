
//
//  Imports
//

#ifndef IMPORT_H
#define IMPORT_H

#include <stdbool.h>
#include <stdlib.h>

// Returns the final path to a package to import from the path to the parent
// package and the import path
char * import_pkg_path(char *parent, char *child);

// Returns true if an import path is valid
bool import_is_valid(char *path);

#endif
