
//
//  Instructions
//

#ifndef INS_H
#define INS_H

#include <stdint.h>

#include "bytecode.h"


// A bytecode instruction (64 bits)
typedef uint64_t Instruction;


// Create a new instruction from an opcode and 3 arguments
static inline Instruction ins_new(BytecodeOpcode opcode, uint16_t arg1,
		uint16_t arg2, uint16_t arg3) {
	return ((Instruction) opcode) |
		(((Instruction) arg1) << 16) |
		(((Instruction) arg2) << 32) |
		(((Instruction) arg3) << 48);
}


// Returns the `n`th argument of an instruction. Argument 0 is the instruction's
// opcode
static inline uint16_t ins_arg(Instruction ins, uint32_t n) {
	return (uint16_t) ((ins >> (n << 4)) & 0xffff);
}


// Sets the `n`th argument of an instruction. Argument 0 is the instruction's
// opcode
static inline Instruction ins_set(Instruction ins, uint32_t n, uint16_t value) {
	uint64_t cleared = ins & ~(((uint64_t) 0xffff) << (n << 4));
	return cleared | (((uint64_t) value) << (n << 4));
}

#endif
