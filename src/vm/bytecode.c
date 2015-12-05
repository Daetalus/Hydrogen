
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
	return (uint16_t) (jump - target);
}


// Returns the target of the jump instruction.
uint32_t jmp_target(Function *fn, uint32_t jump) {
	uint16_t offset = instr_arg(fn->bytecode[jump], JUMP_TARGET_ARG);
	if (offset == 0) {
		return 0;
	} else {
		return jump + offset;
	}
}


// Sets the target of the jump instruction at `index` within the function's
// bytecode.
void jmp_set_target(Function *fn, uint32_t jump, uint32_t target) {
	uint16_t offset = target - jump;
	uint64_t instr = fn->bytecode[jump];
	fn->bytecode[jump] = instr_set(instr, JUMP_TARGET_ARG, offset);
}


// Returns the index of the next jump instruction in a jump list, or 0 if this
// is the end of the jump list.
uint32_t jmp_next(Function *fn, uint32_t jump) {
	uint16_t offset = instr_arg(fn->bytecode[jump], JUMP_LIST_ARG);
	if (offset == 0) {
		return 0;
	} else {
		return jump - (uint32_t) offset;
	}
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


// Returns the type of a jump instruction.
JumpType jmp_type(Function *fn, uint32_t jump) {
	uint16_t arg = instr_arg(fn->bytecode[jump], JUMP_TYPE_ARG);
	return (JumpType) arg;
}


// Sets which type of condition a jump instruction belongs to.
void jmp_set_type(Function *fn, uint32_t jump, JumpType type) {
	uint64_t instr = fn->bytecode[jump];
	fn->bytecode[jump] = instr_set(instr, JUMP_TYPE_ARG, (uint16_t) type);
}


// Returns the inverted condition for the given conditional opcode.
Opcode invert_condition(Opcode opcode) {
	if (opcode == IS_TRUE_L) {
		return IS_FALSE_L;
	} else if (opcode == IS_FALSE_L) {
		return IS_TRUE_L;
	} else if (opcode >= EQ_LL && opcode <= EQ_LP) {
		return NEQ_LL + (opcode - EQ_LL);
	} else if (opcode >= NEQ_LL && opcode <= NEQ_LP) {
		return EQ_LL + (opcode - NEQ_LL);
	} else if (opcode >= LT_LL && opcode <= LT_LN) {
		return GE_LL + (opcode - LT_LL);
	} else if (opcode >= LE_LL && opcode <= LE_LN) {
		return GT_LL + (opcode - LE_LL);
	} else if (opcode >= GT_LL && opcode <= GT_LN) {
		return LE_LL + (opcode - GT_LL);
	} else if (opcode >= GE_LL && opcode <= GE_LN) {
		return LT_LL + (opcode - GE_LL);
	} else {
		return NO_OP;
	}
}


// Inverts the condition of a conditional jump.
void jmp_invert_condition(Function *fn, uint32_t jump) {
	uint64_t condition = fn->bytecode[jump - 1];
	Opcode current = (Opcode) instr_arg(condition, 0);
	Opcode inverted = invert_condition(current);
	fn->bytecode[jump - 1] = instr_set(condition, 0, (uint16_t) inverted);
}


// Finalises a jump condition, assuming the true case is directly after the
// instructions used to evaluate the condition, and the false case is at the
// given index.
void jmp_patch(Function *fn, uint32_t jump, uint32_t false_case) {
	// Iterate over jump list
	uint32_t current = jump;
	while (current != JUMP_LIST_END) {
		// If the jump's target hasn't already been set
		if (jmp_target(fn, current) == 0) {
			// Point the jump to the false case
			jmp_set_target(fn, current, false_case);
		}

		// Get next element in jump list
		current = jmp_next(fn, current);
	}

	// Point the operand to the false case
	jmp_set_target(fn, jump, false_case);
}
