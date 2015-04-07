
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
	vm.source = source;
	lexer_new(&vm.lexer, source);
	vm.function_count = 0;
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
	Function *fn = define_bytecode_function(vm, NULL, 0, 0);

	// Create the constants list that will be populated
	Constants constants;
	constants.count = 0;

	// Compile the source code into the function's
	// bytecode array.
	compile(vm, fn, &constants, TOKEN_END_OF_FILE);
}


// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm) {

}


// Defines a new function on the virtual machine, returning
// a pointer to it.
Function * define_bytecode_function(
		VirtualMachine *vm,
		char *name,
		int length,
		int argument_count) {
	// Increment the number of functions the VM has
	Function *fn = &vm->functions[vm->function_count];
	vm->function_count++;

	// Set the function's initial values
	// Allocate enough space for 32 instructions
	fn->name = name;
	fn->length = length;
	fn->argument_count = argument_count;
	bytecode_new(&fn->bytecode, 32);

	return fn;
}


// Returns the index of a function with the given name and
// name length.
// Returns -1 if the function isn't found.
int index_of_function(VirtualMachine *vm, char *name, int length,
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
NativeFunction index_of_native_function(VirtualMachine *vm,
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
