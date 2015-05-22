
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

	// A copy of the receiver for this function, set to nil if
	// this function isn't a method.
	//
	// The receiver points to the `self` variable.
	uint64_t receiver;
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
	vm.function_capacity = 128;
	vm.functions = malloc(vm.function_capacity * sizeof(Function));

	vm.native_count = 0;
	vm.native_capacity = 64;
	vm.natives = malloc(vm.native_capacity * sizeof(Native));

	vm.literal_count = 0;
	vm.literal_capacity = 128;
	vm.literals = malloc(vm.literal_capacity * sizeof(String *));

	vm.upvalue_count = 0;
	vm.upvalue_capacity = 128;
	vm.upvalues = malloc(vm.upvalue_capacity * sizeof(Upvalue));

	vm.class_definition_count = 0;
	vm.class_definition_capacity = 16;
	size_t size = vm.class_definition_capacity * sizeof(ClassDefinition);
	vm.class_definitions = malloc(size);

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

	free(vm->functions);
	free(vm->natives);
	free(vm->literals);
	free(vm->upvalues);
}


// Compiles the source code into bytecode.
void vm_compile(VirtualMachine *vm) {
	// Create the main function, whose bytecode we'll populate.
	Function *fn;
	vm_new_function(vm, &fn);
	fn->bytecode = bytecode_new(DEFAULT_INSTRUCTIONS_CAPACITY);

	// Compile the source code into the function's
	// bytecode array.
	compile(vm, NULL, fn, TOKEN_END_OF_FILE, NULL);
}



//
//  Standard Library
//

// Attach the whole standard library to the virtual machine.
void vm_attach_standard_library(VirtualMachine *vm) {
	vm_attach_io(vm);
}


// Attach the IO module in the standard library to the virtual
// machine.
void vm_attach_io(VirtualMachine *vm) {
	vm_attach_native(vm, "print", 1, &native_print);
	vm_attach_native(vm, "print2", 2, &native_print_2);
	vm_attach_native(vm, "assert", 1, &native_assert);
}


// Attach a native function to the virtual machine, which acts
// as a library function when running the Hydrogen source.
void vm_attach_native(VirtualMachine *vm, char *name, int arity,
		NativeFunction fn) {
	Native *native;
	vm_new_native(vm, &native);
	native->name = name;
	native->arity = arity;
	native->fn = fn;
}



//
//  User Defined Functions
//

// Defines a new function, returning a pointer to it and its
// index in the VM's function list.
//
// Performs no allocation, so the returned function's bytecode
// object still needs to be allocated.
int vm_new_function(VirtualMachine *vm, Function **fn) {
	int index = vm->function_count;
	vm->function_count++;

	// Check that we haven't exceeded the hard limit on the
	// number of functions we can define.
	if (vm->function_count > MAX_FUNCTIONS) {
		error(-1, "Cannot define more than %d functions", MAX_FUNCTIONS);
	}

	// We've overstepped the limit on the number of functions we
	// can allocate, so increase the array's allocated capacity.
	if (vm->function_count > vm->function_capacity) {
		vm->function_capacity *= 2;

		size_t new_size = vm->function_capacity * sizeof(Function);
		vm->functions = realloc(vm->functions, new_size);
	}

	*fn = &vm->functions[index];
	(*fn)->arity = 0;
	(*fn)->captured_upvalue_count = 0;
	(*fn)->defined_upvalue_count = 0;

	return index;
}



//
//  Native Functions
//

// Defines a new native function, returning a pointer to it and
// its index in the VM's native function list.
int vm_new_native(VirtualMachine *vm, Native **native) {
	int index = vm->native_count;
	vm->native_count++;

	// Check we haven't exceeded the hard limit on the number of
	// native functions we're allowed to allocate.
	if (vm->native_count > MAX_NATIVES) {
		error(-1, "Cannot define more than %d native functions", MAX_NATIVES);
	}

	// Reallocate the natives array if the number of elements
	// exceeds the capacity.
	if (vm->native_count > vm->native_capacity) {
		vm->native_capacity *= 2;

		size_t new_size = vm->native_capacity * sizeof(Native);
		vm->natives = realloc(vm->natives, new_size);
	}

	*native = &vm->natives[index];
	(*native)->name = NULL;
	(*native)->fn = NULL;

	return index;
}


// Returns the index of the native function named `name`, or -1
// if no function is found.
int vm_find_native(VirtualMachine *vm, char *name, int length) {
	for (int i = 0; i < vm->native_count; i++) {
		Native *native = &vm->natives[i];
		if (strncmp(native->name, name, length) == 0) {
			return i;
		}
	}

	return -1;
}



//
//  Classes
//

