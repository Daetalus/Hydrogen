
//
//  Expression
//


#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdbool.h>

#include "compiler.h"


// The function definition for an expression terminator.
typedef bool (*ExpressionTerminator)(Token token);


// Generates bytecode to evaluate an expression parsed from the
// compiler's lexer. Leaves the result of the expression on the
// top of the stack.
//
// Stops parsing when `terminator` returns true. If `terminator`
// is NULL, then terminates the expression at a newline.
//
// Triggers an error if the expression fails to parse.
void expression(Compiler *compiler, ExpressionTerminator terminator);


#endif
