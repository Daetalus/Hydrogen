
//
//  Operators
//


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../error.h"
#include "operator.h"


// Pops an argument from the stack.
#define POP() ((*stack_size)--, stack[*stack_size])


// Pushes an argument onto the stack.
#define PUSH(value) stack[(*stack_size)++] = (value)


// Pops a numerical argument from the stack, triggering
// a runtime error if it isn't a number.
#define POP_NUMBER(name)              \
	uint64_t value_##name = POP();    \
	if (!IS_NUMBER(value_##name)) {   \
		error(-1, "Expected number"); \
	}                                 \
	double name = value_to_number(value_##name);


// Pushes a number by converting it to a value.
#define PUSH_NUMBER(value) PUSH(number_to_value((value)))



//
//  Testing Print Statements
//

void native_print(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	uint64_t arg = POP();

	if (IS_PTR(arg)) {
		String *string = value_to_ptr(arg);
		printf("%s\n", string->contents);
	} else if (IS_NUMBER(arg)) {
		printf("%.2f\n", value_to_number(arg));
	} else if (IS_TRUE(arg)) {
		printf("true\n");
	} else if (IS_FALSE(arg)) {
		printf("false\n");
	} else if (IS_NIL(arg)) {
		printf("nil\n");
	} else {
		error(-1, "Unexpected argument to `print`");
	}
}


void native_print_2(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	// Pop our 2 arguments
	POP();
	POP();
}


void native_assert(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	uint64_t arg = POP();

	if (IS_FALSE(arg) || IS_NIL(arg)) {
		// Exit forcefully
		error(-1, "Assertion failed.");
	}
}



//
//  Mathematical Operators
//

void operator_addition(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	uint64_t right = POP();
	uint64_t left = POP();

	if (IS_NUMBER(left) && IS_NUMBER(right)) {
		// Add to numbers
		PUSH_NUMBER(value_to_number(left) + value_to_number(right));
	} else if (IS_PTR(left) && IS_NUMBER(right)) {
		// Concatenate a string and number
		String *left_str = value_to_ptr(left);
		double right_number = value_to_number(right);
		size_t size = left_str->length + 50;
		String *result = string_new(size);

		int length = sprintf(result->contents, "%s%.2f", left_str->contents,
			right_number);
		result->length = length;

		PUSH(ptr_to_value(result));
	} else if (IS_NUMBER(left) && IS_PTR(right)) {
		// Concatenate a number and string
		double left_number = value_to_number(left);
		String *right_str = value_to_ptr(right);
		size_t size = right_str->length + 50;
		String *result = string_new(size);

		int length = sprintf(result->contents, "%.2f%s", left_number,
			right_str->contents);
		result->length = length;

		PUSH(ptr_to_value(result));
	} else if (IS_PTR(left) && IS_PTR(right)) {
		// Concatenate two strings
		String *left_str = value_to_ptr(left);
		String *right_str = value_to_ptr(right);
		String *result = string_concat(left_str, right_str);
		PUSH(ptr_to_value(result));
	} else {
		error(-1, "Expected string or number");
	}
}


void operator_subtraction(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);
	PUSH_NUMBER(left - right);
}


void operator_multiplication(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	// TODO: implement string multiplication
	POP_NUMBER(right);
	POP_NUMBER(left);
	PUSH_NUMBER(left * right);
}


void operator_division(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);
	PUSH_NUMBER(left / right);
}


void operator_modulo(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);
	PUSH_NUMBER(fmod(left, right));
}


void operator_negation(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	POP_NUMBER(operand);
	PUSH_NUMBER(-operand);
}



//
//  Boolean Operators
//

void operator_boolean_and(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	uint64_t right = POP();
	uint64_t left = POP();

	if (IS_FALSE(left) || IS_NIL(left) || IS_FALSE(right) || IS_NIL(right)) {
		PUSH(FALSE_VALUE);
	} else {
		PUSH(TRUE_VALUE);
	}
}


void operator_boolean_or(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	uint64_t right = POP();
	uint64_t left = POP();

	if ((IS_FALSE(left) || IS_NIL(left)) &&
			(IS_FALSE(right) || IS_NIL(right))) {
		PUSH(FALSE_VALUE);
	} else {
		PUSH(TRUE_VALUE);
	}
}


void operator_boolean_not(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	uint64_t argument = POP();

	if (IS_FALSE(argument) || IS_NIL(argument)) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}


// Pops the arguments to an equal or not equal operator and
// returns the result as a boolean.
bool are_equal(uint64_t *stack, int *stack_size) {
	uint64_t right = POP();
	uint64_t left = POP();

	if (IS_TRUE(left) && IS_TRUE(right)) {
		return true;
	} else if (IS_FALSE(left) && IS_FALSE(right)) {
		return true;
	} else if (IS_NIL(left) && IS_NIL(right)) {
		return true;
	} else if (IS_NUMBER(left) && IS_NUMBER(right)) {
		if (value_to_number(left) == value_to_number(right)) {
			return true;
		} else {
			return false;
		}
	} else if (IS_PTR(left) && IS_PTR(right)) {
		// Compare strings
		String *left_str = value_to_ptr(left);
		String *right_str = value_to_ptr(right);

		if (left_str->length == right_str->length &&
				strcmp(left_str->contents, right_str->contents) == 0) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}


void operator_equal(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	if (are_equal(stack, stack_size)) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}


void operator_not_equal(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	if (are_equal(stack, stack_size)) {
		PUSH(FALSE_VALUE);
	} else {
		PUSH(TRUE_VALUE);
	}
}


void operator_less_than(VirtualMachine *vm, uint64_t *stack, int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);

	if (left < right) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}


void operator_less_than_equal_to(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);

	if (left <= right) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}


void operator_greater_than(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);

	if (left > right) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}


void operator_greater_than_equal_to(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {
	POP_NUMBER(right);
	POP_NUMBER(left);

	if (left >= right) {
		PUSH(TRUE_VALUE);
	} else {
		PUSH(FALSE_VALUE);
	}
}



//
//  Bitwise Operators
//

void operator_left_shift(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}


void operator_right_shift(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}


void operator_bitwise_and(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}


void operator_bitwise_or(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}


void operator_bitwise_not(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}


void operator_bitwise_xor(VirtualMachine *vm, uint64_t *stack,
		int *stack_size) {

}
