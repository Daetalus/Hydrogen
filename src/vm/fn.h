
//
//  Functions
//

#ifndef FN_H
#define FN_H

#include <hydrogen.h>

#include "vec.h"


// A function is a collection of bytecode instructions that can be executed by
// another bytecode instruction. They are defined in Hydrogen source code.
typedef struct {
	// The name of the function, used for error messages and resolving
	// identifiers during compilation.
	char *name;
	uint32_t length;

	// The index of the package the function was defined in.
	// TODO: Is this field actually used anywhere??
	Index package;

	// The number of arguments this function accepts. This is recorded so we can
	// verify it against the number of arguments passed to the function when it
	// is called at runtime, triggering an error if the two values aren't equal.
	uint32_t arity;

	// The maximum number of local variables this function allocates on the
	// stack when it's executing. This is used by the garbage collector when
	// deciding how much of the stack to iterate over when marking GC roots.
	uint32_t frame_size;

	// An array of bytecode instructions. This is the actual contents of the
	// function.
	Vec(Instruction) instructions;

	// When an upvalue is open, we need to know its absolute location on the
	// stack. That depends on the stack starting location of the function
	// that defines it when the function is called. Therefore we need to keep
	// track of which upvalues are defined by each function.
	Upvalue *upvalues;
} Function;


// Defines a new function on the interpreter state.
Index fn_new(HyState *state);

// Frees resources allocated by a function.
void fn_free(Function *fn);

// Appends a bytecode instruction to the end of the function's instruction list.
Index fn_emit(Function *fn, BytecodeOpcode opcode, uint16_t arg1, uint16_t arg2,
	uint16_t arg3);


// A native function is a wrapper around a C function pointer, which allows
// Hydrogen code to interact with native C code.
typedef struct {
	// The name of the native function, used when resolving identifiers during
	// compilation.
	char *name;

	// The index of the package this native function is defined in.
	Index package;

	// The number of arguments accepted by the function. If it is set to -1,
	// then this function accepts any number of arguments.
	int32_t arity;

	// The C function pointer.
	HyNativeFn fn;
} NativeFunction;


// Defines a new native function on the interpreter state.
Index native_new(HyState *state, char *name);

// Frees resources allocated by a native function.
void native_free(NativeFunction *fn);

#endif
