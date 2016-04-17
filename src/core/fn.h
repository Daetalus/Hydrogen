
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
// the interpreter.
typedef struct {
	// The name of the function.
	char *name;
	uint32_t length;

	// The package, source code file, and line where the function was defined.
	Index package;
	Index source;
	uint32_t line;

	// The number of arguments this function accepts. This is recorded so we can
	// verify it against the number of arguments passed to the function when it
	// is called at runtime, triggering an error if the two values aren't equal.
	uint32_t arity;

	// The array of the function's bytecode instructions.
	Vec(Instruction) instructions;
} Function;


// Define a new function on the interpreter state.
Index fn_new(HyState *state);

// Free resources allocated by a function.
void fn_free(Function *fn);

// Append a bytecode instruction to the end of the function's instruction list.
Index fn_emit(Function *fn, BytecodeOpcode opcode, uint16_t arg1, uint16_t arg2,
	uint16_t arg3);


// A native function is a wrapper around a C function pointer, which allows
// Hydrogen code to call native C code.
typedef struct {
	// The name of the native function (heap allocated).
	char *name;

	// The package the function is defined in.
	Index package;

	// The number of arguments the function accepts. If it is set to
	// HY_VAR_ARG, then this function accepts any number of arguments.
	uint32_t arity;

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


// Define a new native function on the package `pkg`.
Index native_new(HyState *state, Index pkg, char *name);

// Free resources allocated by a native function.
void native_free(NativeFunction *fn);

#endif
