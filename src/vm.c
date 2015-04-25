
//
//  Virtual Machine
//


#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "lib/operator.h"
#include "lib/io.h"
#include "compiler.h"
#include "vm.h"
#include "error.h"
#include "debug.h"


// A function call frame, storing information about the
// functions currently executing.
typedef struct {
	// The function's stack pointer, indicating the start of
	// the function's local variables on the stack.
	int stack_start;

	// The saved instruction pointer, pointing into the
	// function's bytecode instructions array.
	uint8_t *instruction_ptr;
} CallFrame;


// Create a new virtual machine with `source` as the program's
// source code.
//
// Nothing is compiled or run until `vm_compile` and `vm_run`
// are called.
VirtualMachine vm_new(char *source) {
	VirtualMachine vm;
	vm.lexer = lexer_new(source);
	vm.function_count = 0;
	vm.literal_count = 0;
	vm.upvalue_count = 0;
	return vm;
}


// Free any resources allocated by the VM.
void vm_free(VirtualMachine *vm) {
	// Free functions
	for (int i = 0; i < vm->function_count; i++) {
		bytecode_free(&vm->functions[i].bytecode);
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
	Function *fn;
	vm_new_function(vm, &fn);
	fn->is_main = true;
	fn->bytecode = bytecode_new(DEFAULT_INSTRUCTIONS_CAPACITY);

	// Compile the source code into the function's
	// bytecode array.
	compile(vm, NULL, fn, TOKEN_END_OF_FILE);
}



//
//  Execution
//

// The maximum size of the stack.
#define MAX_STACK_SIZE 2048

// The maximum size of the function call frame stack.
#define MAX_CALL_STACK_SIZE 1024


// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm) {
	// The stack, where local variables and intermediate values
	// for operations are stored.
	uint64_t stack[MAX_STACK_SIZE];
	int stack_size = 0;

	// The function call frame stack, where the call stack is
	// stored (all functions currently being executed).
	CallFrame call_stack[MAX_CALL_STACK_SIZE];
	int call_stack_size = 0;

	// The currently executing instruction.
	uint8_t *ip = NULL;

	// The stack pointer of the top most call frame, pointing to
	// a place on the stack where the function's variables
	// start.
	int stack_start = 0;

	// Push a value onto the top of the stack.
	#define PUSH(value) \
		stack[stack_size++] = (value);

	// Pop a value from the top of the stack.
	#define POP() \
		stack_size--;

	// Evaluates to the value on the top of the stack.
	#define TOP() \
		stack[stack_size - 1]

	// Pushes a new call frame for `fn` onto the call frame
	// stack. Updates `ip` and `stack_start` with the values for
	// the new function.
	#define PUSH_FRAME(fn)                                           \
		if (call_stack_size + 1 > MAX_CALL_STACK_SIZE) {             \
			error(-1, "Stack overflow");                             \
		}                                                            \
		if (call_stack_size > 0) {                                   \
			call_stack[call_stack_size - 1].instruction_ptr = ip;    \
		}                                                            \
		if (stack_size > 0) {                                        \
			stack_start = stack_size - (fn)->arity;                  \
		} else {                                                     \
			stack_start = 0;                                         \
		}                                                            \
		for (int i = 0; i < (fn)->defined_upvalue_count; i++) {      \
			(fn)->defined_upvalues[i]->function_index = stack_start; \
		}                                                            \
		ip = (fn)->bytecode.instructions;                            \
		call_stack[call_stack_size].stack_start = stack_start;       \
		call_stack[call_stack_size].instruction_ptr = ip;            \
		call_stack_size++;

	// Push the main function onto the call stack. The main
	// function is always the first function in the functions
	// array.
	PUSH_FRAME(&vm->functions[0]);

	// Begin execution
instructions:
	switch (READ_BYTE()) {
	// Push a number onto the top of the stack.
	case CODE_PUSH_NUMBER:
		PUSH(READ_8_BYTES());
		goto instructions;

	// Push a string literal from the virtual machine's literals
	// list onto the top of the stack.
	case CODE_PUSH_STRING: {
		uint16_t index = READ_2_BYTES();
		String *copy = string_copy(vm->literals[index]);
		PUSH(ptr_to_value(copy));
		goto instructions;
	}

	// Push true onto the top of the stack
	case CODE_PUSH_TRUE:
		PUSH(TRUE_VALUE)
		goto instructions;

	// Push false onto the top of the stack.
	case CODE_PUSH_FALSE:
		PUSH(FALSE_VALUE)
		goto instructions;

	// Push nil onto the top of the stack.
	case CODE_PUSH_NIL:
		PUSH(NIL_VALUE)
		goto instructions;

	// Push a copy of a value from the stack onto the top of the
	// stack.
	case CODE_PUSH_LOCAL: {
		uint16_t index = READ_2_BYTES();
		PUSH(stack[stack_start + index]);
		goto instructions;
	}

	// Push a closure index onto the top of the stack.
	case CODE_PUSH_CLOSURE: {
		uint16_t index = READ_2_BYTES();
		PUSH(CLOSURE_TO_VALUE(index));
		goto instructions;
	}

	// Push an upvalue onto the top of the stack. If the upvalue
	// is open, pushes another value in the stack. If the
	// upvalue is closed, then pushes the `value` field of the
	// upvalue.
	case CODE_PUSH_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		Upvalue *upvalue = &vm->upvalues[index];
		if (upvalue->closed) {
			// Push the value stored in the upvalue
			PUSH(upvalue->value);
		} else {
			// Push the value stored in the stack
			PUSH(stack[upvalue->function_index + upvalue->local_index]);
		}

		goto instructions;
	}

	// Pop an item from the top of the stack.
	case CODE_POP:
		POP();
		goto instructions;

	// Pop the item off the top of the stack and write it to
	// another location in the stack.
	case CODE_STORE_LOCAL: {
		uint16_t index = READ_2_BYTES();
		int stack_index = stack_start + index;
		stack[stack_index] = TOP();
		if (stack_size - 1 > stack_index) {
			POP();
		}
		goto instructions;
	}

	// Pop the top of the stack and store it into an upvalue.
	case CODE_STORE_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		Upvalue *upvalue = &vm->upvalues[index];
		if (upvalue->closed) {
			// Store the value directly into the upvalue
			upvalue->value = TOP();
			POP();
		} else {
			int stack_index = upvalue->function_index + upvalue->local_index;
			stack[stack_index] = TOP();

			// Pop the item off the stack only if the upvalue
			// isn't storing into the top stack position
			if (stack_start - 1 > stack_index) {
				POP();
			}
		}
		goto instructions;
	}

	// Hoist the upvalue's value out of the stack and into the
	// `value` field of the upvalue, allowing it to persist
	// in memory even if the function's frame is destroyed.
	case CODE_CLOSE_UPVALUE: {
		uint16_t index = READ_2_BYTES();
		Upvalue *upvalue = &vm->upvalues[index];
		upvalue->value = stack[upvalue->function_index + upvalue->local_index];
		upvalue->closed = true;
		goto instructions;
	}

	// Jump the instruction pointer forwards.
	case CODE_JUMP_FORWARD: {
		uint16_t amount = READ_2_BYTES();
		ip += amount;
		goto instructions;
	}

	// Jump the instruction pointer backwards.
	case CODE_JUMP_BACK: {
		uint16_t amount = READ_2_BYTES();
		ip -= amount;
		goto instructions;
	}

	// Jump the instruction pointer forwards if the value on the
	// top of the stack is false.
	case CODE_JUMP_IF_NOT:
		if (IS_NIL(TOP()) || IS_FALSE(TOP())) {
			uint16_t amount = READ_2_BYTES();
			ip += amount;
		} else {
			// Discard the two byte argument
			READ_2_BYTES();
		}

		// Discard the conditional expression result
		POP();
		goto instructions;

	// Push a new function onto the call stack and start
	// executing it.
	case CODE_CALL: {
		uint16_t index = READ_2_BYTES();
		PUSH_FRAME(&vm->functions[index]);
		goto instructions;
	}

	// Pop the top of the stack and convert it to a function
	// index, calling it.
	case CODE_CALL_STACK: {
		uint64_t value = TOP();
		if (!IS_CLOSURE(value)) {
			error(-1, "Attempting to call non-closure variable");
		}

		POP();
		uint16_t index = VALUE_TO_CLOSURE(value);
		PUSH_FRAME(&vm->functions[index]);
		goto instructions;
	}

	// Call a native C function, giving it the stack.
	case CODE_CALL_NATIVE: {
		NativeFunction fn = value_to_ptr(READ_8_BYTES());
		fn(stack, &stack_size);
		goto instructions;
	}

	// Return from the current function.
	case CODE_RETURN:
		if (call_stack_size == 1) {
			// Returning from the main function, so halt the
			// program.
			break;
		}

		// Store the return value
		uint64_t return_value = TOP();

		// Reset the stack and instruction pointers
		call_stack_size--;
		ip = call_stack[call_stack_size - 1].instruction_ptr;
		stack_size = stack_start;
		stack_start = call_stack[call_stack_size - 1].stack_start;

		// Push the return value
		PUSH(return_value);

		goto instructions;
	}
}



