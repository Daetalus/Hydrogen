
//
//  Virtual Machine
//


#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "vm.h"
#include "operators.h"
#include "bytecode.h"
#include "compiler.h"
#include "debug.h"


// Create a new virtual machine with the given source code.
// Initializes the VM with default values, and doesn't compile
// anything.
VirtualMachine vm_new(char *source) {
	VirtualMachine vm;
	lexer_new(&vm.lexer, source);
	vm.source = source;
	vm.function_count = 0;
	vm.literal_count = 0;
	return vm;
}


// Free any resources allocated by the VM.
void vm_free(VirtualMachine *vm) {
	// Free functions
	for (int i = 0; i < vm->function_count; i++) {
		free(&vm->functions[i].bytecode);
	}

	// Free string literals
	for (int i = 0; i < vm->literal_count; i++) {
		string_free(vm->literals[i]);
	}
}


// Compiles the source code into bytecode.
void vm_compile(VirtualMachine *vm) {
	// Create the main function, whose bytecode we'll populate.
	// The main function is identified by the NULL name value.
	Function *fn = define_bytecode_function(vm);
	fn->name = "main";
	fn->length = 4;
	fn->argument_count = 0;
	bytecode_new(&fn->bytecode, DEFAULT_INSTRUCTION_CAPACITY);

	// Compile the source code into the function's
	// bytecode array.
	compile(vm, fn, TOKEN_END_OF_FILE);
}



//
//  Function Definitions
//

// Defines a new function on the virtual machine, returning
// a pointer to it.
// Performs no allocation, so the bytecode array for the
// returned function still needs to be allocated.
Function * define_bytecode_function(VirtualMachine *vm) {
	vm->function_count++;
	return &vm->functions[vm->function_count - 1];
}


// Returns the index of a function with the given name and
// name length.
// Returns -1 if the function isn't found.
int find_function(
		VirtualMachine *vm,
		char *name,
		int length,
		int argument_count) {
	for (int i = 0; i < vm->function_count; i++) {
		Function *fn = &vm->functions[i];

		if (fn->length == length && fn->argument_count == argument_count &&
				strncmp(fn->name, name, length) == 0) {
			return i;
		}
	}

	return -1;
}


// Returns a function pointer to a library function with the
// given name and length.
// Returns NULL if the function isn't found.
NativeFunction find_native_function(
		VirtualMachine *vm,
		char *name,
		int length,
		int argument_count) {
	if (strncmp(name, "print", length) == 0) {
		if (argument_count == 1) {
			return &native_print;
		} else if (argument_count == 2) {
			return &native_print_2;
		}
	} else if (strncmp(name, "assert", length) == 0) {
		return &native_assert;
	}

	return NULL;
}



//
//  Errors
//

// Trigger a runtime error on the virtual machine.
void vm_crash(VirtualMachine *vm, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED BOLD "error: " WHITE);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n" NORMAL);

	va_end(args);

	// Halt the program
	exit(0);
}



//
//  Execution
//

// The maximum value stack size.
#define MAX_STACK_SIZE 2048

// The maximum call stack size.
#define MAX_CALL_STACK_SIZE 1024


// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm) {
	// The value stack which the program pushes to.
	uint64_t stack[MAX_STACK_SIZE];
	int stack_size = 0;

	// The function call frame stack, which maintains
	// which functions are being executed.
	CallFrame call_stack[MAX_CALL_STACK_SIZE];
	int call_stack_size = 0;

	// The currently executing instruction (ip stands for instruction
	// pointer).
	uint8_t *ip = NULL;

	// The stack pointer of the top most call frame, pointing
	// to a place on the stack where the function's variables
	// start.
	uint64_t *stack_ptr = NULL;

	// Push a value onto the top of the stack.
	#define PUSH(value) stack[stack_size++] = (value);

	// Pop a value from the top of the stack.
	#define POP() stack_size--;

	// Evaluates to the value on the top of the stack.
	#define TOP() stack[stack_size - 1]

	// Pushes a new call frame for `fn` onto the call frame stack.
	// Updates `ip` and `stack_ptr` with the values for the new function.
	#define PUSH_FRAME(fn) {                                         \
		if (call_stack_size > 0) {                                   \
			call_stack[call_stack_size - 1].instruction_ptr = ip;    \
		}                                                            \
		ip = (fn)->bytecode.instructions;                            \
		stack_ptr = (&stack[stack_size - 1]) - (fn)->argument_count; \
		call_stack[call_stack_size].stack_ptr = stack_ptr;           \
		call_stack[call_stack_size].instruction_ptr = ip;            \
		call_stack_size++;                                           \
	}

	// Returns from the executing function to the function that called it.
	#define POP_FRAME()                                       \
		call_stack_size--;                                    \
		ip = call_stack[call_stack_size - 1].instruction_ptr; \
		stack_ptr = call_stack[call_stack_size - 1].stack_ptr;


	// Push the main function onto the call stack as the very first
	// executing function.
	PUSH_FRAME(&vm->functions[0]);

	// Begin execution
instructions:
	switch (READ_BYTE()) {
	case CODE_PUSH_NUMBER:
		PUSH(READ_8_BYTES());
		goto instructions;

	case CODE_PUSH_STRING: {
		uint16_t index = READ_2_BYTES();
		String *copy = string_duplicate(vm->literals[index]);
		PUSH(ptr_to_value(copy));
		goto instructions;
	}

	case CODE_PUSH_TRUE:
		PUSH(TRUE_VALUE)
		goto instructions;

	case CODE_PUSH_FALSE:
		PUSH(FALSE_VALUE)
		goto instructions;

	case CODE_PUSH_NIL:
		PUSH(NIL_VALUE)
		goto instructions;

	case CODE_PUSH_VARIABLE: {
		uint16_t index = READ_2_BYTES();
		PUSH(stack_ptr[index]);
		goto instructions;
	}

	case CODE_POP:
		POP();
		goto instructions;

	case CODE_STORE: {
		uint16_t index = READ_2_BYTES();
		stack_ptr[index] = TOP();
		if (stack_size - 1 > index) {
			POP();
		}
		goto instructions;
	}

	case CODE_JUMP_FORWARD: {
		uint16_t amount = READ_2_BYTES();
		ip += amount;
		goto instructions;
	}

	case CODE_JUMP_BACK: {
		uint16_t amount = READ_2_BYTES();
		ip -= amount;
		goto instructions;
	}

	case CODE_JUMP_IF_NOT:
		if (IS_NIL(TOP()) || IS_FALSE(TOP())) {
			uint16_t amount = READ_2_BYTES();
			ip += amount;
		} else {
			// Discard the two bytes
			READ_2_BYTES();
		}

		// Discard the conditional expression result
		POP();
		goto instructions;

	case CODE_CALL: {
		uint16_t index = READ_2_BYTES();
		PUSH_FRAME(&vm->functions[index]);
		goto instructions;
	}

	case CODE_CALL_NATIVE: {
		NativeFunction fn = value_to_ptr(READ_8_BYTES());
		fn(vm, stack, &stack_size);
		goto instructions;
	}

	case CODE_RETURN:
		if (call_stack_size == 1) {
			break;
		}
		POP_FRAME();
		goto instructions;
	}
}
