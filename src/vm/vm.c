
//
//  Virtual Machine
//

#include <string.h>

#include "vm.h"
#include "parser.h"
#include "error.h"


// Create a new interpreter state.
HyVM * hy_new(void) {
	VirtualMachine *vm = malloc(sizeof(VirtualMachine));

	// Error
	vm->err.description = NULL;
	vm->err.line = 0;

	// Allocate memory for arrays
	ARRAY_INIT(vm->functions, Function, 4);
	ARRAY_INIT(vm->packages, Package, 4);
	ARRAY_INIT(vm->numbers, double, 16);
	ARRAY_INIT(vm->strings, char *, 16);
	ARRAY_INIT(vm->upvalues, Upvalue, 4);

	return (HyVM *) vm;
}


// Free an interpreter's state.
void hy_free(HyVM *vm) {
	// Packages
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];
		package_free(package);
	}

	// Strings
	for (uint32_t i = 0; i < vm->strings_count; i++) {
		free(vm->strings[i]);
	}

	// Functions
	for (uint32_t i = 0; i < vm->functions_count; i++) {
		Function *fn = &vm->functions[i];
		fn_free(fn);
	}

	// Arrays
	free(vm->packages);
	free(vm->functions);
	free(vm->numbers);
	free(vm->strings);
	free(vm->upvalues);

	// Error
	err_free(&vm->err);

	// The VM itself
	free(vm);
}


// Returns true when an error has occurred.
bool vm_has_error(VirtualMachine *vm) {
	return vm->err.description != NULL;
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
	return &vm->err;
}



//
//  Constants
//

// Adds a string to the VM's strings list. Returns the index of the added
// string.
uint16_t vm_add_string(VirtualMachine *vm, char *string) {
	uint16_t index = vm->strings_count++;
	ARRAY_REALLOC(vm->strings, char *);
	vm->strings[index] = string;
	return index;
}


// Adds a number to the VM's numbers list. Returns the index of the added
// number.
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


// Finds a package with the given name. Returns NULL if the package doesn't
// exist.
Package * package_find(VirtualMachine *vm, char *name, size_t length) {
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];

		// Check the length of the package name and the name itself are equal
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

// Defines a new function and associates it with the given package, or with
// the global namespace if `package` is NULL.
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
	fn->arity = 0;
	fn->package = package;
	ARRAY_INIT(fn->bytecode, uint64_t, 64);

	// Add the function to the package's function list
	uint32_t package_index = package->functions_count++;
	ARRAY_REALLOC(package->functions, Function *);
	package->functions[package_index] = fn;

	return fn;
}


// Frees resources allocated by a function.
void fn_free(Function *fn) {
	free(fn->bytecode);
}


// Finds a function with the given name. Returns NULL if the function doesn't
// exist.
Function * fn_find(VirtualMachine *vm, char *name, size_t length,
		uint16_t *index) {
	// Functions that are defined recently are more likely to be used sooner
	// (maybe?), so search the array in reverse order
	for (int i = vm->functions_count; i >= 0; i--) {
		Function *fn = &vm->functions[i];

		// Check if the length of the function's name matches, along with the
		// name itself
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



//
//  Upvalues
//

// Creates a new upvalue.
Upvalue * upvalue_new(VirtualMachine *vm, int *requested_index) {
	// Get the index of the new upvalue
	uint32_t index = vm->upvalues_count++;
	ARRAY_REALLOC(vm->upvalues, Upvalue);

	// Set the requested index
	if (requested_index != NULL) {
		*requested_index = index;
	}

	// Initialise the upvalue
	Upvalue *upvalue = &vm->upvalues[index];
	upvalue->name = NULL;
	upvalue->length = 0;
	upvalue->open = true;
	upvalue->value = 0;
	return upvalue;
}


// Returns the index of the upvalue called `name`, or -1 if no such upvalue
// exists.
int upvalue_find(VirtualMachine *vm, char *name, size_t length) {
	// Iterate in reverse order
	for (int i = vm->upvalues_count - 1; i >= 0; i--) {
		Upvalue *upvalue = &vm->upvalues[i];
		if (upvalue->length == length &&
				strncmp(upvalue->name, name, length) == 0) {
			// Found the upvalue
			return i;
		}
	}

	// Couldn't find the upvalue
	return -1;
}



//
//  Struct Definitions
//

// Creates a new struct definition.
StructDefinition * struct_new(VirtualMachine *vm) {
	int index = vm->structs_count++;
	ARRAY_REALLOC(vm->structs, StructDefinition);

	// Initialise the struct definition
	StructDefinition *def = &vm->structs[index];
	def->name = NULL;
	def->length = 0;
	def->constructor = -1;
	ARRAY_INIT(def->fields, Identifier, 2);
	return def;
}


// Frees a struct.
void struct_free(StructDefinition *def) {
	free(def->fields);
}


// Creates a new field on a struct definition.
Identifier * struct_new_field(StructDefinition *def) {
	int index = def->fields_count++;
	ARRAY_REALLOC(def->fields, Identifier);

	Identifier *field = &def->fields[index];
	field->start = NULL;
	field->length = 0;
	return field;
}


// Returns a struct definition called `name`, or NULL if no such struct exists.
StructDefinition * struct_find(VirtualMachine *vm, char *name, size_t length,
		uint16_t *index) {
	for (int i = vm->structs_count - 1; i >= 0; i--) {
		StructDefinition *def = &vm->structs[i];
		if (def->length == length && strncmp(def->name, name, length) == 0) {
			// Set requested index
			if (index != NULL) {
				*index = i;
			}

			// Found requested struct
			return def;
		}
	}

	// Couldn't find the struct
	return NULL;
}



//
//  Execution
//

// Executes a compiled function on the virtual machine.
HyResult fn_exec(VirtualMachine *vm, uint16_t fn_index) {
	return HY_SUCCESS;
}
