
//
//  Bytecode
//


#include <stdlib.h>
#include <stdio.h>

#include "value.h"
#include "bytecode.h"


// Returns a new bytecode object with an empty instruction
// array, with the capacity `capacity`.
Bytecode bytecode_new(int capacity) {
	Bytecode bytecode;
	bytecode.instructions = malloc(capacity * sizeof(uint8_t));
	bytecode.count = 0;
	bytecode.capacity = capacity;
	return bytecode;
}


// Free any resources allocated by the bytecode array.
void bytecode_free(Bytecode *bytecode) {
	free(bytecode->instructions);
}



//
//  Instruction Emission
//

// Increases the bytecode's instruction array size by `amount`,
// potentially resizing the array and modifying the capacity.
//
// Returns the index in the instructions array where new data
// should be added at.
int resize(Bytecode *bytecode, int amount) {
	bytecode->count += amount;

	if (bytecode->count > bytecode->capacity) {
		// Reisze the array
		bytecode->capacity *= 2;
		if (bytecode->capacity < bytecode->count) {
			bytecode->capacity = bytecode->count * 2;
		}

		int new_size = bytecode->capacity * sizeof(uint8_t);
		bytecode->instructions = realloc(bytecode->instructions, new_size);
	}

	return bytecode->count - amount;
}


// Emit `instruction` by appending it to the bytecode's
// instruction array.
//
// Returns the index of the instruction in the instruction
// array.
int emit(Bytecode *bytecode, uint8_t instruction) {
	int index = resize(bytecode, 1);
	bytecode->instructions[index] = instruction;
	return index;
}


// Emit a 2 byte argument.
void emit_arg_2(Bytecode *bytecode, uint16_t arg) {
	int index = resize(bytecode, 2);
	bytecode->instructions[index] = arg & 0xff;
	bytecode->instructions[index + 1] = (arg >> (1 << 3)) & 0xff;
}


// Emit a 4 byte argument.
void emit_arg_4(Bytecode *bytecode, uint32_t arg) {
	int index = resize(bytecode, 4);
	bytecode->instructions[index] = arg & 0xff;
	bytecode->instructions[index + 1] = (arg >> (1 << 3)) & 0xff;
	bytecode->instructions[index + 2] = (arg >> (2 << 3)) & 0xff;
	bytecode->instructions[index + 3] = (arg >> (3 << 3)) & 0xff;
}


// Emit an 8 byte argument.
void emit_arg_8(Bytecode *bytecode, uint64_t arg) {
	int index = resize(bytecode, 8);
	bytecode->instructions[index] = arg & 0xff;
	bytecode->instructions[index + 1] = (arg >> (1 << 3)) & 0xff;
	bytecode->instructions[index + 2] = (arg >> (2 << 3)) & 0xff;
	bytecode->instructions[index + 3] = (arg >> (3 << 3)) & 0xff;
	bytecode->instructions[index + 4] = (arg >> (4 << 3)) & 0xff;
	bytecode->instructions[index + 5] = (arg >> (5 << 3)) & 0xff;
	bytecode->instructions[index + 6] = (arg >> (6 << 3)) & 0xff;
	bytecode->instructions[index + 7] = (arg >> (7 << 3)) & 0xff;
}


// Emits bytecode to push a number onto the top of the stack.
void emit_push_number(Bytecode *bytecode, double number) {
	emit(bytecode, CODE_PUSH_NUMBER);
	emit_arg_8(bytecode, number_to_value(number));
}


// Emits bytecode to push a copy of a value somewhere in the
// stack onto the top of the stack.
void emit_push_local(Bytecode *bytecode, uint16_t index) {
	emit(bytecode, CODE_PUSH_LOCAL);
	emit_arg_2(bytecode, index);
}



//
//  Jumps
//

// Emit an incomplete jump instruction, where the amount to jump
// is given a dummy value of 0.
int emit_jump(Bytecode *bytecode, uint8_t instruction) {
	int index = emit(bytecode, instruction);
	emit_arg_2(bytecode, 0);
	return index;
}


// Patch a forward jump instruction at the given index. Changes
// the address the jump instruction at `index` jumps to, to the
// most recently emitted bytecode instruction.
void patch_forward_jump(Bytecode *bytecode, int index) {
	// Subtract 3 to account for the jump statement itself.
	uint16_t amount = bytecode->count - index - 3;

	// Add one to the index initially to skip the actual jump
	// instruction and start at the 2 byte argument.
	bytecode->instructions[index + 1] = amount & 0xff;
	bytecode->instructions[index + 2] = (amount >> (1 << 3)) & 0xff;
}


// Emits a backward jump that jumps to the instruction at
// `index`.
void emit_backward_jump(Bytecode *bytecode, int index) {
	emit(bytecode, CODE_JUMP_BACK);

	// Add 2 to account for the jump statement itself.
	emit_arg_2(bytecode, bytecode->count - index + 2);
}



//
//  Function Calls
//

// Emits a call to a native function.
void emit_native(Bytecode *bytecode, void *fn) {
	emit(bytecode, CODE_CALL_NATIVE);
	emit_arg_8(bytecode, ptr_to_value(fn));
}


// Emits a call to a user function.
void emit_bytecode_call(Bytecode *bytecode, uint16_t index) {
	emit(bytecode, CODE_CALL);
	emit_arg_2(bytecode, index);
}
