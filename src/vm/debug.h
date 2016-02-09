
//
//  Debug
//

#ifndef DEBUG_H
#define DEBUG_H

#include <hydrogen.h>
#include <assert.h>

#include "fn.h"
#include "struct.h"
#include "ins.h"
#include "vec.h"


// Set to 1 when asserts should be triggered.
#define DEBUG 1

// Ensures a condition is true.
#define ASSERT(...)          \
	if (DEBUG) {             \
		assert(__VA_ARGS__); \
	}


// Pretty prints an instruction within a function's bytecode to the standard
// output. The instruction index is used to calculate jump offsets.
void debug_ins(HyState *state, Function *fn, Index ins_index);

// Pretty prints the entire bytecode of a function to the standard output.
void debug_fn(HyState *state, Function *fn);

// Pretty prints a struct definition to the standard output.
void debug_struct(HyState *state, StructDefinition *def);

#endif
