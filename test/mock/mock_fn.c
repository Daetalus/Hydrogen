
//
//  Mock Function
//

#include "mock_fn.h"

#include <bytecode.h>


// Create a new mock function.
Function mock_fn(uint32_t count, uint16_t *bytecode) {
	Function fn;
	fn.name = NULL;
	fn.length = 0;
	fn.package = 0;
	fn.source = 0;
	fn.line = 0;
	fn.arity = 0;
	fn.frame_size = 0;

	// Copy across the bytecode instruction
	vec_new(fn.instructions, Instruction, count / 4);
	vec_len(fn.instructions) = count / 4;
	for (uint32_t i = 0; i < count; i += 4) {
		Instruction ins = ins_new(bytecode[i], bytecode[i + 1], bytecode[i + 2],
			bytecode[i + 3]);
		vec_at(fn.instructions, i / 4) = ins;
	}

	return fn;
}


// Free a mock function.
void mock_fn_free(Function *fn) {
	vec_free(fn->instructions);
}
