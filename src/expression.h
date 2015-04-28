
//
//  Expression
//


#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdbool.h>

#include "compiler.h"


// The function definition for an expression terminator.
typedef bool (*ExpressionTerminator)(Token token);


// An expression struct, storing the information needed to
// compile an expression.
typedef struct {
	// The compiler that invoked this expression parser.
	Compiler *compiler;

	// A function that returns true when the expression should
	// be terminated.
	ExpressionTerminator terminator;

	// Whether this expression only contains a function call,
	// used when compiling an expression statement (which can
	// only consist of a function call).
	bool is_only_function_call;
} Expression;


// Create a new expression.
Expression expression_new(Compiler *compiler,
	ExpressionTerminator terminator);

// Generates bytecode to evaluate an expression parsed from the
// compiler's lexer. Leaves the result of the expression on the
// top of the stack.
//
// Stops parsing when `terminator` returns true. If `terminator`
// is NULL, then terminates the expression at a newline.
//
// Triggers an error if the expression fails to parse.
void expression_compile(Expression *expression);

#endif
