
//
//  Compiler
//


#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "value.h"
#include "lexer.h"
#include "bytecode.h"


// The maximum number of local variables that can be
// in scope at any point.
#define MAX_LOCALS 256


// A local variable.
typedef struct {
	// The name of the variable.
	char *name;

	// The length of the local's name.
	int length;

	// The scope depth the local was defined at.
	int scope_depth;
} Local;


// The compiler.
//
// Responsible for taking some Hydrogen source code and
// producing compiled bytecode.
typedef struct {
	// A reference back to the invoking virtual machine.
	VirtualMachine *vm;

	// The function we're compiling.
	Function *fn;

	// The local variables currently in scope at any point
	// during compilation.
	Local locals[MAX_LOCALS];

	// The number of local variables in scope.
	int local_count;

	// The current scope depth of the compiler.
	int scope_depth;
} Compiler;


// Compile source code into bytecode.
//
// The compiler uses the lexer in the virtual machine `vm` as
// its input.
//
// Generates the bytecode directly into `fn`'s
// bytecode array.
//
// It stops compiling when `terminator is found, or end of file
// is reached.
void compile(VirtualMachine *vm, Function *fn, TokenType terminator);


// Emits bytecode to push the local with the name `name` onto
// the stack.
void push_local(Compiler *compiler, char *name, int length);

// Emits bytecode to push a string literal onto the stack.
//
// Returns a pointer to an unallocated string, so the string
// that will be pushed can be modified.
String ** push_string(Compiler *compiler);


// Compiles a function call.
void function_call(Compiler *compiler);

#endif
