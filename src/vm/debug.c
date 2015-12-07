
//
//  Debug
//

#include <stdio.h>

#include "debug.h"
#include "bytecode.h"


// The maximum length of an opcode's name.
#define MAX_NAME_LENGTH 50

// The number of opcodes.
#define OPCODE_COUNT 71


// The name of every opcode.
char OPCODE_NAMES[OPCODE_COUNT][MAX_NAME_LENGTH] = {
	"MOV_LL",
	"MOV_LI",
	"MOV_LN",
	"MOV_LS",
	"MOV_LP",
	"MOV_LF",
	"ADD_LL",
	"ADD_LI",
	"ADD_LN",
	"ADD_IL",
	"ADD_NL",
	"SUB_LL",
	"SUB_LI",
	"SUB_LN",
	"SUB_IL",
	"SUB_NL",
	"MUL_LL",
	"MUL_LI",
	"MUL_LN",
	"MUL_IL",
	"MUL_NL",
	"DIV_LL",
	"DIV_LI",
	"DIV_LN",
	"DIV_IL",
	"DIV_NL",
	"MOD_LL",
	"MOD_LI",
	"MOD_LN",
	"MOD_IL",
	"MOD_NL",
	"CONCAT_LL",
	"CONCAT_LS",
	"CONCAT_SL",
	"NEG_L",
	"IS_TRUE_L",
	"IS_FALSE_L",
	"EQ_LL",
	"EQ_LI",
	"EQ_LN",
	"EQ_LS",
	"EQ_LP",
	"NEQ_LL",
	"NEQ_LI",
	"NEQ_LN",
	"NEQ_LS",
	"NEQ_LP",
	"LT_LL",
	"LT_LI",
	"LT_LN",
	"LE_LL",
	"LE_LI",
	"LE_LN",
	"GT_LL",
	"GT_LI",
	"GT_LN",
	"GE_LL",
	"GE_LI",
	"GE_LN",
	"JMP",
	"LOOP",
	"CALL_L",
	"CALL_F",
	"RET",
	"RET_L",
	"RET_I",
	"RET_N",
	"RET_S",
	"RET_P",
	"RET_F",
	"NO_OP",
};


// The number of arguments for each opcode.
int ARGUMENT_COUNT[OPCODE_COUNT] = {
	2, // MOV_LL
	2, // MOV_LI
	2, // MOV_LN
	2, // MOV_LS
	2, // MOV_LP
	2, // MOV_LF
	3, // ADD_LL
	3, // ADD_LI
	3, // ADD_LN
	3, // ADD_IL
	3, // ADD_NL
	3, // SUB_LL
	3, // SUB_LI
	3, // SUB_LN
	3, // SUB_IL
	3, // SUB_NL
	3, // MUL_LL
	3, // MUL_LI
	3, // MUL_LN
	3, // MUL_IL
	3, // MUL_NL
	3, // DIV_LL
	3, // DIV_LI
	3, // DIV_LN
	3, // DIV_IL
	3, // DIV_NL
	3, // MOD_LL
	3, // MOD_LI
	3, // MOD_LN
	3, // MOD_IL
	3, // MOD_NL
	3, // CONCAT_LL
	3, // CONCAT_LS
	3, // CONCAT_SL
	2, // NEG_L
	2, // IS_TRUE_L
	2, // IS_FALSE_L
	2, // EQ_LL
	2, // EQ_LI
	2, // EQ_LN
	2, // EQ_LS
	2, // EQ_LP
	2, // NEQ_LL
	2, // NEQ_LI
	2, // NEQ_LN
	2, // NEQ_LS
	2, // NEQ_LP
	2, // LT_LL
	2, // LT_LI
	2, // LT_LN
	2, // LE_LL
	2, // LE_LI
	2, // LE_LN
	2, // GT_LL
	2, // GT_LI
	2, // GT_LN
	2, // GE_LL
	2, // GE_LI
	2, // GE_LN
	1, // JMP
	1, // LOOP
	4, // CALL_L
	4, // CALL_F
	0, // RET
	1, // RET_L
	1, // RET_I
	1, // RET_N
	1, // RET_S
	1, // RET_P
	1, // RET_F
	0, // NO_OP
};


// Prints a single instruction.
void debug_print_instruction(int i, uint64_t instruction) {
	// Number
	printf("%4d: ", i);

	// Opcode
	uint16_t opcode = instr_opcode(instruction);
	char *name = &OPCODE_NAMES[opcode][0];
	printf("%-10s ", name);

	// Arguments
	int argument_count = ARGUMENT_COUNT[opcode];
	for (int i = 0; i < argument_count; i++) {
		int index = (argument_count == 4) ? i : i + 1;
		uint16_t arg = instr_argument(instruction, index);
		printf("%-6u ", arg);
	}

	// Jump destination
	if (opcode == JMP) {
		uint32_t destination = i + instr_argument(instruction, 1);
		printf("==> %d", destination);
	}

	printf("\n");
}


// Prints out a function's bytecode.
void debug_print_bytecode(Function *fn) {
	for (uint32_t i = 0; i < fn->bytecode_count; i++) {
		debug_print_instruction(i, fn->bytecode[i]);
	}
}
