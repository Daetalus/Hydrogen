
//
//  Operators
//


#ifndef OPERATORS_H
#define OPERATORS_H

#include <stdlib.h>

// Mathematical operators
void operator_addition(uint64_t *stack, int *stack_size);
void operator_subtraction(uint64_t *stack, int *stack_size);
void operator_multiplication(uint64_t *stack, int *stack_size);
void operator_division(uint64_t *stack, int *stack_size);
void operator_modulo(uint64_t *stack, int *stack_size);
void operator_negation(uint64_t *stack, int *stack_size);

// Boolean operators
void operator_boolean_and(uint64_t *stack, int *stack_size);
void operator_boolean_or(uint64_t *stack, int *stack_size);
void operator_boolean_not(uint64_t *stack, int *stack_size);
void operator_equal(uint64_t *stack, int *stack_size);
void operator_not_equal(uint64_t *stack, int *stack_size);
void operator_less_than(uint64_t *stack, int *stack_size);
void operator_less_than_equal_to(uint64_t *stack, int *stack_size);
void operator_greater_than(uint64_t *stack, int *stack_size);
void operator_greater_than_equal_to(uint64_t *stack, int *stack_size);

// Bitwise operators
void operator_left_shift(uint64_t *stack, int *stack_size);
void operator_right_shift(uint64_t *stack, int *stack_size);
void operator_bitwise_and(uint64_t *stack, int *stack_size);
void operator_bitwise_or(uint64_t *stack, int *stack_size);
void operator_bitwise_not(uint64_t *stack, int *stack_size);
void operator_bitwise_xor(uint64_t *stack, int *stack_size);

#endif
