
//
//  Mock Parser
//

#include <vm.h>
#include <pkg.h>
#include <bytecode.h>

#include <test.h>

#include "mock_parser.h"


// Creates a new parser to run tests on.
MockParser mock_parser(char *code) {
	MockParser parser;
	parser.state = hy_new();
	Index pkg_index = pkg_new(parser.state);
	Index source = state_add_source_string(parser.state, code);
	Package *pkg = &vec_at(parser.state->packages, pkg_index);

	parser.err = pkg_parse(pkg, source, &parser.fn);
	parser.ins = 0;
	return parser;
}


// Frees a mock parser.
void mock_parser_free(MockParser *parser) {
	hy_free(parser->state);
}


// Switch to testing a different function.
void switch_fn(MockParser *parser, Index fn) {
	parser->fn = fn;
}


// Assert the opcode and arguments of an instruction.
void ins(MockParser *parser, BytecodeOpcode opcode, uint16_t arg1,
		uint16_t arg2, uint16_t arg3) {
	Function *fn = &vec_at(parser->state->functions, parser->fn);
	lt_int(parser->ins, vec_len(fn->instructions));

	Instruction ins = vec_at(fn->instructions, parser->ins++);
	eq_int(ins_arg(ins, 0), opcode);
	eq_int(ins_arg(ins, 1), arg1);
	eq_int(ins_arg(ins, 2), arg2);
	eq_int(ins_arg(ins, 3), arg3);
}


// Assert a jump instruction.
void jmp(MockParser *parser, uint16_t offset) {
	Function *fn = &vec_at(parser->state->functions, parser->fn);
	lt_int(parser->ins, vec_len(fn->instructions));

	Instruction ins = vec_at(fn->instructions, parser->ins++);
	eq_int(ins_arg(ins, 0), JMP);
	eq_int(ins_arg(ins, 1), offset);
}
