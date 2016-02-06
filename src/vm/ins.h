
//
//  Instructions
//

#ifndef INS_H
#define INS_H

#include <stdint.h>

#include "bytecode.h"


// A bytecode instruction (64 bits).
typedef uint64_t Instruction;


// Create a new instruction from an opcode and 3 arguments.
static inline Instruction ins_new(BytecodeOpcode opcode, uint16_t arg1,
		uint16_t arg2, uint16_t arg3) {
	return ((Instruction) opcode) |
		(((Instruction) arg1) << 16) |
		(((Instruction) arg2) << 32) |
		(((Instruction) arg3) << 48);
}


// Returns the `n`th argument of an instruction. Argument 0 is the instruction's
// opcode.
static inline uint16_t ins_arg(Instruction instruction, uint32_t n) {
	uint32_t offset = n * 16;
	return (uint16_t) ((instruction & ((uint64_t) 0xffff) << offset) >> offset);
}


// Sets the `n`th argument of an instruction. Argument 0 is the instruction's
// opcode.
static inline Instruction ins_set(Instruction instruction, uint32_t n,
		uint16_t value) {
	uint32_t offset = n * 16;
	uint64_t selection = (((uint64_t) 0xffff) << offset);
	uint64_t to_clear = (((uint64_t) 0xffffffffffffffff) ^ selection);
	uint64_t cleared = instruction & to_clear;
	return cleared | (((uint64_t) value) << offset);
}

#endif
