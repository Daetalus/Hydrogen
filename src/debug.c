
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
	uint8_t *cursor = bytecode->instructions;
	uint8_t *last = &bytecode->instructions[bytecode->count];

	#define READ_BYTE() \
		(cursor++, (uint8_t) (*(cursor - 0)))

	#define READ_2_BYTES() \
		(cursor += 2, ((uint16_t) *(cursor - 0) << (1 << 3)) | *(cursor - 1))

	#define READ_4_BYTES() \
		(cursor += 4, \
			((uint32_t) *(cursor - 0) << (3 << 3)) | \
			((uint32_t) *(cursor - 1) << (2 << 3)) | \
			((uint32_t) *(cursor - 2) << (1 << 3)) | \
			(uint32_t) (*(cursor - 3)))

	#define READ_8_BYTES() \
		(cursor += 8, \
			((uint64_t) *(cursor - 0) << (7 << 3)) | \
			((uint64_t) *(cursor - 1) << (6 << 3)) | \
			((uint64_t) *(cursor - 2) << (5 << 3)) | \
			((uint64_t) *(cursor - 3) << (4 << 3)) | \
			((uint64_t) *(cursor - 4) << (3 << 3)) | \
			((uint64_t) *(cursor - 5) << (2 << 3)) | \
			((uint64_t) *(cursor - 6) << (1 << 3)) | \
			((uint64_t) *(cursor - 7)))

	while (cursor != last) {
		uint8_t instruction = READ_BYTE();

		switch(instruction) {
		case CODE_PUSH_NUMBER: {
			uint64_t number = READ_8_BYTES();
			printf("push number %.3f\n", as_number(number));
			break;
		}

		case CODE_PUSH_STRING: {
			uint64_t pointer = READ_8_BYTES();
			uint32_t length = READ_4_BYTES();
			char *str = (char *) pointer;
			printf("push string %.*s\n", length, str);
			break;
		}

		case CODE_PUSH_VARIABLE: {
			uint16_t index = READ_2_BYTES();
			printf("push variable %hu\n", index);
			break;
		}

		case CODE_POP: {
			printf("pop\n");
			break;
		}

		case CODE_STORE: {
			uint16_t index = READ_2_BYTES();
			printf("store %hu\n", index);
			break;
		}

		case CODE_JUMP_FORWARD: {
			uint16_t amount = READ_2_BYTES();
			printf("jump %hu\n", amount);
			break;
		}

		case CODE_JUMP_BACKWARD: {
			uint16_t amount = READ_2_BYTES();
			printf("jump -%hu\n", amount);
			break;
		}

		case CODE_CONDITIONAL_JUMP: {
			uint16_t amount = READ_2_BYTES();
			printf("jump if %hu\n", amount);
			break;
		}

		case CODE_CALL: {
			uint16_t index = READ_2_BYTES();
			printf("call %hu\n", index);
			break;
		}

		case CODE_CALL_NATIVE: {
			uint64_t pointer = READ_8_BYTES();
			printf("call native %p\n", (void *) pointer);
			break;
		}
		}
	}
}
