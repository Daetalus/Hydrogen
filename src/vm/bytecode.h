
//
//  Bytecode
//


#ifndef BYTECODE_H
#define BYTECODE_H

// Extracts an argument from a 64 bit instruction.
#define INSTRUCTION_ARG(instr, arg)                                           \
	((uint16_t) ((((uint64_t) (instr)) & (((uint64_t) 0xffff) << (arg * 16))) \
		>> (arg * 16)))


// Creates an instruction from an opcode and 3 arguments.
#define INSTRUCTION(opcode, arg1, arg2, arg3)              \
	(((uint64_t) (opcode)) | (((uint64_t) (arg1)) << 16) | \
	 (((uint64_t) (arg2)) << 32) | (((uint64_t) (arg3)) << 48))


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

#endif
