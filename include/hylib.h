
//
//  Hydrogen Standard Library
//

#ifndef HYLIB_H
#define HYLIB_H

#include <hydrogen.h>

// Register the entire standard library.
void hy_add_libs(HyState *state);

// Register the IO library.
void hy_add_io(HyState *state);

#endif
