
//
//  Bytecode
//

#include "bytecode.h"
#include "util.h"

// The argument in a jump instruction used to store it's target offset.
#define JUMP_TARGET_ARG 1

// The argument in a jump instruction used to store it's jump list pointer.
#define JUMP_LIST_ARG 2

// The argument in a jump instruction used to store the type of condition the
// jump belongs to.
#define JUMP_TYPE_ARG 3


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



//
//  Jumps
//

// Sets the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target`.
void jmp_target(Function *fn, int jump, int target) {
	fn->bytecode[jump] = instr_modify_argument(fn->bytecode[jump],
		JUMP_TARGET_ARG, target - jump);
}


// Sets the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target`, if the jump instruction doesn't already have a target set.
void jmp_lazy_target(Function *fn, int jump, int target) {
	uint16_t offset = instr_argument(fn->bytecode[jump], JUMP_TARGET_ARG);
	if (offset == 0) {
		jmp_target(fn, jump, target);
	}
}


// Iterates over the jump list of the jump instruction at `jump` inside `fn`'s
// bytecode, setting the target of each jump instruction in the list to
// `target`.
void jmp_target_all(Function *fn, int jump, int target) {
	while (jump != -1) {
		jmp_target(fn, jump, target);
		jump = jmp_next(fn, jump);
	}
}


// Returns the index of the next jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
int jmp_next(Function *fn, int jump) {
	uint16_t offset = instr_argument(fn->bytecode[jump], JUMP_LIST_ARG);
	return (offset == 0) ? -1 : jump - offset;
}


// Returns the index of the last jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
int jmp_last(Function *fn, int jump) {
	int next = jmp_next(fn, jump);
	while (next != -1) {
		jump = next;
		next = jmp_next(fn, jump);
	}
	return jump;
}


// Adds the jump instruction at index `target` to a jump list, after the jump
// at `jump`.
void jmp_append(Function *fn, int jump, int target) {
	fn->bytecode[jump] = instr_modify_argument(fn->bytecode[jump], JUMP_LIST_ARG,
		jump - target);
}


// Returns the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to.
JumpType jmp_type(Function *fn, int jump) {
	return (JumpType) instr_argument(fn->bytecode[jump], JUMP_TYPE_ARG);
}


// Sets the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to.
void jmp_set_type(Function *fn, int jump, JumpType type) {
	fn->bytecode[jump] = instr_modify_argument(fn->bytecode[jump], JUMP_TYPE_ARG,
		(uint16_t) type);
}
