
//
//  Virtual Machine
//


#ifndef VM_H
#define VM_H

#include <stdlib.h>

#include "lexer.h"
#include "bytecode.h"
#include "value.h"


// The maximum number of functions a program can define.
#define MAX_FUNCTIONS 1024

// The maximum number of arguments that can be passed to a
// function.
#define MAX_ARGUMENTS 32

// The maximum number of constant string literals that can exist
// in a program.
#define MAX_STRING_LITERALS 1024

// The maximum number of upvalues that can be in scope at any
// point.
#define MAX_UPVALUES 512


// The definition for a native function (a function that calls
// into C from Hydrogen code).
typedef void (*NativeFunction)(uint64_t *stack, int *stack_size);


// A struct storing a string with an associated length, rather
// than terminated with a NULL byte.
typedef struct {
	// The length of the string in the source code.
	int length;

	// The pointer into the source code specifying the start of
	// the string.
	char *location;
} SourceString;


// An upvalue captured by a closure. An upvalue is a local from
// outside a function's scope used inside the function. This is
// special because when a function call finishes, its locals are
// destroyed. If a closure is still using one of its locals,
// we'll get a bunch of segmentation faults.
//
// Upvalues have 2 states, open and closed. Open upvalues are
// where the original local they close over is still in scope,
// and modification should modify that local. Upvalues are closed
// when their original variable is destroyed. When this happens,
// the virtual machine copies out the value and puts it into
// the `value` field to allow it to persist.
typedef struct {
	// True if the upvalue is closed.
	bool closed;

	// The number of closures that use this upvalue. Every time
	// a new closure references this upvalue, the value will be
	// incremented. Every time a closure referencing this upvalue
	// goes out of scope, the value will be decremented.
	//
	// This is used by the garbage collector to determine which
	// upvalues are not in use by any closures, so it can
	// deallocate ones with a reference count of 0.
	int reference_count;

	// The absolute position on the stack of this upvalue's
	// value, used when the upvalue is open.
	int stack_position;

	// The value of this upvalue when it is closed.
	uint64_t value;

	// The name of the upvalue, used for comparison against
	// identifiers to check that we haven't already created an
	// upvalue for a local. This is set to NULL when the upvalue
	// is closed, in order to avoid collisions against future
	// upvalues with the same name.
	char *name;

	// The length of the name, as the name is a pointer into the
	// source code.
	int length;
} Upvalue;


// A user-defined function generated by the compiler.
typedef struct {
	// A pointer to the start of the function's name in the
	// source code.
	//
	// NULL if the function is anonymous.
	char *name;

	// The length of the function's name in the source code.
	//
	// 0 if the function is anonymous.
	int length;

	// True if the function is the main function (ie. is top
	// level code, and isn't actually contained in a function
	// definition).
	bool is_main;

	// The function's compiled bytecode.
	Bytecode bytecode;

	// The names of the arguments passed to the function, used
	// when loading the arguments as locals during compilation
	// so we don't trigger undefined variable errors when they're
	// used.
	SourceString arguments[MAX_ARGUMENTS];

	// The number of arguments passed to the function.
	int arity;

	// Pointers to all the upvalues captured by this function.
	Upvalue *upvalues[MAX_UPVALUES];

	// The number of upvalues captured by the function.
	int upvalue_count;
} Function;


// Executes compiled bytecode.
typedef struct {
	// A lexer, producing a stream of tokens from the source
	// code.
	Lexer lexer;

	// An array of functions defined during compilation.
	//
	// The main function (for all code outside of function
	// definitions) will be the first function in this array.
	Function functions[MAX_FUNCTIONS];

	// The number of functions defined.
	int function_count;

	// An array of string literal constants encountered in the
	// source code.
	String *literals[MAX_STRING_LITERALS];

	// The number of string literals in the literals array.
	int literal_count;

	// An array of all upvalues in use by all closures.
	Upvalue upvalues[MAX_UPVALUES];

	// The number of upvalues in the upvalues array.
	int upvalue_count;
} VirtualMachine;


// Create a new virtual machine with `source` as the program's
// source code.
//
// Nothing is compiled or run until `vm_compile` and `vm_run`
// are called.
VirtualMachine vm_new(char *source);

// Free any resources allocated by the VM.
void vm_free(VirtualMachine *vm);

// Compiles the source code into bytecode.
void vm_compile(VirtualMachine *vm);

// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm);


// Defines a new function on the virtual machine, returning a
// pointer to it and its index in the virtual machine's function
// list.
//
// Performs no allocation, so the returned function's bytecode
// array still needs to be allocated.
int vm_new_function(VirtualMachine *vm, Function **fn);

// Returns the index of a user-defined function named `name`.
//
// Returns -1 if no function with that name is found.
int vm_find_function(VirtualMachine *vm, char *name, int length, int arity);

// Returns a function pointer to a library function named `name`.
//
// Returns NULL no function with that name is found.
NativeFunction vm_find_native_function(VirtualMachine *vm, char *name,
	int length, int arity);


// Create a new upvalue, returning a pointer to it and its index
// in the upvalues list.
int vm_new_upvalue(VirtualMachine *vm, Upvalue **upvalue);

#endif
