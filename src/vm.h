
//
//  Virtual Machine
//


#ifndef VM_H
#define VM_H

#include <stdlib.h>

#include "lexer.h"
#include "bytecode.h"
#include "value.h"


// The maximum number of functions a program can define.
#define MAX_FUNCTIONS 65535

// The maximum number of native functions a program can define.
#define MAX_NATIVES 65535

// The maximum number of arguments that can be passed to a
// function.
#define MAX_ARGUMENTS 32

// The maximum number of constant string literals that can exist
// in a program.
#define MAX_STRING_LITERALS 65535

// The maximum number of upvalues that can be in scope at any
// point.
#define MAX_UPVALUES 65535

// The maximum number of locals that can be used as upvalues.
#define MAX_USED_UPVALUES 512

// The maximum number of class definitions that can be created.
#define MAX_CLASSES 65535

// The maximum number of fields that can be defined on a class.
#define MAX_FIELDS 256

// The maximum number of methods that can be defined on a class.
#define MAX_METHODS 128


// A struct storing a string with an associated length, rather
// than terminated with a NULL byte.
typedef struct {
	// The length of the string in the source code.
	int length;

	// The pointer into the source code specifying the start of
	// the string.
	char *location;
} SourceString;


// An upvalue captured by a closure. An upvalue is a local from
// outside a function's scope used inside the function. This is
// special because when a function call finishes, its locals are
// destroyed. If a closure is still using one of its locals,
// we'll get a bunch of segmentation faults.
//
// Upvalues have 2 states, open and closed. Open upvalues are
// where the original local they close over is still in scope,
// and modification should modify that local. Upvalues are
// closed when their original variable is destroyed. When this
// happens, the virtual machine copies out the value and puts it
// into the `value` field to allow it to persist.
typedef struct {
	// True if the upvalue is closed.
	bool closed;

	// The index of the local this upvalue closes over inside
	// the function that defines it.
	//
	// Determined at compile time.
	int local_index;

	// The index of the defining function on the stack during
	// runtime. The sum of this and the `local_index` field give
	// the position of the upvalue on the stack.
	//
	// Determined at runtime.
	int function_index;

	// The value of this upvalue when it is closed.
	uint64_t value;

	// The name of the upvalue, used for comparison against
	// identifiers to check that we haven't already created an
	// upvalue for a local. This is set to NULL when the upvalue
	// is closed, in order to avoid collisions against future
	// upvalues with the same name.
	char *name;

	// The length of the name, as the name is a pointer into the
	// source code.
	int length;

	// A pointer to the function that defined this upvalue as a
	// local.
	struct function *defining_function;
} Upvalue;


// A user-defined function within Hydrogen source code.
typedef struct function {
	// The function's compiled bytecode.
	Bytecode bytecode;

	// The names of the arguments passed to the function, used
	// when loading the arguments as locals during compilation.
	SourceString arguments[MAX_ARGUMENTS];
	int arity;

	// Pointers to all the upvalues captured by this function.
	Upvalue *captured_upvalues[MAX_USED_UPVALUES];
	int captured_upvalue_count;

	// Pointers to all the upvalues defined as locals in this
	// function.
	Upvalue *defined_upvalues[MAX_USED_UPVALUES];
	int defined_upvalue_count;
} Function;


// The definition for a native function (a function that calls
// into C from Hydrogen code).
typedef void (*NativeFunction)(uint64_t *stack, int *stack_size);


// A native C function.
typedef struct {
	// The name of this function, as a NULL terminated string.
	char *name;

	// The number of arguments this function takes.
	int arity;

	// The C function to call as part of this function.
	NativeFunction fn;
} Native;


// A field in a class definition.
typedef struct {
	// The name of the field, as a pointer into the source code
	// string.
	char *name;

	// The length of the name string.
	int length;

	// If this field represents a method, this is set to the
	// index of the method in the class' methods list, or to -1
	// otherwise.
	int method_index;
} Field;


