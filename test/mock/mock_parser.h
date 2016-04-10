
//
//  Mock Parser
//

#ifndef MOCK_PARSER_H
#define MOCK_PARSER_H

#include <hydrogen.h>
#include <parser.h>
#include <bytecode.h>
#include <fn.h>
#include <vm.h>


// A mock parser
typedef struct {
	// The interpreter state
	HyState *state;

	// The current function we're testing instructions on
	Index fn;

	// The index of the next instruction to assert
	Index ins;
} MockParser;


// Creates a new parser to run tests on
MockParser mock_parser(char *code);

// Frees a mock parser
void mock_parser_free(MockParser *parser);

// Switch to testing a different function.
void switch_fn(MockParser *parser, Index fn);


// Assert the opcode and arguments of an instruction.
#define ins(parser, opcode, arg1, arg2, arg3) {                       \
	Function *fn = &vec_at((parser)->state->functions, (parser)->fn); \
	lt_int((parser)->ins, vec_len(fn->instructions));                 \
                                                                      \
	Instruction ins = vec_at(fn->instructions, (parser)->ins++);      \
	eq_int(ins_arg(ins, 0), opcode);                                  \
	eq_int(ins_arg(ins, 1), arg1);                                    \
	eq_int(ins_arg(ins, 2), arg2);                                    \
	eq_int(ins_arg(ins, 3), arg3);                                    \
}


// Assert a jump instruction.
#define jmp(parser, offset) {                                         \
	Function *fn = &vec_at((parser)->state->functions, (parser)->fn); \
	lt_int((parser)->ins, vec_len(fn->instructions));                 \
                                                                      \
	Instruction ins = vec_at(fn->instructions, (parser)->ins++);      \
	eq_int(ins_arg(ins, 0), JMP);                                     \
	eq_int(ins_arg(ins, 1), offset);                                  \
}

#endif
