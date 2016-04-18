
//
//  Debug
//

#include <stdio.h>

#include "debug.h"
#include "state.h"
#include "pkg.h"
#include "err.h"


// The name of each opcode, in the exact order they were defined in.
static char *opcode_names[] = {
	"MOV_LL", "MOV_LI", "MOV_LN", "MOV_LS", "MOV_LP", "MOV_LF", "MOV_LV",
	"MOV_UL", "MOV_UI", "MOV_UN", "MOV_US", "MOV_UP", "MOV_UF", "MOV_UV",
	"MOV_UL", "UPVALUE_CLOSE",
	"MOV_TL", "MOV_TI", "MOV_TN", "MOV_TS", "MOV_TP", "MOV_TF", "MOV_TV",
	"MOV_LT", "MOV_SELF",

	"ADD_LL", "ADD_LI", "ADD_LN", "ADD_IL", "ADD_NL",
	"SUB_LL", "SUB_LI", "SUB_LN", "SUB_IL", "SUB_NL",
	"MUL_LL", "MUL_LI", "MUL_LN", "MUL_IL", "MUL_NL",
	"DIV_LL", "DIV_LI", "DIV_LN", "DIV_IL", "DIV_NL",
	"MOD_LL", "MOD_LI", "MOD_LN", "MOD_IL", "MOD_NL",
	"CONCAT_LL", "CONCAT_LS", "CONCAT_SL",
	"NEG_L",

	"IS_TRUE_L", "IS_FALSE_L",
	"EQ_LL", "EQ_LI", "EQ_LN", "EQ_LS", "EQ_LP", "EQ_LF", "EQ_LV",
	"NEQ_LL", "NEQ_LI", "NEQ_LN", "NEQ_LS", "NEQ_LP", "NEQ_LF", "NEQ_LV",
	"LT_LL", "LT_LI", "LT_LN",
	"LE_LL", "LE_LI", "LE_LN",
	"GT_LL", "GT_LI", "GT_LN",
	"GE_LL", "GE_LI", "GE_LN",

	"JMP", "LOOP",
	"CALL", "RET0", "RET_L", "RET_I", "RET_N", "RET_S", "RET_P", "RET_F",
	"RET_V",

	"STRUCT_NEW", "STRUCT_CALL_CONSTRUCTOR", "STRUCT_FIELD",
	"STRUCT_SET_L", "STRUCT_SET_I", "STRUCT_SET_N", "STRUCT_SET_S",
	"STRUCT_SET_P", "STRUCT_SET_F", "STRUCT_SET_V",

	"ARRAY_NEW",
	"ARRAY_GET_L", "ARRAY_GET_I",
	"ARRAY_I_SET_L", "ARRAY_I_SET_I", "ARRAY_I_SET_N", "ARRAY_I_SET_S",
	"ARRAY_I_SET_P", "ARRAY_I_SET_F", "ARRAY_I_SET_V",
	"ARRAY_L_SET_L", "ARRAY_L_SET_I", "ARRAY_L_SET_N", "ARRAY_L_SET_S",
	"ARRAY_L_SET_P", "ARRAY_L_SET_F", "ARRAY_L_SET_V",

	"NO_OP",
};


