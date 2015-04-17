
//
//  Compiler
//


#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"
#include "value.h"
#include "lexer.h"


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

	// The index of the upvalue that closes over this local in
	// the virtual machine, if the local is an upvalue.
	//
	// -1 if the local is not an upvalue.
	int upvalue_index;
} Local;


// The type of a variable.
typedef enum {
	// A local variable.
	VARIABLE_LOCAL,

	// An upvalue.
	VARIABLE_UPVALUE,

	// An undefined variable.
	VARIABLE_UNDEFINED,
} VariableType;


// A structure encapsulating a variable, which could be a local
// or upvalue.
typedef struct {
	// The type of the variable.
	VariableType type;

	// The index of the local in the compiler's local list, or
	// the index of the upvalue in the virtual machine's upvalue
	// list.
	int index;

	union {
		// A pointer to the local variable stored by the
		// compiler.
		Local *local;

		// A pointer to the upvalue stored by the virtual
		// machine.
		Upvalue *upvalue;
	};
} Variable;


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
typedef struct compiler {
	// A reference back to the invoking virtual machine.
	VirtualMachine *vm;

	// A reference to the compiler invoking this compiler.
	//
	// A new compiler is used for each function, so this is just
	// a reference to the parent function.
	//
	// This is NULL for the top level compiler.
	struct compiler *parent;

	// The function we're compiling.
	Function *fn;

	// The local variables currently in scope at any point
	// during compilation.
	Local locals[MAX_LOCALS];

	// The number of local variables in scope.
	int local_count;

	// The starting index of this compiler's locals on the
	// stack. For the top-most compiler, this is 0 (as its
	// locals start at the bottom of the stack).
	//
	// This is calculated by summing the number of locals in all
	// parent compilers.
	int stack_start;

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
// Stops compiling when `terminator` is found, or end of file is
// reached.
void compile(VirtualMachine *vm, Compiler *parent, Function *fn,
	TokenType terminator);


// Returns true if the lexer matches a function call.
bool match_function_call(Lexer *lexer);

// Compiles a function call, leaving the return value of the
// function on the top of the stack.
void function_call(Compiler *compiler);

// Parses the arguments list for `fn`. Expects the lexer's cursor
// to be on the opening parenthesis of the arguments list.
// Consumes the final closing parenthesis of the arguments.
void function_definition_arguments(Compiler *compiler, Function *fn);


// Searches for a variable with the name `name`.
//
// Search order:
// * Compiler locals
// * Existing upvalues in the virtual machine
// * Parent compilers' locals
Variable capture_variable(Compiler *compiler, char *name, int length);

// Emits bytecode to push a string literal onto the stack.
//
// Returns a pointer to an unallocated string, so the string
// that will be pushed can be modified.
String ** push_string(Compiler *compiler);


// Emits bytecode to push a variable onto the stack, handling
// possible cases when the variable could be a local or upvalue.
void emit_push_variable(Bytecode *bytecode, Variable *variable);

// Returns the appropriate instruction for storing a variable
// of type `type`.
Instruction storage_instruction(VariableType type);

#endif
