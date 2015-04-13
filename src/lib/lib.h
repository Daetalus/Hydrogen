
//
//  Library Utilities
//


#ifndef LIB_H
#define LIB_H

// Pops a value from the top of the stack.
#define POP() ((*stack_size)--, stack[*stack_size])


// Pushes a value onto the stack.
#define PUSH(value) stack[(*stack_size)++] = (value)


// Pops a numerical argument from the stack, triggering a runtime
// error if the popped value isn't a number.
#define POP_NUMBER(name)              \
	uint64_t value_##name = POP();    \
	if (!IS_NUMBER(value_##name)) {   \
		error(-1, "Expected number"); \
	}                                 \
	double name = value_to_number(value_##name);


// Pushes a number onto the top of the stack.
#define PUSH_NUMBER(number) PUSH(number_to_value((number)))

#endif
