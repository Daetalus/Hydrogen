
//
//  Hydrogen Standard Library
//

#ifndef HYSTDLIB_H
#define HYSTDLIB_H

#include <hydrogen.h>

// Register the entire standard library.
void hy_add_stdlib(HyVM *vm);

// Register only the IO library.
void hy_add_io(HyVM *vm);

#endif
