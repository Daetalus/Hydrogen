
//
//  Debug
//

#ifndef DEBUG_H
#define DEBUG_H

#include <hydrogen.h>

#include "fn.h"
#include "ins.h"
#include "vec.h"

// Pretty prints an instruction within a function's bytecode to the standard
// output. The instruction index is used to calculate jump offsets.
void debug_ins(Function *fn, Index ins_index);

// Pretty prints the entire bytecode of a function to the standard output.
void debug_fn(Function *fn);

#endif
