
//
//  Functions
//

#ifndef FN_H
#define FN_H

#include <hydrogen.h>
#include <vec.h>

#include "bytecode.h"
#include "ins.h"


// A function is a collection of bytecode instructions that can be executed by
// another bytecode instruction. They are defined in Hydrogen source code.
typedef struct {
	// The name of the function, used for error messages and resolving
	// identifiers during parsing.
	char *name;
	uint32_t length;

	// The index of the package and source code file the function was defined
	// in.
	Index package;
	Index source;

	// The line on which the function was defined.
	uint32_t line;

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
	// compilation. Heap allocated, will be freed.
	char *name;

	// The index of the package this native function is defined in.
	Index package;

	// The number of arguments accepted by the function. If it is set to -1,
	// then this function accepts any number of arguments.
	int32_t arity;

	// The C function pointer.
	HyNativeFn fn;
} NativeFunction;


// Arguments passed to a native function.
struct hy_args {
	// A pointer to the start of the stack.
	HyValue *stack;

	// The first argument passed to the function.
	uint32_t start;

	// The number of arguments passed to the function.
	uint32_t arity;
};


// Defines a new native function on the package `pkg`.
Index native_new(HyState *state, Index pkg, char *name);

// Frees resources allocated by a native function.
void native_free(NativeFunction *fn);

#endif