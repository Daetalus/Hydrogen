
//
//  IO Library
//

#ifndef IO_H
#define IO_H

#include <hystdlib.h>
#include <stdio.h>

// Print a value to the given stream.
HyValue io_fprint(FILE *stream, HyVM *vm, HyArgs *args);

// Print a value to the given stream with a trailing newline.
HyValue io_fprintln(FILE *stream, HyVM *vm, HyArgs *args);

#endif