//
//  Function Definitions
//

// Defines a new function on the virtual machine, returning a
// pointer to it and its index in the virtual machine's function
// list.
//
// Performs no allocation, so the returned function's bytecode
// array still needs to be allocated.
int vm_new_function(VirtualMachine *vm, Function **fn) {
	int index = vm->function_count;
	vm->function_count++;
	*fn = &vm->functions[index];
	(*fn)->is_main = false;
	(*fn)->name = NULL;
	(*fn)->length = 0;
	(*fn)->arity = 0;
	(*fn)->captured_upvalue_count = 0;
	(*fn)->defined_upvalue_count = 0;
	return index;
}


// Returns the index of a user-defined function named `name`.
// Returns -1 if no function with that name is found.
int vm_find_function(VirtualMachine *vm, char *name, int length, int arity) {
	for (int i = 0; i < vm->function_count; i++) {
		Function *fn = &vm->functions[i];

		if (fn->length == length && fn->arity == arity &&
				strncmp(fn->name, name, length) == 0) {
			return i;
		}
	}

	return -1;
}


// Returns a function pointer to a library function named
// `name`.
//
// Returns NULL no function with that name is found.
NativeFunction vm_find_native_function(VirtualMachine *vm, char *name,
		int length, int arity) {
	if (length == 5 && strncmp(name, "print", length) == 0) {
		if (arity == 1) {
			return &native_print;
		} else if (arity == 2) {
			return &native_print_2;
		}
	} else if (arity == 1 && length == 6 &&
			strncmp(name, "assert", length) == 0) {
		return &native_assert;
	}

	return NULL;
}



//
//  Upvalues
//

// Create a new upvalue, returning a pointer to it and its index
// in the upvalues list.
int vm_new_upvalue(VirtualMachine *vm, Upvalue **upvalue) {
	*upvalue = &vm->upvalues[vm->upvalue_count++];
	(*upvalue)->closed = false;
	(*upvalue)->local_index = 0;
	(*upvalue)->function_index = 0;
	(*upvalue)->name = NULL;
	(*upvalue)->length = 0;
	(*upvalue)->defining_function = NULL;
	return vm->upvalue_count - 1;
}
