
//
//  Virtual Machine
//


#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "bytecode.h"
#include "compiler.h"


// Create a new virtual machine with the given source code.
// Initialises the VM with default values, and doesn't compile
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

	// Create the constants list that will be populated
	Constants constants;
	constants.count = 0;

	// Compile the source code into the function's
	// bytecode array.
	compile(vm, fn, TOKEN_END_OF_FILE);
}


// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm) {

}


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
int find_function(VirtualMachine *vm, char *name, int length,
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


void native_print(void) {
	printf("IT WORKS!\n");
}


void native_print_2(void) {
	printf("IT WORKS AGAIN!\n");
}


// Returns a function pointer to a library function with the
// given name and length.
// Returns NULL if the function isn't found.
NativeFunction find_native_function(VirtualMachine *vm,
		char *name, int length, int argument_count) {
	if (strncmp(name, "print", length) == 0) {
		if (argument_count == 1) {
			return &native_print;
		} else if (argument_count == 2) {
			return &native_print_2;
		}
	}

	return NULL;
}
