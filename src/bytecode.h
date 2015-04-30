
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
	// unsigned integer (through the `number_to_value`
	// conversion function in `value.h`).
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

	// Pushes the function pointer for a native C function onto
	// the stack.
	//
	// Arguments:
	// * 8 bytes - the pointer to a native C function.
	CODE_PUSH_NATIVE,

	// Pushes a user defined function onto the stack.
	//
	// Arguments:
	// * 2 bytes - the index in the VM's functions list of the
	//   function to push.
	CODE_PUSH_FUNCTION,

	// Push the value in the upvalue onto the top of the stack.
	// If the upvalue is open, the VM indexes the stack with the
	// upvalue's absolute stack position to find the value to
	// push.
	// If the upvalue is closed, it pushes the value stored in
	// the upvalue's `value` field.
	//
	// Arguments:
	// * 2 bytes - the index of the upvalue in the VM's upvalue
	//   list.
	CODE_PUSH_UPVALUE,

	// Pops a variable off the stack, assuming it is an instance
	// of a class (triggering an error if it isn't). Then pushes
	// the value in the field named `arg1` (with length in
	// `arg2`).
	//
	// Arguments:
	// * 2 bytes - the length of the name of the field.
	// * 8 bytes - char pointer to the name of the field to
	//   push.
	CODE_PUSH_FIELD,

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
	// * 2 bytes - the index in the value stack to transfer the
	//   top of the stack into.
	CODE_STORE_LOCAL,

	// Pops an item off the top of the stack and stores it in
	// the upvalue at the index `arg` in the virtual machine's
	// upvalue list.
	//
	// Arguments:
	// * 2 bytes - the index in the VM's upvalues list to store
	//   the top of the stack to.
	CODE_STORE_UPVALUE,

	// Pops a value off the stack, saving it as the value to set
	// a field of a class to. Then pops another value off the
	// stack, assuming its an instance of a class (triggering an
	// error if it isn't). Then sets the field named `arg1`
	// (with length `arg2`) on the class to the first popped
	// value.
	//
	// Arguments:
	// * 8 bytes - char pointer to the name of the field to
	//   store to.
	// * 2 bytes - the length of the name of the field.
	CODE_STORE_FIELD,

	// Closes an upvalue by copying its value off the stack
	// (found at its absolute stack location) into the upvalue's
	// `value` field.
	//
	// Arguments:
	// * 2 bytes - the index of the upvalue to close in the VM's
	//   upvalues list.
	CODE_CLOSE_UPVALUE,

	// Unconditionally jumps the instruction pointer forward.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump forward
	//   by.
	CODE_JUMP_FORWARD,

	// Unconditionally jumps the instruction pointer backwards.
	//
	// The distinction between jumping forward and backward is
	// done because the 2 byte argument given to each
	// instruction is unsigned, meaning we can't represent
	// negative numbers easily.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump back by.
	CODE_JUMP_BACK,

	// Pops the top item from the stack and, if it's false,
	// jumps the instruction pointer forward. If the popped item
	// is true, then the program continues execution normally.
	//
	// Arguments:
	// * 2 bytes - the number of instructions to jump forward
	//   by, if the value is false.
	CODE_JUMP_IF_NOT,

	// Pops a value off the top of the stack and attempts to
	// call it as a function, triggering a runtime error if it
	// is not a function (function, closure, or native).
	//
	// Arguments:
	// * 2 bytes - the number of arguments provided to the
	//   function.
	CODE_CALL,

	// Calls a native function without having it pushed onto the
	// stack first, for use in calling binary operators.
	//
	// Arguments:
	// * 8 bytes - the C function pointer to call.
	CODE_CALL_NATIVE,

	// Creates a new instance of a class and pushes it onto the
	// stack. Doesn't call the class' constructor - that needs
	// to be done with a separate call instruction.
	//
	// Arguments:
	// * 2 bytes - the index in the VM's class definitions list
	//   of the class to create an instance of.
	CODE_INSTANTIATE_CLASS,

	// Returns from a function. Pops the return argument off the
	// top of the stack and saves it, then discards all local
	// variables, then pushes the return argument for the
	// location that called the function.
	//
	// If a function doesn't explicitly return anything, nil
	// should be pushed before emitting the return instruction.
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

// Emits bytecode to push a number onto the top of the stack.
void emit_push_number(Bytecode *bytecode, double number);

// Emits bytecode to push a native function onto the stack.
void emit_push_native(Bytecode *bytecode, int index);

// Emits bytecode to push a user function onto the stack.
void emit_push_function(Bytecode *bytecode, uint16_t index);

// Emits bytecode to push a field of the class on the top of the
// stack.
void emit_push_field(Bytecode *bytecode, char *name, int length);

// Emits a call to a function.
void emit_call(Bytecode *bytecode, int arity);

// Emits a call to a native function.
void emit_call_native(Bytecode *bytecode, void *fn);

#endif
