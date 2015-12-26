
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


// The maximum number of upvalues that can be defined in a function.
#define MAX_UPVALUES_IN_FN 64


// A package.
typedef struct package Package;


// An upvalue.
typedef struct {
	// The name of the upvalue.
	char *name;
	size_t length;

	// Whether or not the upvalue is currently open.
	bool open;

	// The position on the stack of the first local of function the upvalue was
	// defined in. Set when the function this upvalue is defined in is called.
	uint32_t fn_stack_start;

	union {
		// The stack position of the upvalue, relative to the function the
		// local the upvalue closes over was defined in, used when the upvalue
		// is open.
		uint16_t slot;

		// The value of the upvalue, used once its been closed.
		uint64_t value;
	};
} Upvalue;


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

	// A list of upvalues defined in this function. Used when the function is
	// called, in order to set the stack start of each of the upvalues.
	Upvalue *defined_upvalues[MAX_UPVALUES_IN_FN];
	uint32_t defined_upvalues_count;
} Function;


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
	ARRAY(uint64_t, numbers);

	// Strings encountered during compilation.
	ARRAY(uint64_t, strings);

	// Upvalues found during compilation.
	ARRAY(Upvalue, upvalues);

	// Struct definitions encountered during compilation.
	ARRAY(StructDefinition, structs);

	// Struct field names encountered during compilation.
	ARRAY(Identifier, fields);

	// The most recent error triggered on the VM.
	HyError err;
} VirtualMachine;


// Adds a string to the VM's strings list. Returns the index of the added
// string.
uint16_t vm_add_string(VirtualMachine *vm, char *string);

// Returns the string at `index` in the VM's strings list.
char * vm_string(VirtualMachine *vm, int index);

// Adds a number to the VM's numbers list. Returns the index of the added
// number.
uint16_t vm_add_number(VirtualMachine *vm, double number);

// Adds a field name to the VM's struct field names list. Returns the index of
// the added name.
uint16_t vm_add_field(VirtualMachine *vm, Identifier field);

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
HyResult fn_exec(VirtualMachine *vm, uint16_t main_fn);

#endif
