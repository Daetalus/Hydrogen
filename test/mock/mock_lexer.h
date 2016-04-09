
//
//  Mock Lexer
//

#ifndef MOCK_LEXER_H
#define MOCK_LEXER_H

#include <hydrogen.h>
#include <vm.h>
#include <lexer.h>

// Frees a mock lexer.
void mock_lexer_free(Lexer *lexer);

// Creates a new lexer to run tests on.
Lexer mock_lexer(char *code);

#endif
