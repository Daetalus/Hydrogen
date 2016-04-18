
//
//  Jump Lists
//

#ifndef JMP_H
#define JMP_H

#include <vec.h>

#include "ins.h"
#include "fn.h"

// * Jump lists are like linked lists, except for jump instructions inside a
//   function's bytecode
// * Each jump instruction points to the previous jump instruction in the list
//   by a relative offset stored in an argument in the instruction

// The index of the argument in a jump instruction used to store the offset to
// its target.
#define JMP_TARGET_ARG 1

// The index of the argument in a jump instruction used to store it's jump list
// pointer (where the next element in the jump list is).
#define JMP_LIST_ARG 2

// The index of the argument in a jump instruction used to store the type of
// condition the jump belongs to (and, or, none).
#define JMP_TYPE_ARG 3


// The different types of conditions a jump instruction can belong to. This is
// needed so we can target jump instructions differently depending on whether
// they belong to an `and` or `or` condition.
typedef enum {
	JMP_NONE,
	JMP_AND,
	JMP_OR,
} JmpType;


// Return the index of the next jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
static inline Index jmp_next(Function *fn, Index jump) {
	uint16_t offset = ins_arg(vec_at(fn->instructions, jump), JMP_LIST_ARG);
	return (offset == 0) ? NOT_FOUND : jump - offset;
}


// Return the index of the last jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
static inline Index jmp_last(Function *fn, Index jump) {
	Index next = jmp_next(fn, jump);
	while (next != NOT_FOUND) {
		jump = next;
		next = jmp_next(fn, jump);
	}
	return jump;
}


// Set the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target`.
static inline void jmp_target(Function *fn, Index jump, Index target) {
	uint16_t offset = target - jump;
	Instruction ins = vec_at(fn->instructions, jump);
	vec_at(fn->instructions, jump) = ins_set(ins, JMP_TARGET_ARG, offset);
}


// Set the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target` only if the jump instruction doesn't already have a target set.
static inline void jmp_lazy_target(Function *fn, Index jump, Index target) {
	uint16_t offset = ins_arg(vec_at(fn->instructions, jump), JMP_TARGET_ARG);
	if (offset == 0) {
		jmp_target(fn, jump, target);
	}
}


// Iterate over the jump list at `jump` inside `fn`'s bytecode and set the
// target of each jump to `target`.
static inline void jmp_target_all(Function *fn, Index jump, Index target) {
	while (jump != NOT_FOUND) {
		jmp_target(fn, jump, target);
		jump = jmp_next(fn, jump);
	}
}


// Add the jump at `to_append` to the end of the jump list `list`.
static inline void jmp_append(Function *fn, Index list, Index to_append) {
	Index last = jmp_last(fn, list);
	uint16_t offset = last - to_append;
	Instruction ins = vec_at(fn->instructions, last);
	vec_at(fn->instructions, last) = ins_set(ins, JMP_LIST_ARG, offset);
}


// Add the jump at `to_append` to the start of the jump list `list`.
static inline void jmp_prepend(Function *fn, Index *list, Index to_append) {
	if (*list != NOT_FOUND) {
		jmp_append(fn, to_append, *list);
	}
	*list = to_append;
}


// Return the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to.
static inline JmpType jmp_type(Function *fn, Index jump) {
	return (JmpType) ins_arg(vec_at(fn->instructions, jump), JMP_TYPE_ARG);
}


// Set the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to, if a value isn't already set.
static inline void jmp_set_type(Function *fn, Index jump, JmpType type) {
	if (jmp_type(fn, jump) == JMP_NONE) {
		Instruction ins = vec_at(fn->instructions, jump);
		vec_at(fn->instructions, jump) = ins_set(ins, JMP_TYPE_ARG, type);
	}
}


// Modify the target of all jumps in a conditional expression to point the
// location of the false case to `target`.
void jmp_false_case(Function *fn, Index jump, Index target);

// Invert the condition of a conditional jump operation.
void jmp_invert_condition(Function *fn, Index jump);

#endif
