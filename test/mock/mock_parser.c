
//
//  Mock Parser
//

#include <vm.h>
#include <pkg.h>
#include <bytecode.h>

#include <test.h>

#include "mock_parser.h"


// Triggers a test error if a compiler error occurred.
void check_err(HyError *err) {
	if (err == NULL) {
		return;
	}

	printf("\nCompilation error!\n");
	printf("%s\n", err->description);
	printf("Line: %d\n", err->line);
	printf("Column: %d\n", err->column);
	hy_err_free(err);
	trigger();
}


// Creates a new parser to run tests on.
MockParser mock_parser(char *code) {
	MockParser parser;
	parser.state = hy_new();
	Index pkg_index = pkg_new(parser.state);
	Index source = state_add_source_string(parser.state, code);
	Package *pkg = &vec_at(parser.state->packages, pkg_index);

	HyError *err = pkg_parse(pkg, source, &parser.fn);
	check_err(err);
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
	parser->ins = 0;
}
