
//
//  Bytecode
//

#ifndef BYTECODE_H
#define BYTECODE_H

#include "vm.h"

// The value of a jump list argument that signals the end of the jump list.
#define JUMP_LIST_END 0


// All bytecode operation codes.
typedef enum {
	// Storage instructions
	MOV_LL,
	MOV_LI,
	MOV_LN,
	MOV_LS,
	MOV_LP,

	// Math instructions
	ADD_LL,
	ADD_LI,
	ADD_LN,
	ADD_IL,
	ADD_NL,

	SUB_LL,
	SUB_LI,
	SUB_LN,
	SUB_IL,
	SUB_NL,

	MUL_LL,
	MUL_LI,
	MUL_LN,
	MUL_IL,
	MUL_NL,

	DIV_LL,
	DIV_LI,
	DIV_LN,
	DIV_IL,
	DIV_NL,

	MOD_LL,
	MOD_LI,
	MOD_LN,
	MOD_IL,
	MOD_NL,

	CONCAT_LL,
	CONCAT_LS,
	CONCAT_SL,

	NEG_L,

	// Comparison
	IS_TRUE_L,
	IS_FALSE_L,

	EQ_LL,
	EQ_LI,
	EQ_LN,
	EQ_LS,
	EQ_LP,

	NEQ_LL,
	NEQ_LI,
	NEQ_LN,
	NEQ_LS,
	NEQ_LP,

	LT_LL,
	LT_LI,
	LT_LN,

	LE_LL,
	LE_LI,
	LE_LN,

	GT_LL,
	GT_LI,
	GT_LN,

	GE_LL,
	GE_LI,
	GE_LN,

	// Control flow
	JMP,
	LOOP,

	// No operation
	NO_OP,

	// Return
	RET0,
} Opcode;

// Creates an instruction from an opcode and arguments.
uint64_t instr_new(Opcode opcode, uint16_t arg1, uint16_t arg2, uint16_t arg3);

// Returns an argument to an instruction, where argument 0 is the opcode.
uint16_t instr_arg(uint64_t instruction, int arg);

// Modifies an argument to an instruction.
uint64_t instr_set(uint64_t instruction, int arg, uint16_t value);


// Emits an instruction for a function. Returns the index of the instruction in
// the bytecode.
uint32_t emit(Function *fn, uint64_t instruction);

// Emits an empty jump instruction. Returns the index of the jump instruction.
uint32_t jmp_new(Function *fn);

// Sets the target of the jump instruction at `index` within the function's
// bytecode.
void jmp_target(Function *fn, uint32_t jump, uint32_t target);


// The type of conditions a jump instruction can belong to.
typedef enum {
	JUMP_NONE,
	JUMP_AND,
	JUMP_OR,
} JumpType;

// Returns the index of the next jump instruction in a jump list, or 0 if this
// is the end of the jump list.
uint32_t jmp_next(Function *fn, uint32_t jump);

// Returns the index of the last jump in a jump list.
uint32_t jmp_last(Function *fn, uint32_t jump);

// Points `jump`'s jump list to `target`.
void jmp_point(Function *fn, uint32_t jump, uint32_t target);

// Returns the type of a jump instruction.
JumpType jmp_type(Function *fn, uint32_t jump);

// Sets which type of condition a jump instruction belongs to.
void jmp_set_type(Function *fn, uint32_t jump, JumpType type);

// Inverts the condition of a conditional jump.
void jmp_invert_condition(Function *fn, uint32_t jump);

#endif
