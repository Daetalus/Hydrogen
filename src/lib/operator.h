
//
//  Operators
//


#ifndef OPERATORS_H
#define OPERATORS_H

#include "../vm.h"


// Defines a native function.
#define DEFINE_NATIVE_FUNCTION(name) \
	void name(VirtualMachine *vm, uint64_t *stack, int *stack_size);


// Testing print statements
DEFINE_NATIVE_FUNCTION(native_print);
DEFINE_NATIVE_FUNCTION(native_print_2);
DEFINE_NATIVE_FUNCTION(native_assert);

// Mathematical operators
DEFINE_NATIVE_FUNCTION(operator_addition);
DEFINE_NATIVE_FUNCTION(operator_subtraction);
DEFINE_NATIVE_FUNCTION(operator_multiplication);
DEFINE_NATIVE_FUNCTION(operator_division);
DEFINE_NATIVE_FUNCTION(operator_modulo);
DEFINE_NATIVE_FUNCTION(operator_negation);

// Boolean operators
DEFINE_NATIVE_FUNCTION(operator_boolean_and);
DEFINE_NATIVE_FUNCTION(operator_boolean_or);
DEFINE_NATIVE_FUNCTION(operator_boolean_not);
DEFINE_NATIVE_FUNCTION(operator_equal);
DEFINE_NATIVE_FUNCTION(operator_not_equal);
DEFINE_NATIVE_FUNCTION(operator_less_than);
DEFINE_NATIVE_FUNCTION(operator_less_than_equal_to);
DEFINE_NATIVE_FUNCTION(operator_greater_than);
DEFINE_NATIVE_FUNCTION(operator_greater_than_equal_to);

// Bitwise operators
DEFINE_NATIVE_FUNCTION(operator_left_shift);
DEFINE_NATIVE_FUNCTION(operator_right_shift);
DEFINE_NATIVE_FUNCTION(operator_bitwise_and);
DEFINE_NATIVE_FUNCTION(operator_bitwise_or);
DEFINE_NATIVE_FUNCTION(operator_bitwise_not);
DEFINE_NATIVE_FUNCTION(operator_bitwise_xor);

#endif
