
//
//  Bytecode
//

#include "bytecode.h"
#include "util.h"


// Creates an instruction from an opcode and 3 arguments. Sets the 0th argument
// to 0.
uint64_t ins_new(Opcode opcode, uint16_t arg1, uint16_t arg2, uint16_t arg3) {
	return ((uint64_t) opcode) | (((uint64_t) arg1) << 16) |
		(((uint64_t) arg2) << 32) | (((uint64_t) arg3) << 48);
}


// Creates an instruction from an opcode and 4 arguments.
uint64_t ins_full(Opcode opcode, uint8_t arg0, uint16_t arg1, uint16_t arg2,
		uint16_t arg3) {
	return ((uint64_t) opcode) | (((uint64_t) arg0) << 8) |
		(((uint64_t) arg1) << 16) | (((uint64_t) arg2) << 32) |
		(((uint64_t) arg3) << 48);
}


// Returns an instruction's opcode.
Opcode ins_opcode(uint64_t instruction) {
	return (Opcode) (instruction & 0xff);
}


// Returns the `arg`th argument of an instruction.
uint16_t ins_arg(uint64_t instruction, int arg) {
	uint32_t offset;
	uint64_t selection;
	if (arg == 0) {
		offset = 8;
		selection = 0xff;
	} else {
		offset = arg * 16;
		selection = 0xffff;
	}

	uint64_t and = selection << offset;
	return (uint16_t) ((instruction & and) >> offset);
}


// Returns `instruction` with a modified opcode.
uint64_t ins_set_opcode(uint64_t instruction, Opcode opcode) {
	uint64_t cleared = instruction & ((uint64_t) 0xffffffffffffff00);
	return cleared | ((uint8_t) opcode);
}


// Returns `instruction` with the `arg`th argument modified.
uint64_t ins_set_arg(uint64_t instruction, int arg, uint16_t value) {
	uint32_t offset;
	uint64_t to_clear;
	if (arg == 0) {
		offset = 8;
		to_clear = 0xff;
	} else {
		offset = arg * 16;
		to_clear = 0xffff;
	}

	uint64_t xor = to_clear << offset;
	uint64_t cleared = instruction & (((uint64_t) 0xffffffffffffffff) ^ xor);
	return cleared | (((uint64_t) value) << offset);
}
