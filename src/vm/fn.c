
//
//  Functions
//

#include "fn.h"
#include "vm.h"



//
//  Functions
//

// Defines a new function on the interpreter state.
Index fn_new(HyState *state) {
	vec_add(state->functions);
	Function *fn = &vec_last(state->functions);
	fn->name = NULL;
	fn->length = 0;
	fn->package = NOT_FOUND;
	fn->arity = 0;
	fn->frame_size = 0;
	vec_new(fn->instructions, Instruction, 64);
	return vec_len(state->functions) - 1;
}


// Frees resources allocated by a function.
void fn_free(Function *fn) {
	vec_free(fn->instructions);
}


// Appends a bytecode instruction to the end of the function's instruction list.
Index fn_emit(Function *fn, BytecodeOpcode opcode, uint16_t arg1, uint16_t arg2,
		uint16_t arg3) {
	vec_add(fn->instructions);
	vec_last(fn->instructions) = ins_new(opcode, arg1, arg2, arg3);
	return vec_len(fn->instructions) - 1;
}



//
//  Natives
//

// Defines a new native function on the package `pkg`.
Index native_new(HyState *state, Index pkg, char *name) {
	vec_add(state->natives);
	NativeFunction *fn = &vec_last(state->natives);
	fn->name = name;
	fn->package = pkg;
	fn->arity = 0;
	fn->fn = NULL;
	return vec_len(state->natives) - 1;
}


// Frees resources allocated by a native function.
void native_free(NativeFunction *fn) {
	free(fn->name);
}
