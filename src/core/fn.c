
//
//  Functions
//

#include "fn.h"
#include "state.h"


// Define a new function on the interpreter state.
Index fn_new(HyState *state) {
	vec_inc(state->functions);
	Function *fn = &vec_last(state->functions);
	fn->name = NULL;
	fn->length = 0;
	fn->package = NOT_FOUND;
	fn->source = 0;
	fn->line = 0;
	fn->arity = 0;
	fn->frame_size = 0;
	vec_new(fn->instructions, Instruction, 64);
	return vec_len(state->functions) - 1;
}


// Free resources allocated by a function.
void fn_free(Function *fn) {
	vec_free(fn->instructions);
}


// Append a bytecode instruction to the end of the function's instruction list.
Index fn_emit(Function *fn, BytecodeOpcode opcode, uint16_t arg1, uint16_t arg2,
		uint16_t arg3) {
	vec_inc(fn->instructions);
	vec_last(fn->instructions) = ins_new(opcode, arg1, arg2, arg3);
	return vec_len(fn->instructions) - 1;
}



//
//  Natives
//

// Free resources allocated by a native function.
void native_free(NativeFunction *fn) {
	free(fn->name);
}


// Add a native function to a package. `arity` is the number of arguments the
// function accepts. If it is set to HY_VAR_ARG, then the function can accept
// any number of arguments.
void hy_add_fn(HyState *state, HyPackage pkg_index, char *name, uint32_t arity,
		HyNativeFn fn) {
	// Copy the name into a heap allocated string
	char *name_copy = malloc(strlen(name) + 1);
	strcpy(name_copy, name);

	// Create a new native function
	vec_inc(state->native_fns);
	NativeFunction *native = &vec_last(state->native_fns);
	native->name = name_copy;
	native->package = pkg_index;
	native->arity = arity;
	native->fn = fn;

	// Create a local on package with a default value
	Package *pkg = &vec_at(state->packages, pkg_index);
	Index index = vec_len(state->native_fns) - 1;
	pkg_local_add(pkg, name, strlen(name), fn_to_val(index, TAG_NATIVE));
}
