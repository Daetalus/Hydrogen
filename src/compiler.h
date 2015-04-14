
//
//  Compiler
//


#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "value.h"
#include "lexer.h"
#include "bytecode.h"


// The maximum number of local variables that can be in scope at
// any point.
#define MAX_LOCALS 256

// The maximum number of loops that can exist inside each other
// at any point in the code.
#define MAX_LOOP_DEPTH 256

// The maximum number of break statements inside one loop.
#define MAX_BREAK_STATEMENTS 256


// A local variable.
typedef struct {
	// The name of the variable.
	char *name;

	// The length of the local's name.
	int length;

	// The scope depth the local was defined at.
	int scope_depth;
} Local;


// A loop.
typedef struct {
	// A list of jump statements that need to be patched to the
	// ending position of this loop. Each time a break statement
	// is encountered, an incomplete jump instruction is emitted
	// and added to this list for patching once the ending
	// position of the loop is known.
	int break_statements[MAX_BREAK_STATEMENTS];

	// The number of break statements.
	int break_statement_count;

	// The scope depth this loop was created at, for emitting
	// pop instructions up to this depth upon encountering a
	// break statement.
	int scope_depth;
} Loop;


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

	// A stack representing the number of encasing loops at any
	// point during the code. Used to return from break
	// statements.
	//
	// Since we're using a recursive algorithm, loops can be
	// created on the stack (rather than the heap). We use
	// pointers to these loops on the stack, instead of copying
	// then by value.
	Loop *loops[MAX_LOOP_DEPTH];

	// The number of loops in the loop stack.
	int loop_count;
} Compiler;


// Compile source code into bytecode, using the lexer in the
// virtual machine `vm` as input. Outputs bytecode directly into
// `fn`'s bytecode array.
//
// Stops compiling when `terminator is found, or end of file is
// reached.
void compile(VirtualMachine *vm, Function *fn, TokenType terminator);


// Emits bytecode to push the local with the name `name` onto
// the stack.
void push_local(Compiler *compiler, char *name, int length);

// Emits bytecode to push a string literal onto the stack.
//
// Returns a pointer to an unallocated string, so the string
// that will be pushed can be modified.
String ** push_string(Compiler *compiler);

#endif
