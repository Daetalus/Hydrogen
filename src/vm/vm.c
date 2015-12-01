
//
//  Virtual Machine
//

#include <string.h>

#include "vm.h"
#include "parser.h"


// Frees a pointer only if it's not NULL.
#define FREE(ptr)        \
	if ((ptr) != NULL) { \
		free((ptr));     \
	}


// Create a new interpreter state.
HyVM * hy_new(void) {
	VirtualMachine *vm = malloc(sizeof(VirtualMachine));

	// Allocate memory for arrays
	ARRAY_INIT(vm->functions, Function, 4);
	ARRAY_INIT(vm->packages, Package, 4);
	ARRAY_INIT(vm->numbers, double, 16);
	ARRAY_INIT(vm->strings, char *, 16);

	// Initialise the error
	vm->error.description = NULL;
	vm->error.line = 0;
	vm->error.package = NULL;
	vm->error.file = NULL;

	return (HyVM *) vm;
}


// Free an interpreter's state.
void hy_free(HyVM *vm) {
	// Packages
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];
		package_free(package);
	}

	// Arrays
	free(vm->packages);
	free(vm->functions);
	free(vm->numbers);
	free(vm->strings);

	// The VM itself
	free(vm);
}


// Frees the current error on the VM.
void vm_free_error(VirtualMachine *vm) {
	FREE(vm->error.description);
	FREE(vm->error.package);
	FREE(vm->error.file);
}


// Returns true when an error has occurred.
bool vm_has_error(VirtualMachine *vm) {
	return vm->error.description != NULL;
}



//
//  Execution
//

// Runs the given source code string.
HyResult hy_exec_string(HyVM *vm, char *source) {
	Package *main = package_new(vm);
	main->source = source;
	parse_package(vm, main);
	return fn_exec(vm, main->main_fn);
}


// Returns the most recent error that has occurred.
HyError * hy_error(HyVM *vm) {
	return &vm->error;
}



//
//  Constants
//

// Adds a string to the VM's strings list. Returns the
// index of the added string.
uint16_t vm_add_string(VirtualMachine *vm, char *string) {
	uint16_t index = vm->strings_count++;
	ARRAY_REALLOC(vm->strings, char *);
	vm->strings[index] = string;
	return index;
}


// Adds a number to the VM's numbers list. Returns the
// index of the added number.
uint16_t vm_add_number(VirtualMachine *vm, double number) {
	uint16_t index = vm->numbers_count++;
	ARRAY_REALLOC(vm->numbers, double);
	vm->numbers[index] = number;
	return index;
}



//
//  Packages
//

// Defines a new package.
Package * package_new(VirtualMachine *vm) {
	// Increment the size of the packages array
	uint32_t index = vm->packages_count++;
	ARRAY_REALLOC(vm->packages, Package);

	// Initialise the package
	Package *package = &vm->packages[index];
	package->name = NULL;
	package->source = NULL;
	package->main_fn = 0;
	ARRAY_INIT(package->functions, Function *, 4);
	return package;
}


// Frees a package.
void package_free(Package *package) {
	free(package->functions);
}


// Finds a package with the given name. Returns NULL if
// the package doesn't exist.
Package * package_find(VirtualMachine *vm, char *name, size_t length) {
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];

		// Check the length of the package name and the
		// name itself are equal
		if (strlen(package->name) == length &&
				strncmp(package->name, name, length) == 0) {
			return package;
		}
	}

	// Couldn't find a package with the given name
	return NULL;
}



//
//  Functions
//

// Defines a new function and associates it with the given
// package, or with the global namespace if `package` is
// NULL.
Function * fn_new(VirtualMachine *vm, Package *package, uint16_t *index) {
	// Increment the size of the functions array
	uint32_t fn_index = vm->functions_count++;
	ARRAY_REALLOC(vm->functions, Function);

	// Set the index
	if (index != NULL) {
		*index = fn_index;
	}

	// Initialise the function
	Function *fn = &vm->functions[fn_index];
	fn->name = NULL;
	fn->length = 0;
	fn->package = NULL;
	ARRAY_INIT(fn->bytecode, uint64_t, 64);

	// Add the function to the package's function list
	uint32_t package_index = package->functions_count++;
	ARRAY_REALLOC(package->functions, Function *);
	package->functions[package_index] = fn;

	return fn;
}


// Finds a function with the given name. Returns NULL if
// the function doesn't exist.
Function * fn_find(VirtualMachine *vm, char *name, size_t length,
		uint16_t *index) {
	// Functions that are defined recently are more likely
	// to be used sooner, so search the array in reverse
	// order
	for (int i = vm->functions_count; i >= 0; i--) {
		Function *fn = &vm->functions[i];

		// Check if the length of the function's name
		// matches, along with the name itself
		if (fn->length == length && strncmp(fn->name, name, length) == 0) {
			// Set the index
			if (index != NULL) {
				*index = i;
			}
			return fn;
		}
	}

	// Couldn't find a function with the given name
	return NULL;
}


// Emits an instruction.
uint32_t fn_emit(Function *fn, uint64_t instruction) {
	uint16_t index = fn->bytecode_count++;
	ARRAY_REALLOC(fn->bytecode, uint64_t);
	fn->bytecode[index] = instruction;
	return index;
}



//
//  Execution
//

// Executes a compiled function on the virtual machine.
HyResult fn_exec(VirtualMachine *vm, uint16_t fn_index) {
	return HY_SUCCESS;
}
