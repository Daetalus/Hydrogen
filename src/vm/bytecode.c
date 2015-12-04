
//
//  Bytecode
//

#include "bytecode.h"
#include "util.h"


// Creates an instruction from an opcode and arguments.
uint64_t instr_new(Opcode opcode, uint16_t arg1, uint16_t arg2, uint16_t arg3) {
	return ((uint64_t) opcode) |
		(((uint64_t) arg1) << 16) |
		(((uint64_t) arg2) << 32) |
		(((uint64_t) arg3) << 48);
}


// Returns an argument to an instruction, where argument 0 is the opcode.
uint16_t instr_arg(uint64_t instruction, int arg) {
	int offset = arg * 16;
	uint64_t and = ((uint64_t) 0xffff) << offset;
	uint64_t offset_arg = instruction & and;
	return (uint16_t) (offset_arg >> offset);
}


// Modifies an argument to an instruction.
uint64_t instr_set(uint64_t instruction, int arg, uint16_t value) {
	int offset = arg * 16;
	uint64_t xor = ((uint64_t) 0xffff) << offset;
	uint64_t cleared = instruction & (((uint64_t) 0xffffffffffffffff) ^ xor);
	uint64_t offset_value = ((uint64_t) value) << offset;
	return cleared | offset_value;
}


// Emits an instruction for a function. Returns the index of the instruction in
// the bytecode.
uint32_t emit(Function *fn, uint64_t instruction) {
	uint32_t index = fn->bytecode_count++;
	ARRAY_REALLOC(fn->bytecode, uint64_t);
	fn->bytecode[index] = instruction;
	return index;
}


// Emits an empty jump instruction. Returns the index of the jump instruction.
uint32_t jmp_new(Function *fn) {
	return emit(fn, instr_new(JMP, 0, 0, 0));
}


// Sets the target of the jump instruction at `index` within the function's
// bytecode.
void jmp_set(Function *fn, uint32_t index, uint32_t target) {
	uint16_t offset = target - index;
	fn->bytecode[index] = instr_set(fn->bytecode[index], 1, offset);
}
