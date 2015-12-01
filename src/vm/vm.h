
//
//  Virtual Machine
//


#ifndef VM_H
#define VM_H

#include <stdlib.h>
#include <stdbool.h>

#include <hydrogen.h>

#include "util.h"


// A package.
typedef struct package Package;


// A user-defined function.
typedef struct {
	// The name of the function. NULL for an anonymous
	// function.
	char *name;
	size_t length;

	// The compiled bytecode for the function.
	ARRAY(uint64_t, bytecode);

	// The package the function was defined in.
	Package *package;
} Function;


// A package.
struct package {
	// The name of the package as a heap allocated string.
	// NULL if the package is anonymous.
	char *name;

	// The source code of the package.
	char *source;

	// The index of the package's main function.
	uint16_t main_fn;

	// A list of functions defined in this package.
	ARRAY(Function *, functions);
};


// A virtual machine, storing the interpreter's state.
typedef struct _vm {
	// A list of compiled functions.
	ARRAY(Function, functions);

	// All imported packages.
	ARRAY(Package, packages);

	// All numbers encountered during compilation.
	ARRAY(double, numbers);

	// All strings encountered during compilation.
	ARRAY(char *, strings);

	// The most recent error triggered on the VM.
	HyError error;
} VirtualMachine;


// Adds a string to the VM's strings list. Returns the
// index of the added string.
uint16_t vm_add_string(VirtualMachine *vm, char *string);

// Adds a number to the VM's numbers list. Returns the
// index of the added number.
uint16_t vm_add_number(VirtualMachine *vm, double number);

// Frees the current error on the VM.
void vm_free_error(VirtualMachine *vm);

// Returns true when an error has occurred.
bool vm_has_error(VirtualMachine *vm);


// Defines a new package.
Package * package_new(VirtualMachine *vm);

// Frees a package.
void package_free(Package *package);

// Finds a package with the given name. Returns NULL if
// the package doesn't exist.
Package * package_find(VirtualMachine *vm, char *name, size_t length);


// Defines a new function and associates it with the given
// package, or with the global namespace if `package` is
// NULL.
Function * fn_new(VirtualMachine *vm, Package *package, uint16_t *index);

// Finds a function with the given name. Returns NULL if
// the function doesn't exist.
Function * fn_find(VirtualMachine *vm, char *name, size_t length,
	uint16_t *index);

// Emits an instruction.
uint32_t fn_emit(Function *fn, uint64_t instruction);


// Executes a compiled function on the virtual machine.
HyResult fn_exec(VirtualMachine *vm, uint16_t fn_index);

#endif