// The number of arguments each opcode accepts, in the same order in which they
// were defined.
static uint32_t argument_count[] = {
	2, /* MOV_LL */ 2, /* MOV_LI */ 2, /* MOV_LN */ 2, /* MOV_LS */
	2, /* MOV_LP */ 2, /* MOV_LF */ 2, /* MOV_LV */

	2, /* MOV_UL */ 2, /* MOV_UI */ 2, /* MOV_UN */ 2, /* MOV_US */
	2, /* MOV_UP */ 2, /* MOV_UF */ 2, /* MOV_UV */
	2, /* MOV_LU */ 1, /* UPVALUE_CLOSE */

	3, /* MOV_TL */ 3, /* MOV_TI */ 3, /* MOV_TN */ 3, /* MOV_TS */
	3, /* MOV_TP */ 3, /* MOV_TF */ 3, /* MOV_TV */
	3, /* MOV_LT */ 1, /* MOV_SELF */

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
	2, /* EQ_LF */ 2, /* EQ_LV */
	2, /* NEQ_LL */ 2, /* NEQ_LI */ 2, /* NEQ_LN */ 2, /* NEQ_LS */
	2, /* NEQ_LP */ 2, /* NEQ_LF */ 2, /* NEQ_LV */
	2, /* LT_LL */ 2, /* LT_LI */ 2, /* LT_LN */
	2, /* LE_LL */ 2, /* LE_LI */ 2, /* LE_LN */
	2, /* GT_LL */ 2, /* GT_LI */ 2, /* GT_LN */
	2, /* GE_LL */ 2, /* GE_LI */ 2, /* GE_LN */

	1, /* JMP */ 1, /* LOOP */
	3, /* CALL */ 0, /* RET0 */ 2, /* RET_L */ 2, /* RET_I */ 2, /* RET_N */
	2, /* RET_S */ 2, /* RET_P */ 2, /* RET_F */ 2, /* RET_V */

	2, /* STRUCT_NEW */ 3, /* STRUCT_CALL_CONSTRUCTOR */ 3, /* STRUCT_FIELD */
	3, /* STRUCT_SET_L */ 3, /* STRUCT_SET_I */ 3, /* STRUCT_SET_N */
	3, /* STRUCT_SET_S */ 3, /* STRUCT_SET_P */ 3, /* STRUCT_SET_F */
	3, /* STRUCT_SET_V */

	2, /* ARRAY_NEW */
	3, /* ARRAY_GET_L */ 3, /* ARRAY_GET_I */
	3, /* ARRAY_I_SET_L */ 3, /* ARRAY_I_SET_I */ 3, /* ARRAY_I_SET_N */
	3, /* ARRAY_I_SET_S */ 3, /* ARRAY_I_SET_P */ 3, /* ARRAY_I_SET_F */
	3, /* ARRAY_I_SET_V */
	3, /* ARRAY_L_SET_L */ 3, /* ARRAY_L_SET_I */ 3, /* ARRAY_L_SET_N */
	3, /* ARRAY_L_SET_S */ 3, /* ARRAY_L_SET_P */ 3, /* ARRAY_L_SET_F */
	3, /* ARRAY_L_SET_V */

	0, /* NO_OP */
};


// The index of an argument in an instruction with the specified opcode that is
// an integer, so we can display negative numbers correctly. If the opcode has
// no integer arguments, the index is 0.
static uint32_t integer_argument[] = {
	0, /* MOV_LL */ 2, /* MOV_LI */ 0, /* MOV_LN */ 0, /* MOV_LS */
	0, /* MOV_LP */ 0, /* MOV_LF */ 0, /* MOV_LV */

	0, /* MOV_UL */ 2, /* MOV_UI */ 0, /* MOV_UN */ 0, /* MOV_US */
	0, /* MOV_UP */ 0, /* MOV_UF */ 0, /* MOV_UV */
	0, /* MOV_LU */ 0, /* UPVALUE_CLOSE */

	0, /* MOV_TL */ 2, /* MOV_TI */ 0, /* MOV_TN */ 0, /* MOV_TS */
	0, /* MOV_TP */ 0, /* MOV_TF */ 0, /* MOV_TV */
	0, /* MOV_LT */ 0, /* MOV_SELF */

	0, /* ADD_LL */ 3, /* ADD_LI */ 0, /* ADD_LN */ 2, /* ADD_IL */
	0, /* ADD_NL */
	0, /* SUB_LL */ 3, /* SUB_LI */ 0, /* SUB_LN */ 2, /* SUB_IL */
	0, /* SUB_NL */
	0, /* MUL_LL */ 3, /* MUL_LI */ 0, /* MUL_LN */ 2, /* MUL_IL */
	0, /* MUL_NL */
	0, /* DIV_LL */ 3, /* DIV_LI */ 0, /* DIV_LN */ 2, /* DIV_IL */
	0, /* DIV_NL */
	0, /* MOD_LL */ 3, /* MOD_LI */ 0, /* MOD_LN */ 2, /* MOD_IL */
	0, /* MOD_NL */
	0, /* CONCAT_LL */ 0, /* CONCAT_LS */ 0, /* CONCAT_SL */
	0, /* NEG_L */

	0, /* IS_TRUE_L */ 0, /* IS_FALSE_L */
	0, /* EQ_LL */ 2, /* EQ_LI */ 0, /* EQ_LN */ 0, /* EQ_LS */ 0, /* EQ_LP */
	0, /* EQ_LF */
	0, /* NEQ_LL */ 2, /* NEQ_LI */ 0, /* NEQ_LN */ 0, /* NEQ_LS */
	0, /* NEQ_LP */ 0, /* NEQ_LF */
	0, /* LT_LL */ 2, /* LT_LI */ 0, /* LT_LN */
	0, /* LE_LL */ 2, /* LE_LI */ 0, /* LE_LN */
	0, /* GT_LL */ 2, /* GT_LI */ 0, /* GT_LN */
	0, /* GE_LL */ 2, /* GE_LI */ 0, /* GE_LN */

	0, /* JMP */ 0, /* LOOP */
	0, /* CALL */ 0, /* RET0 */ 0, /* RET */

	0, /* STRUCT_NEW */ 0, /* STRUCT_CALL_CONSTRUCTOR */ 0, /* STRUCT_FIELD */
	0, /* STRUCT_SET_L */ 2, /* STRUCT_SET_I */ 0, /* STRUCT_SET_N */
	0, /* STRUCT_SET_S */ 0, /* STRUCT_SET_P */ 0, /* STRUCT_SET_F */
	0, /* STRUCT_SET_V */

	0, /* ARRAY_NEW */
	0, /* ARRAY_GET_L */ 0, /* ARRAY_GET_I */
	0, /* ARRAY_I_SET_L */ 2, /* ARRAY_I_SET_I */ 0, /* ARRAY_I_SET_N */
	0, /* ARRAY_I_SET_S */ 0, /* ARRAY_I_SET_P */ 0, /* ARRAY_I_SET_F */
	0, /* ARRAY_I_SET_V */
	0, /* ARRAY_L_SET_L */ 2, /* ARRAY_L_SET_I */ 0, /* ARRAY_L_SET_N */
	0, /* ARRAY_L_SET_S */ 0, /* ARRAY_L_SET_P */ 0, /* ARRAY_L_SET_F */
	0, /* ARRAY_L_SET_V */

	0, /* NO_OP */
};


