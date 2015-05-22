
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

	case CODE_PUSH_TRUE:
		printf("%lu: push true\n", position);
		break;

	case CODE_PUSH_FALSE:
		printf("%lu: push false\n", position);
		break;

	case CODE_PUSH_NIL:
		printf("%lu: push nil\n", position);
		break;

	case CODE_PUSH_LOCAL: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push local %hu\n", position, index);
		break;
	}

	case CODE_PUSH_NATIVE: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push native %hu\n", position, index);
		break;
	}

	case CODE_PUSH_FUNCTION: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push function %hu\n", position, index);
		break;
	}

	case CODE_PUSH_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: push upvalue %hu\n", position, index);
		break;
	}

	case CODE_PUSH_FIELD: {
		uint16_t length = READ_2_BYTES();
		char *name = value_to_ptr(READ_8_BYTES());
		printf("%lu: push field `%.*s`\n", position, length, name);
		break;
	}

	case CODE_PUSH_RECEIVER: {
		printf("%lu: push self\n", position);
		break;
	}

	case CODE_POP: {
		printf("%lu: pop\n", position);
		break;
	}

	case CODE_STORE_LOCAL: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: store local %hu\n", position, index);
		break;
	}

	case CODE_STORE_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: store upvalue %hu\n", position, index);
		break;
	}

	case CODE_CLOSE_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: close upvalue %hu\n", position, index);
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
		uint16_t arity = READ_2_BYTES();
		printf("%lu: call with arity %hu\n", position, arity);
		break;
	}

	case CODE_CALL_NATIVE: {
		uint64_t ptr = READ_8_BYTES();
		printf("%lu: call native %p\n", position, value_to_ptr(ptr));
		break;
	}

	case CODE_INSTANTIATE_CLASS: {
		uint16_t index = READ_2_BYTES();
		printf("%lu: instantiate class %d\n", position, index);
		break;
	}

	case CODE_RETURN: {
		printf("%lu: return\n", position);
		break;
	}

	default:
		printf("Unrecognised instruction %d\n", (int) instruction);
		break;
	}

	return ip;
}


// Pretty prints out the contents of the stack.
void print_stack(uint64_t *stack, int stack_size) {
	printf("---------- Stack:\n");
	for (int i = 0; i < stack_size; i++) {
		printf("%d: %llx, %.2f, is ptr: %d, is true: %d, is false: %d, "
			"is nil: %d, is function %d, is native: %d\n", i, stack[i],
			value_to_number(stack[i]), IS_PTR(stack[i]), IS_TRUE(stack[i]),
			IS_FALSE(stack[i]), IS_NIL(stack[i]), IS_NATIVE(stack[i]),
			IS_FUNCTION(stack[i]));
	}
	printf("----------\n");
}


// Pretty prints out the virtual machine's upvalues.
void print_upvalues(VirtualMachine *vm) {
	printf("Upvalues:\n");
	for (int i = 0; i < vm->upvalue_count; i++) {
		Upvalue *upvalue = &vm->upvalues[i];
		printf("  %d: closed: %d, value: %llx, stack pos: %d\n", i,
			upvalue->closed,
			upvalue->value,
			upvalue->local_index + upvalue->function_index);
	}
}
