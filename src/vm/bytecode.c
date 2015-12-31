
//
//  Bytecode
//

#include "bytecode.h"
#include "util.h"


//
//  Instructions
//

// Creates an instruction from an opcode and 3 arguments. Sets the 0th argument
// to 0.
uint64_t instr_new(Opcode opcode, uint16_t arg1, uint16_t arg2, uint16_t arg3) {
	return ((uint64_t) opcode) | (((uint64_t) arg1) << 16) |
		(((uint64_t) arg2) << 32) | (((uint64_t) arg3) << 48);
}


// Creates an instruction from an opcode and 4 arguments.
uint64_t instr_new_4(Opcode opcode, uint8_t arg0, uint16_t arg1, uint16_t arg2,
		uint16_t arg3) {
	return ((uint64_t) opcode) | (((uint64_t) arg0) << 8) |
		(((uint64_t) arg1) << 16) | (((uint64_t) arg2) << 32) |
		(((uint64_t) arg3) << 48);
}


// Returns an instruction's opcode.
Opcode instr_opcode(uint64_t instruction) {
	return (Opcode) (instruction & 0xff);
}


// Returns the `n`th argument of an instruction.
uint16_t instr_argument(uint64_t instruction, int n) {
	if (n == 0) {
		return (uint16_t) ((instruction & 0xff00) >> 8);
	} else {
		int offset = n * 16;
		uint64_t and = ((uint64_t) 0xffff) << offset;
		return (uint16_t) ((instruction & and) >> offset);
	}
}


// Returns `instruction` with a modified opcode.
uint64_t instr_modify_opcode(uint64_t instruction, Opcode new_opcode) {
	uint64_t cleared = instruction & ((uint64_t) 0xffffffffffffff00);
	return cleared | ((uint8_t) new_opcode);
}


// Returns `instruction` with the `n`th argument modified.
uint64_t instr_modify_argument(uint64_t instruction, int n,
		uint16_t new_argument) {
	if (n == 0) {
		uint64_t cleared = instruction & ((uint64_t) 0xffffffffffff00ff);
		return cleared | (((uint64_t) new_argument) << 8);
	} else {
		int offset = n * 16;
		uint64_t xor = ((uint64_t) 0xffff) << offset;
		uint64_t cleared = instruction & (((uint64_t) 0xffffffffffffffff) ^ xor);
		return cleared | (((uint64_t) new_argument) << offset);
	}
}



//
//  Bytecode
//

// Appends an instruction to the end of a function's bytecode. Returns the index
// of the instruction in the function's bytecode.
int emit(Function *fn, uint64_t instruction) {
	int index = fn->bytecode_count++;
	ARRAY_REALLOC(fn->bytecode, uint64_t);
	fn->bytecode[index] = instruction;
	return index;
}


// Appends an empty jump instruction (with no target set) to the end of a
// function's bytecode. Returns the index of the jump instruction.
int jmp_new(Function *fn) {
	return emit(fn, instr_new(JMP, 0, 0, 0));
}