// Return the number of digits in a number. Assumes the number is positive.
static int digits(int number) {
	int count = 0;
	while (number > 0) {
		count++;
		number /= 10;
	}
	return count;
}


// Print the file path and line number for a piece of source code.
static void print_location(HyState *state, Index src_index, uint32_t line) {
	Source *src = &vec_at(state->sources, src_index);
	if (src->file == NULL) {
		printf("<string>:%d", line);
	} else {
		printf("%s:%d", src->file, line);
	}
}


// Print an instruction's opcode.
static void print_opcode(Instruction ins) {
	// Find the length of the longest opcode
	uint32_t max_length = 0;
	for (uint32_t i = 0; i <= NO_OP; i++) {
		uint32_t length = strlen(opcode_names[i]);
		if (length > max_length) {
			max_length = length;
		}
	}

	// Align the name of the opcode to the length of the longest opcode's name
	BytecodeOpcode opcode = ins_arg(ins, 0);
	printf("%-*s  ", max_length, opcode_names[opcode]);
}


// Print an instruction's arguments.
static void print_arguments(Instruction ins) {
	BytecodeOpcode opcode = ins_arg(ins, 0);
	for (uint32_t i = 1; i <= argument_count[opcode]; i++) {
		if (i == integer_argument[opcode]) {
			// Convert to signed integer
			int16_t arg = unsigned_to_signed(ins_arg(ins, i));
			printf("%-5d", arg);
		} else {
			printf("%-5d", ins_arg(ins, i));
		}
	}
}


// Print a number, using an index into the interpreter state's constants list.
static void print_number(HyState *state, uint32_t index) {
	double value = val_to_num(vec_at(state->constants, index));
	printf("    ; %.15g", value);
}


// Print a string, using an index into the interpreter state's strings list.
static void print_string(HyState *state, uint32_t index) {
	char *str = &(vec_at(state->strings, index)->contents[0]);
	printf("    ; \"%s\"", str);
}


