
//
//  Expression
//


#ifndef EXPRESSION_H
#define EXPRESSION_H


// Generates bytecode for evaluating an expression, leaving the
// resulting value on the top of the stack.
//
// Uses a Pratt parser to compile the expression. Triggers an
// error on the compiler if the expression fails to parse.
//
// The expression stops parsing when it reaches the terminator
// token. If the terminator is a new line token, and we could
// continue successfully parsing the expression on the next line,
// then parsing continues.
//
// The terminator token is not consumed.
void expression(Compiler *compiler, TokenType terminator);


#endif
