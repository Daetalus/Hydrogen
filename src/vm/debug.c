
//
//  Debug
//

#include <stdio.h>

#include "debug.h"
#include "bytecode.h"


// The maximum length of an opcode's name.
#define MAX_NAME_LENGTH 50


// The name of each opcode, in the exact order they were defined in.
static char OPCODE_NAMES[][MAX_NAME_LENGTH] = {
	"MOV_LL", "MOV_LI", "MOV_LN", "MOV_LS", "MOV_LP", "MOV_LF",
	"MOV_LU", "MOV_UL", "UPVALUE_CLOSE",
	"MOV_LT", "MOV_TL",

	"ADD_LL", "ADD_LI", "ADD_LN", "ADD_IL", "ADD_NL",
	"SUB_LL", "SUB_LI", "SUB_LN", "SUB_IL", "SUB_NL",
	"MUL_LL", "MUL_LI", "MUL_LN", "MUL_IL", "MUL_NL",
	"DIV_LL", "DIV_LI", "DIV_LN", "DIV_IL", "DIV_NL",
	"MOD_LL", "MOD_LI", "MOD_LN", "MOD_IL", "MOD_NL",
	"CONCAT_LL", "CONCAT_LS", "CONCAT_SL",
	"NEG_L",

	"IS_TRUE_L", "IS_FALSE_L",
	"EQ_LL", "EQ_LI", "EQ_LN", "EQ_LS", "EQ_LP", "EQ_LF",
	"NEQ_LL", "NEQ_LI", "NEQ_LN", "NEQ_LS", "NEQ_LP", "NEQ_LF",
	"LT_LL", "LT_LI", "LT_LN",
	"LE_LL", "LE_LI", "LE_LN",
	"GT_LL", "GT_LI", "GT_LN",
	"GE_LL", "GE_LI", "GE_LN",

	"JMP", "LOOP",
	"CALL_L", "CALL_F", "CALL_NATIVE", "RET0", "RET1",
	"STRUCT_NEW", "STRUCT_FIELD", "STRUCT_SET",

	"NO_OP",
};


// The number of arguments each opcode accepts, in the same order in which they
// were defined.
static uint32_t ARGUMENT_COUNT[] = {
	2, /* MOV_LL */ 2, /* MOV_LI */ 2, /* MOV_LN */ 2, /* MOV_LS */
	2, /* MOV_LP */ 2, /* MOV_LF */
	2, /* MOV_LU */ 2, /* MOV_UL */ 1, /* UPVALUE_CLOSE */
	3, /* MOV_LT */ 3, /* MOV_TL */

	3, /* ADD_LL */ 3, /* ADD_LI */ 3, /* ADD_LN */ 3, /* ADD_IL */
	3, /* ADD_NL */
	3, /* SUB_LL */ 3, /* SUB_LI */ 3, /* SUB_LN */ 3, /* SUB_IL */
	3, /* SUB_NL */
	3, /* MUL_LL */ 3, /* MUL_LI */ 3, /* MUL_LN */ 3, /* MUL_IL */
	3, /* MUL_NL */
	3, /* DIV_LL */ 3, /* DIV_LI */ 3, /* DIV_LN */ 3, /* DIV_IL */
	3, /* DIV_NL */
	3, /* MOD_LL */ 3, /* MOD_LI */ 3, /* MOD_LN */ 3, /* MOD_IL */
	3, /* MOD_NL */
	3, /* CONCAT_LL */ 3, /* CONCAT_LS */ 3, /* CONCAT_SL */
	2, /* NEG_L */

	1, /* IS_TRUE_L */ 1, /* IS_FALSE_L */
	2, /* EQ_LL */ 2, /* EQ_LI */ 2, /* EQ_LN */ 2, /* EQ_LS */ 2, /* EQ_LP */
	2, /* EQ_LF */
	2, /* NEQ_LL */ 2, /* NEQ_LI */ 2, /* NEQ_LN */ 2, /* NEQ_LS */
	2, /* NEQ_LP */ 2, /* NEQ_LF */
	2, /* LT_LL */ 2, /* LT_LI */ 2, /* LT_LN */
	2, /* LE_LL */ 2, /* LE_LI */ 2, /* LE_LN */
	2, /* GT_LL */ 2, /* GT_LI */ 2, /* GT_LN */
	2, /* GE_LL */ 2, /* GE_LI */ 2, /* GE_LN */

	1, /* JMP */ 1, /* LOOP */

	4, /* CALL_L */ 4, /* CALL_F */ 4, /* CALL_NATIVE */
	0, /* RET0 */ 1, /* RET1 */
	2, /* STRUCT_NEW */ 3, /* STRUCT_FIELD */ 3, /* STRUCT_SET */

	0, /* NO_OP */
};


// Prints the opcode of an instruction.
static void debug_opcode(uint64_t instruction) {
	uint16_t opcode = ins_opcode(instruction);
	char *name = OPCODE_NAMES[opcode];
	printf("%-12s ", name);
}


// Prints the arguments to an instruction.
static void debug_arguments(uint64_t instruction) {
	uint16_t opcode = ins_opcode(instruction);
	uint32_t count = ARGUMENT_COUNT[opcode];
	for (uint32_t i = 0; i < count; i++) {
		// If this is a 4 argument opcode, we need to start indexing arguments
		// at 0 (to print the 4th, 8 bit argument)
		uint32_t index = (count == 4) ? i : i + 1;
		uint16_t arg = ins_arg(instruction, index);
		printf("%-6u ", arg);
	}
}


// Prints the destination for a jump or loop instruction.
static void debug_jump_destination(uint32_t index, uint64_t instruction) {
	uint16_t opcode = ins_opcode(instruction);

	// Only for jump or loop instructions
	if (opcode != JMP && opcode != LOOP) {
		return;
	}

	int32_t offset = ins_arg(instruction, 1);

	// Subtract the offset, rather than add it, for a loop, since loops jump
	// backwards in the bytecode
	if (opcode == LOOP) {
		offset *= -1;
	}

	uint32_t destination = index + offset;
	printf("==> %d", destination);
}


// Pretty prints `instruction` to the standard output. `index` specifies the
// index of the instruction in the bytecode, used to calculate the destination
// for a jump instruction.
void debug_instruction(uint32_t index, uint64_t instruction) {
	printf("%8d: ", index);
	debug_opcode(instruction);
	debug_arguments(instruction);
	debug_jump_destination(index, instruction);
	printf("\n");
}


// Pretty prints `fn`'s bytecode to the standard output.
void debug_bytecode(Function *fn) {
	for (uint32_t i = 0; i < fn->bytecode_count; i++) {
		debug_instruction(i, fn->bytecode[i]);
	}
}
