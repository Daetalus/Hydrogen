
//
//  Debug
//

#include "debug.h"


// Pretty prints an instruction within a function's bytecode to the standard
// output. The instruction index is used to calculate jump offsets.
void debug_ins(Function *fn, Index ins_index) {
	// TODO
}


// Pretty prints the entire bytecode of a function to the standard output.
void debug_fn(Function *fn) {
	// TODO
}


// Read source code from a file and compile it into bytecode, printing it to
// the standard output.
HyError * hy_print_bytecode_file(HyState *state, HyPackage pkg, char *path) {
	return NULL;
}


// Compile source code into bytecode and print it to the standard output. An
// error object is returned if one occurred during compilation, otherwise NULL
// is returned.
HyError * hy_print_bytecode_string(HyState *state, HyPackage pkg,
		char *source) {
	return NULL;
}
