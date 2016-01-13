
//
//  Jumps
//

#ifndef JMP_H
#define JMP_H

#include "../vm.h"


// * A jump list is a collection of jump instructions that are chained together
//   in a linked list fashion
// * Each jump instruction points to the jump before it in the list


// The different types of conditions a jump instruction can belong to.
typedef enum {
	JUMP_NONE,
	JUMP_AND,
	JUMP_OR,
} JumpType;


// Sets the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target`.
void jmp_target(Function *fn, int jump, int target);

// Sets the target of the jump instruction at `jump` inside `fn`'s bytecode to
// `target`, if the jump instruction doesn't already have a target set.
void jmp_lazy_target(Function *fn, int jump, int target);

// Iterates over the jump list of the jump instruction at `jump` inside `fn`'s
// bytecode, setting the target of each jump instruction in the list to
// `target`.
void jmp_target_all(Function *fn, int jump, int target);

// Returns the index of the next jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
int jmp_next(Function *fn, int jump);

// Returns the index of the last jump instruction in the jump list starting at
// `jump` in `fn`'s bytecode.
int jmp_last(Function *fn, int jump);

// Adds the jump instruction at index `target` to a jump list, after the jump
// at `jump`.
void jmp_append(Function *fn, int jump, int target);

// Returns the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to.
JumpType jmp_type(Function *fn, int jump);

// Sets the type of conditional the jump instruction at `jump` in `fn`'s
// bytecode belongs to.
void jmp_set_type(Function *fn, int jump, JumpType type);

#endif
