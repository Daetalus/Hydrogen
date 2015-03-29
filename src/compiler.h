
//
//  Compiler
//


#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "bytecode.h"
#include "lexer.h"


// The maximum number of local variables that can be
// in scope at any point.
//
// The physical maximum value for this is 65535, which
// is the maximum number an unsigned 16 bit integer can
// hold. We set the maximum to less than this though to
// save on space.
#define MAX_LOCALS 256


// A local variable.
//
// This is used only during compilation. When we encounter
// a new local variable, it's pushed onto the compiler's
// locals stack. When the scope depth decreases, we loop
// over all the locals and discard the ones that are at
// that scope depth or deeper.
typedef struct {
	// The name of this local. This is a pointer that
	// points directly into the source string.
	char *name;

	// The length of the name of this local variable.
	int length;

	// The scope depth this local was defined at.
	int scope_depth;
} Local;


// The compiler.
//
// Responsible for taking some Hydrogen source code and
// producing compiled bytecode.
//
// The source code is given to the compiler in the form of
// a lexer. The lexer produces a sequence of tokens from
// some source code. We use this sequence of tokens as our
// input.
//
// The compiler is invoked by the virtual machine when
// the `vm_compile` function is called. Compiled bytecode
// is given back to the virtual machine by generating a
// function struct (see below), and calling the
// `vm_define_bytecode_function` function on the virtual
// machine.
//
// When another function definition is encountered by the
// compiler, another compiler is created recursively, which
// is used to compile the function into bytecode, which is
// then given back to the same virtual machine.
//
// The top level of the file, when we're not in a function,
// is treated as if it were a "main" function, and is the
// first function defined by the virtual machine.
typedef struct {
	// A reference back to the invoking virtual machine.
	VirtualMachine *vm;

	// The function we're compiling.
	Function *fn;

	// The local variables currently in scope at any point
	// during the compilation.
	Local locals[MAX_LOCALS];

	// The number of local variables in scope.
	int local_count;

	// The current scope depth of the compiler.
	int scope_depth;

	// Set to true when the compiler encounters a syntax
	// or compilation error.
	bool has_error;
} Compiler;


// Compile source code into bytecode.
//
// The compiler uses the lexer in the given virtual
// machine as its input.
//
// The compiler generates the bytecode output directly
// into the given function's bytecode array.
//
// It stops compiling when the given terminator token
// is found, or end of file is reached.
void compile(VirtualMachine *vm, Function *fn, TokenType terminator);

// Emits bytecode to push the local with the given name onto the
// stack.
void push_local(Compiler *compiler, char *name, int length);

// Emits bytecode to push a number onto the stack.
void push_number(Compiler *compiler, double number);

// Emits bytecode to push a string literal onto the stack.
void push_string(Compiler *compiler, char *location, int length);

// Triggers the given error on the compiler.
void error(Compiler *compiler, char *fmt, ...);


// Emits bytecode to call the function with the given name.
void emit_function_call(Compiler *compiler, char *name, int length);

// Emits bytecode to call the native function for the given
// operator.
// Assumes the arguments to the call are on the stack already.
void emit_native_operator_call(Compiler *compiler, TokenType operator);

#endif
