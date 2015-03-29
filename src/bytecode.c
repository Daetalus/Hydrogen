
//
//  Bytecode
//


#include <stdlib.h>

#include "bytecode.h"


// Create a new empty bytecode array with the given
// initial capacity.
void bytecode_new(Bytecode *bytecode, int initial_capacity) {
	bytecode->instructions = malloc(initial_capacity * sizeof(uint8_t));
	bytecode->count = 0;
	bytecode->capacity = initial_capacity;
	bytecode->should_record = true;
}


// Free a bytecode array.
void bytecode_free(Bytecode *bytecode) {
	free(bytecode->instructions);
}


// Increases the bytecode's instruction array size by the given
// amount, potentially resizing the array and modifying the
// capacity.
//
// Returns the index in the instruction array where the new data
// should be populated from.
int resize(Bytecode *bytecode, int amount) {
	bytecode->count += amount;

	if (bytecode->count > bytecode->capacity) {
		// Reisze the array
		bytecode->capacity *= 1.5;
		size_t new_size = bytecode->capacity * sizeof(uint8_t);
		bytecode->instructions = realloc(bytecode->instructions, new_size);
	}

	return bytecode->count - amount + 1;
}


// Emit a bytecode instruction. Returns the index of the
// bytecode instruction in the bytecode array.
int emit(Bytecode *bytecode, uint8_t instruction) {
	if (!bytecode->should_record) {
		return -1;
	}

	int index = resize(bytecode, 1);
	bytecode->instructions[index] = instruction;
	return index;
}


// Emit a 1 byte argument.
void emit_arg_1(Bytecode *bytecode, uint8_t arg) {
	if (!bytecode->should_record) {
		return;
	}

	int index = resize(bytecode, 1);
	bytecode->instructions[index] = arg;
}


// Emit a 2 byte argument.
void emit_arg_2(Bytecode *bytecode, uint16_t arg) {
	if (!bytecode->should_record) {
		return;
	}

	int index = resize(bytecode, 2);
	bytecode->instructions[index] = arg & 0xff;
	bytecode->instructions[index + 1] = (arg >> (1 << 3)) & 0xff;
}


// Emit a 4 byte argument.
void emit_arg_4(Bytecode *bytecode, uint32_t arg) {
	if (!bytecode->should_record) {
		return;
	}

	int index = resize(bytecode, 4);
	bytecode->instructions[index] = arg & 0xff;
	bytecode->instructions[index + 1] = (arg >> (1 << 3)) & 0xff;
	bytecode->instructions[index + 2] = (arg >> (2 << 3)) & 0xff;
	bytecode->instructions[index + 3] = (arg >> (3 << 3)) & 0xff;
}


// Emit an 8 byte argument.
void emit_arg_8(Bytecode *bytecode, uint64_t arg) {
	if (!bytecode->should_record) {
		return;
	}

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


// Emit an incomplete jump instruction.
int emit_jump(Bytecode *bytecode, uint8_t instruction) {
	if (!bytecode->should_record) {
		return -1;
	}

	int index = emit(bytecode, CODE_CONDITIONAL_JUMP);
	emit_arg_2(bytecode, 0);
	return index;
}


// Patch a jump instruction at the given index. Will
// jump to the instruction after the most recent
// instruction emitted.
void patch_jump(Bytecode *bytecode, int index) {
	if (!bytecode->should_record) {
		return;
	}

	uint16_t amount = bytecode->count - index;

	// Add one to the index initially to skip the actual
	// instruction and start at the 2 byte argument.
	bytecode->instructions[index + 1] = amount & 0xff;
	bytecode->instructions[index + 2] = (amount >> (1 << 3)) & 0xff;
}
