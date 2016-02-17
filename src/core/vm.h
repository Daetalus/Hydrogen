
//
//  Virtual Machine
//

#ifndef VM_H
#define VM_H

#include <hydrogen.h>
#include <setjmp.h>

#include "vec.h"
#include "pkg.h"
#include "fn.h"
#include "struct.h"
#include "parser.h"
#include "value.h"


// Information stored about a function's caller when a function call is
// triggered.
typedef struct {
	// A pointer to the calling function being executed in this frame.
	Function *fn;

	// The start of the calling function's locals on the stack (absolute stack
	// position).
	uint32_t stack_start;

	// The absolute position on the stack where the called function's return
	// value should be stored.
	uint32_t return_slot;

	// The saved instruction pointer for the calling function, pointing to the
	// call instruction used to execute the called function.
	Instruction *ip;
} Frame;


// The interpreter state, used to execute Hydrogen source code. Variables,
// functions, etc. are preserved by the state across calls to `hy_run`.
struct hy_state {
	// We store all functions, native functions, struct definitions, and
	// upvalues in the interpreter state rather than in their respective
	// packages in order to simplify the bytecode (we don't have to specify
	// a package index in each instruction). The cost is that we can only define
	// 2^16 functions/structs/etc across all packages, rather than per package.
	Vec(Package) packages;
	Vec(Function) functions;
	Vec(NativeFunction) natives;
	Vec(StructDefinition) structs;

	// We can't store 64 bit values like numbers (doubles) and strings
	// (pointers) directly in the bytecode (because each argument is only 16
	// bits), so we use an index into these arrays instead.
	//
	// The constants array holds all number literals and values defined using
	// `const`. Struct fields are stored as the hash of the field name.
	Vec(HyValue) constants;
	Vec(String *) strings;
	Vec(Identifier) fields;

	// The interpreter's runtime stack, used to store variables.
	HyValue *stack;
	Frame *call_stack;
	uint32_t call_stack_count;

	// We use longjmp/setjmp for errors, which requires a jump buffer, which we
	// store in the state (instead of as a global variable).
	jmp_buf error_jmp;

	// This is set to a heap allocated error object before longjmp is called, so
	// we can return it to the user calling the API function.
	HyError *error;
};


// Adds a constant to the interpreter state, returning its index.
Index state_add_constant(HyState *state, HyValue constant);

// Creates a new string constant that is `length` bytes long.
Index state_add_string(HyState *state, uint32_t length);

// Adds a field name to the interpreter state's fields list. If a field matching
// `ident` already exists, then it returns the index of the existing field.
Index state_add_field(HyState *state, Identifier ident);

// Executes a function on the interpreter state.
HyError * vm_run_fn(HyState *state, Index fn);

// Resets an interpreter state's error, returning the current error.
HyError * vm_reset_error(HyState *state);

#endif
