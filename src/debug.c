
//
//  Debug
//


#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "lexer.h"
#include "value.h"


// Pretty prints out the contents of a bytecode array.
void pretty_print_bytecode(Bytecode *bytecode) {
	uint8_t *ip = bytecode->instructions;
	uint8_t *first = bytecode->instructions;
	uint8_t *last = &bytecode->instructions[bytecode->count];

	while (ip < last) {
		long position = ip - first;
		uint8_t instruction = READ_BYTE();

		switch(instruction) {
		case CODE_PUSH_NUMBER: {
			uint64_t number = READ_8_BYTES();
			printf("%lu: push number %.3f\n", position, as_number(number));
			break;
		}

		case CODE_PUSH_STRING: {
			uint64_t pointer = READ_8_BYTES();
			uint32_t length = READ_4_BYTES();
			char *str = (char *) pointer;
			printf("%lu: push string %.*s\n", position, length, str);
			break;
		}

		case CODE_PUSH_VARIABLE: {
			uint16_t index = READ_2_BYTES();
			printf("%lu: push variable %hu\n", position, index);
			break;
		}

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

		case CODE_JUMP_BACKWARD: {
			uint16_t amount = READ_2_BYTES();
			printf("%lu: jump -%hu\n", position, amount);
			break;
		}

		case CODE_CONDITIONAL_JUMP: {
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
		}
	}
}