// Print useful information about the arguments to an instruction.
static void print_info(HyState *state, Index ins_index, Instruction ins) {
	BytecodeOpcode opcode = ins_arg(ins, 0);
	switch (opcode) {
		// Numbers (value we want is the second argument)
	case MOV_LN:
	case MOV_UN:
	case MOV_TN:
	case ADD_LN:
	case SUB_LN:
	case MUL_LN:
	case DIV_LN:
	case MOD_LN:
	case EQ_LN:
	case NEQ_LN:
	case LT_LN:
	case LE_LN:
	case GT_LN:
	case GE_LN:
	case STRUCT_SET_N:
		print_number(state, ins_arg(ins, 2));
		break;

		// Numbers (value we want is the first argument)
	case ADD_NL:
	case SUB_NL:
	case MUL_NL:
	case DIV_NL:
	case MOD_NL:
		print_number(state, ins_arg(ins, 1));
		break;

		// Strings (value is the second argument)
	case MOV_LS:
	case MOV_US:
	case MOV_TS:
	case EQ_LS:
	case NEQ_LS:
	case CONCAT_SL:
	case STRUCT_SET_S:
		print_string(state, ins_arg(ins, 2));
		break;

		// Strings (value is the third argument)
	case CONCAT_LS:
		print_string(state, ins_arg(ins, 3));
		break;

		// Function name
	case MOV_LF:
	case MOV_UF:
	case MOV_TF:
	case STRUCT_SET_F: {
		Function *fn = &vec_at(state->functions, ins_arg(ins, 2));
		printf("    ; ");
		print_location(state, fn->source, fn->line);
		break;
	}

		// Native function name
	case MOV_LV:
	case MOV_UV:
	case MOV_TV:
	case STRUCT_SET_V: {
		NativeFunction *native = &vec_at(state->native_fns, ins_arg(ins, 2));
		Package *pkg = &vec_at(state->packages, native->package);
		printf("    ; `%s.%s`", pkg->name, native->name);
		break;
	}

		// Jump destination
	case JMP:
		printf("    => %d", ins_index + ins_arg(ins, 1));
		break;
	case LOOP:
		// Convert instruction index to an integer so that in case there's a bug
		// in the parser, it's more obvious
		printf("    => %d", (int) ins_index - ins_arg(ins, 1));
		break;

		// Top level name
	case MOV_LT: {
		Package *pkg = &vec_at(state->packages, ins_arg(ins, 3));
		Identifier *name = &vec_at(pkg->names, ins_arg(ins, 2));
		printf("    ; ");

		// Package name
		if (pkg->name == NULL) {
			printf("<anonymous>.");
		} else {
			printf("%s.", pkg->name);
		}

		// Field name
		printf("%.*s", name->length, name->name);
		break;
	}

		// Field name
	case STRUCT_FIELD: {
		Identifier *field = &vec_at(state->fields, ins_arg(ins, 3));
		printf("    ; <%d>.%.*s", ins_arg(ins, 1), field->length, field->name);
		break;
	}

	default:
		break;
	}
}


// Pretty print an instruction within a function's bytecode to the standard
// output. The instruction index is used to calculate jump offsets.
void debug_ins(HyState *state, Function *fn, Index ins_index) {
	// Index
	printf("%.*d    ", digits(vec_len(fn->instructions) - 1), ins_index);

	// Opcode, arguments, and useful info
	Instruction ins = vec_at(fn->instructions, ins_index);
	print_opcode(ins);
	print_arguments(ins);
	print_info(state, ins_index, ins);

	// Final newline
	printf("\n");
}


// Pretty print the entire bytecode of a function to the standard output.
void debug_fn(HyState *state, Function *fn) {
	// File and line number
	print_location(state, fn->source, fn->line);

	// Name
	if (fn->name != NULL) {
		printf(": %.*s", fn->length, fn->name);
	} else {
		printf(": <anonymous>");
	}

	printf("\n");

	// Bytecode instructions
	for (uint32_t i = 0; i < vec_len(fn->instructions); i++) {
		debug_ins(state, fn, i);
	}
}


// Parse some source code and print the corresponding bytecode to the standard
// output.
static HyError * parse_and_print_bytecode(HyState *state, Index index,
		Index source) {
	Package *pkg = &vec_at(state->packages, index);

	// Newly defined functions will appear at the end of the functions array.
	// Save its length so we can print only the newly defined functions
	uint32_t functions_length = vec_len(state->functions);

	// Parse source code
	HyError *err = pkg_parse(pkg, source, NULL);
	if (err != NULL) {
		return err;
	}

	// Print new function definitions
	for (uint32_t i = functions_length; i < vec_len(state->functions); i++) {
		debug_fn(state, &vec_at(state->functions, i));
		if (i < vec_len(state->functions) - 1) {
			printf("\n");
		}
	}

	// No error can occur when printing debug information
	return NULL;
}


// Read source code from a file and parse it into bytecode, printing it to
// the standard output.
HyError * hy_print_bytecode_file(HyState *state, HyPackage pkg, char *path) {
	Index source = state_add_source_file(state, path);

	// Check we could open the file
	if (source == NOT_FOUND) {
		Error err = err_new(state);
		err_print(&err, "Failed to open file");
		err_file(&err, path);
		return err_make(&err);
	}

	return parse_and_print_bytecode(state, pkg, source);
}


// Parse source code into bytecode and print it to the standard output. An
// error object is returned if one occurred during parsing, otherwise NULL
// is returned.
HyError * hy_print_bytecode_string(HyState *state, HyPackage pkg, char *src) {
	Index source = state_add_source_string(state, src);
	return parse_and_print_bytecode(state, pkg, source);
}
