
//
//  Operators
//


#ifndef OPERATORS_H
#define OPERATORS_H

#include "lexer.h"


// Returns true if the given token is a binary operator.
bool is_binary_operator(TokenType operator);

// Returns the precedence of an operator.
int operator_precedence(TokenType operator);

// Returns the associativity of an operator.
Associativity operator_associativity(TokenType operator);


// Mathematical operators
void operator_addition(void);
void operator_subtraction(void);
void operator_multiplication(void);
void operator_division(void);
void operator_modulo(void);
void operator_negation(void);

// Boolean operators
void operator_boolean_and(void);
void operator_boolean_or(void);
void operator_boolean_not(void);
void operator_equal(void);
void operator_not_equal(void);
void operator_less_than(void);
void operator_less_than_equal_to(void);
void operator_greater_than(void);
void operator_greater_than_equal_to(void);

// Bitwise operators
void operator_left_shift(void);
void operator_right_shift(void);
void operator_bitwise_and(void);
void operator_bitwise_or(void);
void operator_bitwise_not(void);
void operator_bitwise_xor(void);

#endif