// Create a new class definition, returning its index in the
// VM's class definitions list.
int vm_new_class_definition(VirtualMachine *vm, ClassDefinition **definition) {
	int index = vm->class_definition_count;
	vm->class_definition_count++;

	// Check we haven't exceeded the hard limit on the number of
	// class definitions we can have
	if (vm->class_definition_count > MAX_CLASSES) {
		error(-1, "Cannot define more than %d classes", MAX_CLASSES);
	}

	// If the number of classes we've defined exceeds the
	// array's capacity, then reallocate the array.
	if (vm->class_definition_count > vm->class_definition_capacity) {
		vm->class_definition_capacity *= 2;

		size_t new_size = vm->class_definition_capacity *
			sizeof(ClassDefinition);
		vm->class_definitions = realloc(vm->class_definitions, new_size);
	}

	*definition = &vm->class_definitions[index];
	(*definition)->name = NULL;
	(*definition)->length = 0;
	(*definition)->field_count = 0;
	(*definition)->method_count = 0;

	return index;
}


// Returns the index of the class named `name`, or -1 if no
// class with that name if found.
int vm_find_class(VirtualMachine *vm, char *name, int length) {
	for (int i = 0; i < vm->class_definition_count; i++) {
		ClassDefinition *definition = &vm->class_definitions[i];

		if (definition->length == length &&
				strncmp(definition->name, name, length) == 0) {
			// Found a matching class
			return i;
		}
	}

	return -1;
}


// Returns the index of a field within a class instance.
int find_class_field(ClassInstance *instance, char *name, int length) {
	ClassDefinition *definition = instance->definition;

	for (int i = 0; i < definition->field_count; i++) {
		Field *field = &definition->fields[i];

		if (field->length == length &&
				strncmp(field->name, name, length) == 0) {
			return i;
		}
	}

	return -1;
}



//
//  String Literals
//

// Create a new string literal, returning a pointer to it and
// its index in the literals list.
int vm_new_string_literal(VirtualMachine *vm, String ***literal) {
	int index = vm->literal_count;
	vm->literal_count++;

	// Check we haven't exceeded the hard limit on the number of
	// string literals we're allowed to allocate.
	if (vm->literal_count > MAX_STRING_LITERALS) {
		error(-1, "Cannot allocate more than %d string literals",
			MAX_STRING_LITERALS);
	}

	// Reallocate the literals array if the number of elements
	// in it is greater than its capacity.
	if (vm->literal_count > vm->literal_capacity) {
		vm->literal_capacity *= 2;

		size_t new_size = vm->literal_capacity * sizeof(String *);
		vm->literals = realloc(vm->literals, new_size);
	}

	*literal = &vm->literals[index];
	return index;
}



//
//  Upvalues
//

// Create a new upvalue, returning a pointer to it and its index
// in the upvalues list.
int vm_new_upvalue(VirtualMachine *vm, Upvalue **upvalue) {
	int index = vm->upvalue_count;
	vm->upvalue_count++;

	// Check we haven't exceeded the hard limit on the number of
	// upvalues we can create
	if (vm->upvalue_count > MAX_UPVALUES) {
		error(-1, "Cannot create more than %d upvalues", MAX_UPVALUES);
	}

	// Reallocate the array if the number of elements in it
	// exceeds its capacity
	if (vm->upvalue_count > vm->upvalue_capacity) {
		vm->upvalue_capacity *= 2;

		size_t new_size = vm->upvalue_capacity * sizeof(Upvalue);
		vm->upvalues = realloc(vm->upvalues, new_size);
	}

	*upvalue = &vm->upvalues[index];
	(*upvalue)->closed = false;
	(*upvalue)->local_index = 0;
	(*upvalue)->function_index = 0;
	(*upvalue)->name = NULL;
	(*upvalue)->length = 0;
	(*upvalue)->defining_function = NULL;

	return index;
}



//
//  Execution
//

// The maximum size of the stack.
#define MAX_STACK_SIZE 2048

// The maximum size of the function call frame stack (ie. the
// recursive depth limit before we hit a stack overflow).
#define MAX_CALL_STACK_SIZE 1024


