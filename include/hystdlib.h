
//
//  Hydrogen Standard Library
//

#ifndef HYSTDLIB_H
#define HYSTDLIB_H

#include <hydrogen.h>

// Register the entire standard library.
void hy_add_stdlib(HyVM *vm);

// Register the IO library.
void hy_add_io(HyVM *vm);

// Register the error library.
void hy_add_err(HyVM *vm);

#endif
