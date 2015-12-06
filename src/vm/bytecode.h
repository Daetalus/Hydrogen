
//
//  Bytecode
//

#ifndef BYTECODE_H
#define BYTECODE_H

#include "vm.h"

// The value of a jump list argument that signals the end of the jump list.
#define JUMP_LIST_END 0


// All bytecode operation codes. Opcodes are stored in the first byte of an
// instruction, so there cannot be more than 256 opcodes.
typedef enum {
	// Storage instructions
	MOV_LL, // destination, slot
	MOV_LI, // destination, integer
	MOV_LN, // destination, number index
	MOV_LS, // destination, string index
	MOV_LP, // destination, primitive tag
	MOV_LF, // destination, function index

	// Math instructions
	ADD_LL, // destination, slot, slot
	ADD_LI, // destination, slot, integer
	ADD_LN, // ...
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
	IS_TRUE_L, // slot
	IS_FALSE_L, // slot

	EQ_LL, // slot, slot
	EQ_LI, // slot, integer
	EQ_LN, // ...
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
	JMP, // target offset (forwards)
	LOOP, // target offset (backwards)

	// Function calls
	CALL, // arity, slot with function as local, slot with first argument,
		  // slot for return value
	RET0, // none
	RET1, // slot return value was stored in

	// No operation
	NO_OP, // none
} Opcode;

// Creates an instruction from an opcode and arguments.
uint64_t instr_new(Opcode opcode, uint8_t arg0, uint16_t arg1, uint16_t arg2,
	uint16_t arg3);

// Returns the opcode of an instruction.
Opcode instr_opcode(uint64_t instruction);

// Sets the opcode for an instruction.
uint64_t instr_set_opcode(uint64_t instruction, Opcode opcode);

// Returns an argument to an instruction. Arguments 0 to 3 can be fetched.
// Argument 0 is only 8 bits (1 byte), the other 3 are 16 bits (2 bytes).
uint16_t instr_arg(uint64_t instruction, int arg);

// Modifies an argument to an instruction. Arguments 0 to 3 can be set.
// Argument 0 is only 8 bits (1 byte), the other 3 are 16 bits (2 bytes).
uint64_t instr_set(uint64_t instruction, int arg, uint16_t value);


// Emits an instruction for a function. Returns the index of the instruction in
// the bytecode.
uint32_t emit(Function *fn, uint64_t instruction);

// Emits an empty jump instruction. Returns the index of the jump instruction.
uint32_t jmp_new(Function *fn);

// Returns the target of the jump instruction.
uint32_t jmp_target(Function *fn, uint32_t jump);

// Sets the target of the jump instruction at `index` within the function's
// bytecode.
void jmp_set_target(Function *fn, uint32_t jump, uint32_t target);


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

// Finalises a jump condition, assuming the true case is directly after the
// instructions used to evaluate the condition, and the false case is at the
// given index.
void jmp_patch(Function *fn, uint32_t jump, uint32_t false_case);

#endif
