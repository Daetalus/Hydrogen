
//
//  Jumps
//

#include "jmp.h"
#include "../bytecode.h"

// The argument in a jump instruction used to store it's target offset.
#define JUMP_TARGET_ARG 1

// The argument in a jump instruction used to store it's jump list pointer.
#define JUMP_LIST_ARG 2

// The argument in a jump instruction used to store the type of condition the
// jump belongs to.
#define JUMP_TYPE_ARG 3


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
