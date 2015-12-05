
//
//  Bytecode
//

#include "bytecode.h"
#include "util.h"

// The argument in a jump instruction used to store it's target offset.
#define JUMP_TARGET_ARG 1

// The argument in a jump instruction used to store it's jump list pointer.
#define JUMP_LIST_ARG 2

// The value of a jump list argument that signals the end of the jump list.
#define JUMP_LIST_END 0


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


// Returns the jump offset between two indices in a function's bytecode.
uint16_t jmp_offset(uint32_t jump, uint32_t target) {
	return (uint16_t) (target - jump);
}


// Sets the target of the jump instruction at `index` within the function's
// bytecode.
void jmp_target(Function *fn, uint32_t jump, uint32_t target) {
	uint16_t offset = jmp_offset(jump, target);
	uint64_t instr = fn->bytecode[jump];
	fn->bytecode[jump] = instr_set(instr, JUMP_TARGET_ARG, offset);
}


// Returns the index of the next jump instruction in a jump list, or 0 if this
// is the end of the jump list.
uint32_t jmp_next(Function *fn, uint32_t jump) {
	return instr_arg(fn->bytecode[jump], JUMP_LIST_ARG);
}


// Returns the index of the last jump in a jump list.
uint32_t jmp_last(Function *fn, uint32_t jump) {
	uint32_t current = jump;
	uint32_t next = jmp_next(fn, current);
	while (next != JUMP_LIST_END) {
		current = next;
		next = jmp_next(fn, current);
	}
	return current;
}


// Points `jump`'s jump list to `target`.
void jmp_point(Function *fn, uint32_t jump, uint32_t target) {
	uint16_t offset = jmp_offset(jump, target);
	uint64_t instr = fn->bytecode[jump];
	fn->bytecode[jump] = instr_set(instr, JUMP_LIST_ARG, offset);
}