// Trigger an invalid number of arguments error.
void assert_arity(int have, int expected) {
	if (have != expected) {
		error(-1, "Attempt to call function with incorrect number"
			"of arguments (have %d, expected %d)", have, expected);
	}
}


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
	#define PUSH_FRAME(fn, receiver_value)                           \
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
		call_stack[call_stack_size].receiver = (receiver_value);     \
		call_stack_size++;

	// Push the main function onto the call stack. The main
	// function is always the first function in the functions
	// array.
	PUSH_FRAME(&vm->functions[0], NIL_VALUE);

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

	// Push a native function onto the stack.
	case CODE_PUSH_NATIVE: {
		uint16_t index = READ_2_BYTES();
		PUSH(NATIVE_TO_VALUE(index));
		goto instructions;
	}

	// Push a closure index onto the top of the stack.
	case CODE_PUSH_FUNCTION: {
		uint16_t index = READ_2_BYTES();
		PUSH(FUNCTION_TO_VALUE(index));
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

	// Pop a class off the stack (triggering an error if it
	// isn't one) and push one of its fields.
	case CODE_PUSH_FIELD: {
		uint16_t length = READ_2_BYTES();
		char *name = value_to_ptr(READ_8_BYTES());

		uint64_t ptr = TOP();
		POP();

		if (!IS_PTR(ptr)) {
			// Not a class, so trigger an error
			error(-1, "Attempt to access field `%.*s` of non-object",
				length, name);
		}

		ClassInstance *instance = value_to_ptr(ptr);
		int index = find_class_field(instance, name, length);
		if (index == -1) {
			// Couldn't find a field with the given name
			error(-1, "Attempt to access missing field `%.*s` on object",
				length, name);
		}

		// Push the field
		PUSH(instance->fields[index]);
		goto instructions;
	}

	// Push the receiver of the current function's stack frame,
	// triggering an error if it's nil (meaning we're not in a
	// method and we're using `self` illegally).
	case CODE_PUSH_RECEIVER: {
		uint64_t receiver = call_stack[call_stack_size - 1].receiver;

		if (IS_NIL(receiver)) {
			// We're using `self` inside a function that isn't a
			// method
			error(-1, "Attempt to use `self` in non-method");
		}

		PUSH(receiver);
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

	// Pop an item off the stack, using this as the value to
	// store. Pop another item off the stack, and store the
	// first value into a field on this second item.
	case CODE_STORE_FIELD: {
		uint16_t length = READ_2_BYTES();
		char *name = value_to_ptr(READ_8_BYTES());

		// Pop the value to store
		uint64_t value = TOP();
		POP();

		// Pop the class to store into
		uint64_t ptr = TOP();
		POP();

		if (!IS_PTR(ptr)) {
			// Trying to store into a value that isn't an
			// instance of a class
			error(-1, "Attempt to write to field `%.*s` of non-object",
				length, name);
		}

		ClassInstance *instance = value_to_ptr(ptr);
		int index = find_class_field(instance, name, length);
		if (index == -1) {
			// Field doesn't exist
			error(-1, "Attempt to write to missing field `%.*s` on object",
				length, name);
		}

		// Write to the field
		instance->fields[index] = value;
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

	// Pop the top off the stack and call it.
	case CODE_CALL: {
		uint16_t arity = READ_2_BYTES();

		// The function we're trying to call is placed
		// underneath the arguments we're passing to it, so we
		// can't just access the top element in the stack.
		//
		// This function which was pushed underneath the
		// arguments is popped upon the CODE_RETURN instruction
		// at the end of the function.
		uint64_t value = stack[stack_size - arity - 1];

		if (IS_METHOD(value)) {
			Method *method = VALUE_TO_METHOD(value);
			Function *fn = &vm->functions[method->function_index];
			assert_arity(arity, fn->arity);
			PUSH_FRAME(fn, ptr_to_value(method->instance));
		} else if (IS_FUNCTION(value)) {
			uint16_t index = VALUE_TO_FUNCTION(value);
			Function *fn = &vm->functions[index];
			assert_arity(arity, fn->arity);
			PUSH_FRAME(fn, NIL_VALUE);
		} else if (IS_NATIVE(value)) {
			uint16_t index = VALUE_TO_NATIVE(value);
			Native *native = &vm->natives[index];
			assert_arity(arity, native->arity);
			native->fn(stack, &stack_size);

			// Save the return value from native function and
			// pop it, because we need to pop the function we're
			// calling from beneath it
			uint16_t return_value = TOP();
			POP();

			// The function we're calling was pushed before the
			// arguments passed to it, so once we've finished
			// calling the function, we need to pop the function
			// itself.
			POP();

			// Since we popped the return value earlier, we need
			// to push it again.
			PUSH(return_value);
		} else {
			error(-1, "Attempt to call non-function variable");
		}

		goto instructions;
	}

	// Call a native C function, giving it the stack.
	case CODE_CALL_NATIVE: {
		NativeFunction fn = value_to_ptr(READ_8_BYTES());
		fn(stack, &stack_size);
		goto instructions;
	}

	// Instantiate a new instance of a class and push it onto
	// the stack.
	case CODE_INSTANTIATE_CLASS: {
		uint16_t index = READ_2_BYTES();
		ClassDefinition *definition = &vm->class_definitions[index];

		// Create the instance on the heap
		size_t size = definition->field_count * sizeof(uint64_t) +
			sizeof(ClassInstance);
		ClassInstance *instance = malloc(size);

		// Set the instance's definition
		instance->definition = definition;

		// Set the instance's methods list
		for (int i = 0; i < definition->method_count; i++) {
			Method *method = &instance->methods[i];
			method->function_index = definition->methods[i].function_index;
			method->instance = instance;
		}

		// Set each of the instance's fields
		for (int i = 0; i < definition->field_count; i++) {
			int method_index = definition->fields[i].method_index;
			if (method_index != -1) {
				// The field is a method
				Method *method = &instance->methods[method_index];
				instance->fields[i] = METHOD_TO_VALUE(method);
			} else {
				// Just set the field to nil
				instance->fields[i] = NIL_VALUE;
			}
		}

		// Push the class
		PUSH(ptr_to_value(instance));
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

		// The original function value we called with the
		// `CODE_CALL` instruction (which we need to pop) lies
		// beneath the return value, so pop it.
		POP();

		// Push the return value
		PUSH(return_value);

		goto instructions;
	}
}
