
//
//  Bytecode
//


#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdbool.h>


// All instructions that can be emitted as valid bytecode.
typedef enum {
	// Pushes a number value onto the stack.
	//
	// Takes 1, 8 byte argument - the value to push.
	CODE_PUSH_NUMBER,

	// Pushes a constant string literal onto the stack.
	// The literal's value is extracted from the source
	// and allocated as a new object on the heap.
	//
	// Takes 1, 2 byte argument: the index in the compiler's
	// string constants list of the literal to push.
	CODE_PUSH_STRING,

	// Push the value of a variable in the value stack
	// slot [slot] onto the stack.
	//
	// The slot position is relative to the top call
	// frame's stack pointer.
	//
	// Takes 1, 2 byte argument - the index in the value
	// stack of the variable to push.
	CODE_PUSH_VARIABLE,

	// Pops the top item off the value stack.
	CODE_POP,

	// Pops the top item off the stack and stores it into stack
	// slot [slot].
	//
	// The stack slot is relative to the top call frame's stack
	// pointer.
	//
	// Takes 1, 2 byte argument - the index in the value stack
	// to transfer the top of the stack into.
	CODE_STORE,

	// Unconditionally jumps the instruction pointer [argument]
	// number of instructions forward.
	//
	// Takes 1, 2 byte argument - the number of instructions to
	// jump forward.
	CODE_JUMP_FORWARD,

	// Unconditionally jumps the instruction pointer [argument]
	// number of instructions backwards.
	//
	// The distinction is made between jumping the instruction
	// pointer forward and backward because arguments to
	// instructions are unsigned.
	//
	// Takes 1, 2 byte argument - the number of instructions to
	// jump back.
	CODE_JUMP_BACKWARD,

	// Pops the top item from the stack and, if it is false,
	// jumps the instruction pointer forward [argument]
	// number of instructions. If it's true, then just
	// continues execution normally.
	//
	// Takes 1, 2 byte argument - the number of instructions
	// to jump forward if the value is false.
	CODE_CONDITIONAL_JUMP,

	// Calls the function at [index] in the virtual machine's
	// function list, by pushing a new call frame onto the call
	// frame stack.
	//
	// Takes 1, 2 byte argument - the index of the function to
	// call in the virtual machine's function list.
	CODE_CALL,

	// Calls a native C function by converting an 8 byte
	// argument into a C function pointer, then calling
	// the function pointer.
	//
	// Takes 1, 8 byte argument - the function pointer of
	// the C function to call.
	CODE_CALL_NATIVE,

	// Emitted to return from a function.
	//
	// Takes no arguments.
	CODE_RETURN,
} Instruction;


// The compiled bytecode for a function.
typedef struct {
	// The bytecode array.
	uint8_t *instructions;

	// The number of instructions in the bytecode array.
	int count;

	// The capacity of the bytecode array (total amount of
	// memory allocated for the array).
	int capacity;
} Bytecode;


// Create a new empty bytecode array with the given
// initial capacity.
void bytecode_new(Bytecode *bytecode, int initial_capacity);

// Free a bytecode array.
void bytecode_free(Bytecode *bytecode);

// Emit a bytecode instruction. Returns the index of the
// bytecode instruction in the bytecode array.
int emit(Bytecode *bytecode, uint8_t instruction);

// Emit a 1 byte argument.
void emit_arg_1(Bytecode *bytecode, uint8_t arg);

// Emit a 2 byte argument.
void emit_arg_2(Bytecode *bytecode, uint16_t arg);

// Emit a 4 byte argument.
void emit_arg_4(Bytecode *bytecode, uint32_t arg);

// Emit an 8 byte argument.
void emit_arg_8(Bytecode *bytecode, uint64_t arg);

// Emit an incomplete jump instruction.
int emit_jump(Bytecode *bytecode, uint8_t instruction);

// Patch a jump instruction at the given index. Will
// jump to the instruction after the most recent
// instruction emitted.
void patch_jump(Bytecode *bytecode, int index);

#endif
