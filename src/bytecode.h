
//
//  Bytecode
//


#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdbool.h>

#include "value.h"


// The default size of the bytecode's instructions array.
#define DEFAULT_INSTRUCTIONS_CAPACITY 512


// Read a single byte from an instruction cursor.
#define READ_BYTE() \
	(ip++, (uint8_t) (*(ip - 1)))


// Read two bytes from an instruction cursor.
#define READ_2_BYTES() \
	(ip += 2, ((uint16_t) *(ip - 1) << (1 << 3)) | *(ip - 2))


// Read four bytes from an instruction cursor.
#define READ_4_BYTES()                           \
	(ip += 4,                                \
		((uint32_t) *(ip - 1) << (3 << 3)) | \
		((uint32_t) *(ip - 2) << (2 << 3)) | \
		((uint32_t) *(ip - 3) << (1 << 3)) | \
		((uint32_t) *(ip - 4)))


// Read eight bytes from an instruction cursor.
#define READ_8_BYTES()                           \
	(ip += 8,                                \
		((uint64_t) *(ip - 1) << (7 << 3)) | \
		((uint64_t) *(ip - 2) << (6 << 3)) | \
		((uint64_t) *(ip - 3) << (5 << 3)) | \
		((uint64_t) *(ip - 4) << (4 << 3)) | \
		((uint64_t) *(ip - 5) << (3 << 3)) | \
		((uint64_t) *(ip - 6) << (2 << 3)) | \
		((uint64_t) *(ip - 7) << (1 << 3)) | \
		((uint64_t) *(ip - 8)))


// All instructions that can be emitted as valid bytecode.
typedef enum {
	// Pushes a number onto the stack, passed in as a 64 bit
	// unsigned integer (through the `number_to_value` conversion
	// function in `value.h`).
	//
	// Arguments:
	// * 8 bytes - the value to push.
	CODE_PUSH_NUMBER,

	// Pushes a constant string literal onto the stack.
	//
	// Arguments:
	// * 2 bytes - the index in the compiler's string literals
	//   list, indicating which literal to push.
	CODE_PUSH_STRING,

	// Pushes true onto the stack.
	//
	// Arguments:
	// None
	CODE_PUSH_TRUE,

	// Pushes false onto the stack.
	//
	// Arguments:
	// None
	CODE_PUSH_FALSE,

	// Pushes nil onto the stack.
	//
	// Arguments:
	// None
	CODE_PUSH_NIL,

	// Push the value of a variable onto the top of the stack.
	// All local variables are found somewhere in the stack, and
	// are pushed using their index in the stack.
	//
	// The slot position specified is relative to the currently
	// executing function's call frame's stack pointer.
	//
	// Arguments:
	// * 2 bytes - the index in the stack of the variable to
	//   push.
	CODE_PUSH_LOCAL,

	// Pops an item off the top of the stack.
	//
	// Arguments:
	// None
	CODE_POP,

	// Pops the top item off the stack and stores it into stack
	// slot [slot].
	//
	// The stack slot is relative to the top call frame's stack
	// pointer.
	//
	// Arguments:
	// * 2 bytes -  the index in the value stack
	// to transfer the top of the stack into.
	CODE_STORE,

	// Unconditionally jumps the instruction pointer forward.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump forward
	//   by.
	CODE_JUMP_FORWARD,

	// Unconditionally jumps the instruction pointer backwards.
	//
	// The distinction between jumping forward and backward is
	// done because the 2 byte argument given to each instruction
	// is unsigned, meaning we can't represent negative numbers
	// easily.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump back by.
	CODE_JUMP_BACK,

	// Pops the top item from the stack and, if it's false, jumps
	// the instruction pointer forward. If the popped item is
	// true, then the program continues execution normally.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump forward by,
	//   if the value is false.
	CODE_JUMP_IF_NOT,

	// Calls a user defined function by pushing a new function
	// call frame onto the call frame stack.
	//
	// Arguments:
	// * 2 bytes - the index of the function to call in the
	//   virtual machine's functions list.
	CODE_CALL,

	// Calls a native C function by converting an 8 byte argument
	// into a C function pointer, then calling this function
	// pointer.
	//
	// Arguments:
	// * 8 bytes - the C function pointer to call.
	CODE_CALL_NATIVE,

	// Returns from a function, popping any local variables off
	// the top of the stack.
	//
	// Arguments:
	// None
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


// Returns a new bytecode object with an empty instruction
// array, with the capacity `capacity`.
Bytecode bytecode_new(int capacity);

// Free any resources allocated by the bytecode array.
void bytecode_free(Bytecode *bytecode);


// Emit `instruction` by appending it to the bytecode's
// instruction array.
//
// Returns the index of the instruction in the instruction
// array.
int emit(Bytecode *bytecode, uint8_t instruction);

// Emit a 2 byte argument.
void emit_arg_2(Bytecode *bytecode, uint16_t arg);

// Emit a 4 byte argument.
void emit_arg_4(Bytecode *bytecode, uint32_t arg);

// Emit an 8 byte argument.
void emit_arg_8(Bytecode *bytecode, uint64_t arg);


// Emits bytecode to push a number onto the top of the stack.
void emit_push_number(Bytecode *bytecode, double number);

// Emits bytecode to push a copy of a value somewhere in the
// stack onto the top of the stack.
void emit_push_local(Bytecode *bytecode, uint16_t index);


// Emit an incomplete jump instruction, where the amount to jump
// is given a dummy value of 0.
int emit_jump(Bytecode *bytecode, uint8_t instruction);

// Patch a forward jump instruction at the given index. Changes
// the address the jump instruction at `index` jumps to, to the
// most recently emitted bytecode instruction.
void patch_forward_jump(Bytecode *bytecode, int index);

// Emits a backward jump that jumps to the instruction at
// `index`.
void emit_backward_jump(Bytecode *bytecode, int index);


// Emits a call to a native function.
void emit_native(Bytecode *bytecode, void *fn);

// Emits a call to a user function.
void emit_bytecode_call(Bytecode *bytecode, uint16_t index);

#endif
