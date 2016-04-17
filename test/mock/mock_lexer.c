
//
//  Mock Lexer
//

#include "mock_lexer.h"


// Creates a new lexer to run tests on.
Lexer mock_lexer(char *code) {
	HyState *state = hy_new();
	Index source = state_add_source_string(state, code);
	Lexer lexer = lexer_new(state, source);
	return lexer;
}


// Frees a mock lexer.
void mock_lexer_free(Lexer *lexer) {
	hy_free(lexer->state);
}
