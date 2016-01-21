
//
//  Hydrogen Standard Library
//

#ifndef HYSTDLIB_H
#define HYSTDLIB_H

#include <hydrogen.h>

// Register the entire standard library.
void hy_add_stdlib(HyState *state);

// Register the IO library.
void hy_add_io(HyState *state);

// Register the error library.
void hy_add_err(HyState *state);

#endif
