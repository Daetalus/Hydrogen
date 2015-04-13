
//
//  Debug
//


#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"
#include "value.h"
#include "debug.h"


// Pretty prints out the contents of a bytecode array.
void print_bytecode(Bytecode *bytecode) {
	uint8_t *ip = bytecode->instructions;
	uint8_t *first = bytecode->instructions;
	uint8_t *last = &bytecode->instructions[bytecode->count - 1];

	while (ip <= last) {
		long position = ip - first;
		ip = print_instruction(ip, position);
	}
}


// Prints an instruction.
uint8_t * print_instruction(uint8_t *ip, long position) {
	uint8_t instruction = READ_BYTE();

	switch(instruction) {
	case CODE_PUSH_NUMBER: {
		uint64_t number = READ_8_BYTES();
		printf("%lu: push number %.3f\n", position, value_to_number(number));
		break;
	}

	case CODE_PUSH_STRING: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push string %d\n", position, index);
		break;
	}

	case CODE_PUSH_LOCAL: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push variable %hu\n", position, index);
		break;
	}

	case CODE_PUSH_TRUE:
		printf("%lu: push true\n", position);
		break;

	case CODE_PUSH_FALSE:
		printf("%lu: push false\n", position);
		break;

	case CODE_PUSH_NIL:
		printf("%lu: push nil\n", position);
		break;

	case CODE_POP: {
		printf("%lu: pop\n", position);
		break;
	}

	case CODE_STORE: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: store %hu\n", position, index);
		break;
	}

	case CODE_JUMP_FORWARD: {
		uint16_t amount = READ_2_BYTES();
		printf("%lu: jump %hu\n", position, amount);
		break;
	}

	case CODE_JUMP_BACK: {
		uint16_t amount = READ_2_BYTES();
		printf("%lu: jump -%hu\n", position, amount);
		break;
	}

	case CODE_JUMP_IF_NOT: {
		uint16_t amount = READ_2_BYTES();
		printf("%lu: jump if %hu\n", position, amount);
		break;
	}

	case CODE_CALL: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: call %hu\n", position, index);
		break;
	}

	case CODE_CALL_NATIVE: {
		uint64_t pointer = READ_8_BYTES();
		printf("%lu: call native %p\n", position, (void *) pointer);
		break;
	}

	case CODE_RETURN: {
		printf("%lu: return\n", position);
		break;
	}

	default:
		printf("Unrecognised instruction %d\n", instruction);
		break;
	}

	return ip;
}


// Pretty prints out the contents of the stack.
void print_stack(uint64_t *stack, int stack_size) {
	printf("---------- Stack:\n");
	for (int i = 0; i < stack_size; i++) {
		printf("%d: %llu, %.2f, is true: %d, is false: %d, is nil: %d\n",
			i, stack[i], value_to_number(stack[i]),
			IS_TRUE(stack[i]), IS_FALSE(stack[i]), IS_NIL(stack[i]));
	}
	printf("----------\n");
}
