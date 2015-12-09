
//
//  Virtual Machine
//

#ifndef VM_H
#define VM_H

#include <stdlib.h>
#include <stdbool.h>

#include <hydrogen.h>

#include "util.h"
#include "lexer.h"


// A package.
typedef struct package Package;


// A user-defined function.
typedef struct {
	// The name of the function. NULL for an anonymous function.
	char *name;
	size_t length;

	// The number of arguments expected by the function.
	uint32_t arity;

	// The compiled bytecode for the function.
	ARRAY(uint64_t, bytecode);

	// The package the function was defined in.
	Package *package;
} Function;


// An upvalue.
typedef struct {
	// The name of the upvalue.
	char *name;
	size_t length;

	// Whether or not the upvalue is currently open.
	bool open;

	// The function the local the upvalue closes over was defined in.
	Function *defining_fn;

	union {
		// The stack position of the upvalue, used when it is open.
		uint16_t slot;

		// The value of the upvalue, used once its been closed.
		uint64_t value;
	};
} Upvalue;


// A package (collection of functions, global variables, and constants),
// contained in a single file.
struct package {
	// The name of the package as a heap allocated string. NULL if the package
	// is anonymous.
	char *name;

	// The source code of the package. A pointer provided by the user, not to be
	// freed with the rest of the package.
	char *source;

	// The index of the package's main function.
	uint16_t main_fn;

	// A list of functions defined in this package.
	ARRAY(Function *, functions);
};


// A struct definition.
typedef struct {
	// The name of the struct.
	char *name;
	size_t length;

	// The index of the struct's constructor function, or -1 if the struct
	// doesn't have a constructor.
	int constructor;

	// The names of the struct's fields.
	ARRAY(Identifier, fields);

	// The default values for the struct's fields.
	ARRAY(uint64_t, values);
} StructDefinition;


// A virtual machine, storing the interpreter's state.
typedef struct vm {
	// A list of compiled functions.
	ARRAY(Function, functions);

	// All imported packages.
	ARRAY(Package, packages);

	// Numbers encountered during compilation.
	ARRAY(double, numbers);

	// Strings encountered during compilation.
	ARRAY(char *, strings);

	// Upvalues found during compilation.
	ARRAY(Upvalue, upvalues);

	// Struct definitions encountered during compilation.
	ARRAY(StructDefinition, structs);

	// The most recent error triggered on the VM.
	HyError err;
} VirtualMachine;


// Adds a string to the VM's strings list. Returns the index of the added
// string.
uint16_t vm_add_string(VirtualMachine *vm, char *string);

// Adds a number to the VM's numbers list. Returns the index of the added
// number.
uint16_t vm_add_number(VirtualMachine *vm, double number);

// Returns true when an error has occurred.
bool vm_has_error(VirtualMachine *vm);


// Defines a new package.
Package * package_new(VirtualMachine *vm);

// Frees a package.
void package_free(Package *package);

// Finds a package with the given name. Returns NULL if the package doesn't
// exist.
Package * package_find(VirtualMachine *vm, char *name, size_t length);


// Defines a new function in the given package, or in the global namespace if
// the package is NULL.
Function * fn_new(VirtualMachine *vm, Package *package, uint16_t *index);

// Frees resources allocated by a function.
void fn_free(Function *fn);

// Finds a function with the given name. Returns NULL if the function doesn't
// exist.
Function * fn_find(VirtualMachine *vm, char *name, size_t length,
	uint16_t *index);


// Creates a new upvalue.
Upvalue * upvalue_new(VirtualMachine *vm, int *index);

// Returns the index of the upvalue called `name`, or -1 if no such upvalue
// exists.
int upvalue_find(VirtualMachine *vm, char *name, size_t length);


// Creates a new struct definition.
StructDefinition * struct_new(VirtualMachine *vm);

// Frees a struct.
void struct_free(StructDefinition *def);

// Creates a new field on a struct definition, returning its index.
int struct_new_field(StructDefinition *def);

// Returns a struct definition called `name`, or NULL if no such struct exists.
StructDefinition * struct_find(VirtualMachine *vm, char *name, size_t length,
	uint16_t *index);


// Executes a compiled function on the virtual machine.
HyResult fn_exec(VirtualMachine *vm, uint16_t fn_index);

#endif