// A method in a class instance and definition.
typedef struct {
	// A pointer to the class instance this method belongs to.
	//
	// Don't use the typedef'ed value because the struct hasn't
	// been defined yet, and C allows pointers to forward
	// declared structs.
	//
	// This is set to NULL when this struct is used in a method
	// definition, and only set at runtime when a class is
	// instantiated.
	struct class_instance *instance;

	// The index of the method's function in the VM's functions
	// list.
	int function_index;
} Method;


// A class definition, constructed during compilation.
typedef struct {
	// The name of the class, as a pointer into the source code.
	char *name;

	// The length of the class' name.
	int length;

	// A list of methods defined on this class.
	Method methods[MAX_METHODS];
	int method_count;

	// A list of all fields defined on the class.
	Field fields[MAX_FIELDS];
	int field_count;
} ClassDefinition;


// An instance of a class, heap allocated during runtime.
typedef struct class_instance {
	// A pointer to the definition that this object is an
	// instance of.
	ClassDefinition *definition;

	// A list of all methods defined on this class, which method
	// fields point to.
	Method methods[MAX_METHODS];

	// The fields for this class, indexed in the same order as
	// the `fields` list defined in the class definition.
	//
	// This uses the C struct "hack", similar to strings, where
	// we allocate extra space after the class on the heap for
	// the fields.
	uint64_t fields[0];
} ClassInstance;


// Executes compiled bytecode.
typedef struct {
	// A lexer, producing a stream of tokens from the source
	// code.
	Lexer lexer;

	// An array of functions defined during compilation. Stored
	// as a heap allocated array, so we don't use up stack space
	// and can have a practically unlimited number of functions.
	//
	// The main function (for all code outside of function
	// definitions) will be the first function in this array.
	Function *functions;
	int function_count;
	int function_capacity;

	// An array of native functions defined by libraries in C.
	// Heap allocated, so we don't use up stack space and can
	// have a practically unlimited number of native functions.
	Native *natives;
	int native_count;
	int native_capacity;

	// An array of string literal constants encountered in the
	// source code.
	String **literals;
	int literal_count;
	int literal_capacity;

	// An array of upvalues in use by all closures.
	Upvalue *upvalues;
	int upvalue_count;
	int upvalue_capacity;

	// An array of all classes defined in the source code.
	ClassDefinition *class_definitions;
	int class_definition_count;
	int class_definition_capacity;
} VirtualMachine;


// Create a new virtual machine with `source` as the program's
// source code.
//
// Nothing is compiled or run until `vm_compile` and `vm_run`
// are called.
VirtualMachine vm_new(char *source);

// Free any resources allocated by the VM.
void vm_free(VirtualMachine *vm);

// Compiles the source code into bytecode.
void vm_compile(VirtualMachine *vm);

// Runs the compiled bytecode.
void vm_run(VirtualMachine *vm);

// Attach the whole standard library to the virtual machine.
void vm_attach_standard_library(VirtualMachine *vm);

// Attach the IO module in the standard library to the virtual
// machine.
void vm_attach_io(VirtualMachine *vm);

// Attach a native function to the virtual machine, which acts
// as a library function when running the Hydrogen source.
void vm_attach_native(VirtualMachine *vm, char *name, int arity,
	NativeFunction fn);


// Defines a new function, returning a pointer to it and its
// index in the VM's function list.
//
// Performs no allocation, so the returned function's bytecode
// object still needs to be allocated.
int vm_new_function(VirtualMachine *vm, Function **fn);


// Defines a new native function, returning a pointer to it and
// its index in the VM's native function list.
int vm_new_native(VirtualMachine *vm, Native **native);

// Returns the index of the native function named `name`, or -1
// if no function is found.
int vm_find_native(VirtualMachine *vm, char *name, int length);


// Create a new class definition, returning its index in the
// VM's class definitions list.
int vm_new_class_definition(VirtualMachine *vm, ClassDefinition **definition);

// Returns the index of the class named `name`, or -1 if no
// class with that name if found.
int vm_find_class(VirtualMachine *vm, char *name, int length);


// Create a new string literal, returning a pointer to it and
// its index in the literals list.
int vm_new_string_literal(VirtualMachine *vm, String ***literal);

// Create a new upvalue, returning a pointer to it and its index
// in the upvalues list.
int vm_new_upvalue(VirtualMachine *vm, Upvalue **upvalue);

#endif
