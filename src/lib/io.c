
//
//  IO Library
//


#include <stdio.h>

#include "io.h"
#include "lib.h"
#include "str.h"
#include "../value.h"
#include "../error.h"
#include "../debug.h"


void native_print(uint64_t *stack, int *stack_size) {
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
		printf("unexpected: %llx\n", arg);
		error(-1, "Unexpected argument to `print`");
	}

	PUSH(NIL_VALUE);
}


void native_print_2(uint64_t *stack, int *stack_size) {
	// Pop our 2 arguments
	POP();
	POP();
	PUSH(NIL_VALUE);
}


void native_assert(uint64_t *stack, int *stack_size) {
	uint64_t arg = POP();

	if (IS_FALSE(arg) || IS_NIL(arg)) {
		// Exit forcefully
		error(-1, "Assertion failed.");
	}

	PUSH(NIL_VALUE);
}


void native_print_stack(uint64_t *stack, int *stack_size) {
	print_stack(stack, *stack_size);
	PUSH(NIL_VALUE);
}
