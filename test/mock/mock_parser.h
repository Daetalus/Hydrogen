
//
//  Mock Parser
//

#ifndef MOCK_PARSER_H
#define MOCK_PARSER_H

#include <hydrogen.h>
#include <parser.h>
#include <bytecode.h>


// A mock parser
typedef struct {
	// The compilation error, if one occurred, or NULL otherwise
	HyError *err;

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
void ins(MockParser *parser, BytecodeOpcode opcode, uint16_t arg1,
	uint16_t arg2, uint16_t arg3);

// Assert a jump instruction.
void jmp(MockParser *parser, uint16_t delta);

#endif
