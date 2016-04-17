
//
//  Functions
//

#include "fn.h"
#include "vm.h"


// Defines a new function on the interpreter state
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


// Frees resources allocated by a function
void fn_free(Function *fn) {
	vec_free(fn->instructions);
}


// Appends a bytecode instruction to the end of the function's instruction list
Index fn_emit(Function *fn, BytecodeOpcode opcode, uint16_t arg1, uint16_t arg2,
		uint16_t arg3) {
	vec_inc(fn->instructions);
	vec_last(fn->instructions) = ins_new(opcode, arg1, arg2, arg3);
	return vec_len(fn->instructions) - 1;
}



//
//  Natives
//

// Defines a new native function on the package `pkg`
Index native_new(HyState *state, Index pkg_index, char *name) {
	// Create native function on interpreter state
	vec_inc(state->natives);
	NativeFunction *fn = &vec_last(state->natives);
	fn->name = name;
	fn->package = pkg_index;
	fn->arity = 0;
	fn->fn = NULL;

	// Create local on package with a default value
	Package *pkg = &vec_at(state->packages, pkg_index);
	Index index = vec_len(state->natives) - 1;
	pkg_local_add(pkg, name, strlen(name), native_to_val(index));
	return index;
}


// Frees resources allocated by a native function
void native_free(NativeFunction *fn) {
	free(fn->name);
}


// Add a native function to a package. `arity` is the number of arguments the
// function accepts. If it is set to -1, then the function can accept any number
// of arguments
void hy_add_fn(HyState *state, HyPackage pkg, char *name, int32_t arity,
		HyNativeFn fn) {
	// Copy the name into a heap allocated string
	char *name_copy = malloc(strlen(name) + 1);
	strcpy(name_copy, name);

	// Create a new native function
	Index index = native_new(state, pkg, name_copy);
	NativeFunction *native = &vec_at(state->natives, index);
	native->arity = arity;
	native->fn = fn;
}
