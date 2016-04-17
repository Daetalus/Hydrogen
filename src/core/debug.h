
//
//  Debug
//

#ifndef DEBUG_H
#define DEBUG_H

#include <hydrogen.h>
#include <assert.h>

#include <vec.h>

#include "fn.h"
#include "struct.h"
#include "ins.h"

// Set to `true` when asserts should be checked, otherwise they'll be optimised
// out by the compiler
#define DEBUG true

// Ensures a condition is true
#define ASSERT(...)          \
	if (DEBUG) {             \
		assert(__VA_ARGS__); \
	}

// Pretty prints an instruction within a function's bytecode to the standard
// output. The instruction index is used to calculate jump offsets
void debug_ins(HyState *state, Function *fn, Index ins_index);

// Pretty prints the entire bytecode of a function to the standard output
void debug_fn(HyState *state, Function *fn);

#endif
