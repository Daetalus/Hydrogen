
//
//  Virtual Machine
//

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "parser/parser.h"
#include "parser/import.h"
#include "error.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"


// Create a new interpreter state.
HyVM * hy_new(void) {
	VirtualMachine *vm = malloc(sizeof(VirtualMachine));
	vm->err = NULL;

	// Allocate memory for arrays
	ARRAY_INIT(vm->functions, Function, 4);
	ARRAY_INIT(vm->packages, Package, 4);
	ARRAY_INIT(vm->native_packages, HyNativePackage, 4);
	ARRAY_INIT(vm->native_fns, NativeFn, 4);
	ARRAY_INIT(vm->numbers, HyValue, 16);
	ARRAY_INIT(vm->strings, HyValue, 16);
	ARRAY_INIT(vm->upvalues, Upvalue, 4);
	ARRAY_INIT(vm->structs, StructDefinition, 2);
	ARRAY_INIT(vm->fields, Identifier, 4);

	return (HyVM *) vm;
}


// Free an interpreter's state.
void hy_free(HyVM *vm) {
	// Packages
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];
		package_free(package);
	}

	// Native packages
	for (uint32_t i = 0; i < vm->native_packages_count; i++) {
		HyNativePackage *package = &vm->native_packages[i];
		native_package_free(package);
	}

	// Native functions
	for (uint32_t i = 0; i < vm->native_fns_count; i++) {
		NativeFn *fn = &vm->native_fns[i];
		native_fn_free(fn);
	}

	// Strings
	for (uint32_t i = 0; i < vm->strings_count; i++) {
		free(value_to_ptr(vm->strings[i]));
	}

	// Functions
	for (uint32_t i = 0; i < vm->functions_count; i++) {
		Function *fn = &vm->functions[i];
		fn_free(fn);
	}

	// Structs
	for (uint32_t i = 0; i < vm->structs_count; i++) {
		StructDefinition *def = &vm->structs[i];
		struct_free(def);
	}

	// Arrays
	free(vm->packages);
	free(vm->functions);
	free(vm->native_packages);
	free(vm->native_fns);
	free(vm->numbers);
	free(vm->strings);
	free(vm->upvalues);
	free(vm->structs);
	free(vm->fields);

	// The VM itself
	free(vm);
}



//
//  Execution
//

// Runs the interpreter with the given package as the main package.
HyError * hy_exec(HyVM *vm, Package *main) {
	if (setjmp(vm->error_jump) == 0) {
		// Compile the source
		parse_package(vm, main);
	}

	// Check for a compile error
	if (vm->err != NULL) {
		return vm->err;
	}

	// Execute the compiled bytecode
	return fn_exec(vm, main->main_fn);
}


// Runs the given source code string, returning a pointer to an error object
// if an error occurred, or NULL otherwise. The returned error object must be
// freed.
HyError * hy_run(HyVM *vm, char *source) {
	Package *main = package_new(vm, NULL);

	// Copy across the source code
	main->source = malloc(sizeof(char) * (strlen(source) + 1));
	strcpy(main->source, source);

	// Run
	return hy_exec(vm, main);
}


// Runs a file, returning an error if one occurred, or NULL otherwise.
HyError * hy_run_file(HyVM *vm, char *path) {
	Package *main = package_new(vm, NULL);

	// Copy the path into the package
	main->file = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	strcpy(main->file, path);

	// Read the source code
	main->source = read_file(main->file);
	if (main->source == NULL) {
		err_new(vm, "Failed to open file `%s`", path);
		return vm->err;
	}

	// Get the package name from the path
	main->name = import_package_name(main->file);

	// Run
	return hy_exec(vm, main);
}


// Directly triggers an error.
void hy_trigger_error(HyVM *vm, char *message) {
	err_fatal(vm, message);
}


// Run the garbage collector.
void hy_collect_garbage(HyVM *vm) {
	// TODO
}



//
//  Constants
//

// Adds a string to the VM's strings list. Returns the index of the added
// string.
uint16_t vm_add_string(VirtualMachine *vm, char *string) {
	uint16_t index = vm->strings_count++;
	ARRAY_REALLOC(vm->strings, HyValue);

	Object *obj = malloc(sizeof(Object) + sizeof(char) * (strlen(string) + 1));
	obj->type = OBJ_STRING;
	strcpy(&obj->string[0], string);
	vm->strings[index] = ptr_to_value(obj);
	return index;
}


// Returns the string at `index` in the VM's strings list.
char * vm_string(VirtualMachine *vm, int index) {
	Object *obj = value_to_ptr(vm->strings[index]);
	return &obj->string[0];
}


// Adds a number to the VM's numbers list. Returns the index of the added
// number.
uint16_t vm_add_number(VirtualMachine *vm, double number) {
	uint16_t index = vm->numbers_count++;
	ARRAY_REALLOC(vm->numbers, HyValue);
	vm->numbers[index] = number_to_value(number);
	return index;
}


// Adds a field name to the VM's struct field names list. Returns the index of
// the added name.
uint16_t vm_add_field(VirtualMachine *vm, Identifier field) {
	// Check if a field already exists
	for (uint32_t i = 0; i < vm->fields_count; i++) {
		Identifier *ident = &vm->fields[i];
		if (ident->length == field.length &&
				strncmp(ident->start, field.start, field.length) == 0) {
			return i;
		}
	}

	// Doesn't exist, so create it
	uint16_t index = vm->fields_count++;
	ARRAY_REALLOC(vm->fields, Identifier);
	vm->fields[index] = field;
	return index;
}



//
//  Packages
//

// Defines a new package.
Package * package_new(VirtualMachine *vm, uint32_t *requested) {
	// Increment the size of the packages array
	uint32_t index = vm->packages_count++;
	ARRAY_REALLOC(vm->packages, Package);
	if (requested != NULL) {
		*requested = index;
	}

	// Initialise the package
	Package *package = &vm->packages[index];
	package->name = NULL;
	package->file = NULL;
	package->source = NULL;
	package->main_fn = 0;
	ARRAY_INIT(package->functions, Function *, 4);
	ARRAY_INIT(package->structs, StructDefinition *, 4);
	ARRAY_INIT(package->locals, Identifier, 4);
	ARRAY_INIT(package->values, HyValue, 4);
	return package;
}


// Frees a package.
void package_free(Package *package) {
	if (package->file != NULL) {
		free(package->file);
	}
	if (package->name != NULL) {
		free(package->name);
	}
	free(package->source);
	free(package->functions);
	free(package->structs);
}


// Returns the index of a package with the given name, or -1 if one couldn't be
// found.
int package_find(VirtualMachine *vm, char *name, size_t length) {
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];

		// Check the length of the package name and the name itself are equal
		if (strlen(package->name) == length &&
				strncmp(package->name, name, length) == 0) {
			return i;
		}
	}

	// Couldn't find a package with the given name
	return -1;
}


// Creates a new top level local on a package, returning its index.
int package_local_new(Package *package, char *name, size_t length) {
	int index = package->locals_count++;
	package->values_count++;
	ARRAY_REALLOC(package->locals, Identifier);
	ARRAY_REALLOC(package->values, HyValue);

	package->values[index] = NIL_VALUE;
	Identifier *ident = &package->locals[index];
	ident->start = name;
	ident->length = length;
	return index;
}


// Returns the index of a package's top level variable with the given name, or
// -1 if no such local could be found.
int package_local_find(Package *package, char *name, size_t length) {
	for (uint32_t i = 0; i < package->locals_count; i++) {
		Identifier *ident = &package->locals[i];
		if (ident->length == length &&
				strncmp(ident->start, name, length) == 0) {
			return i;
		}
	}
	return -1;
}



//
//  Native Packages
//

// Define a native package on an interpreter with the given name.
HyNativePackage * hy_package_new(HyVM *vm, char *name) {
	// TODO: Ensure the package name contains only ASCII characters
	int index = vm->native_packages_count++;
	ARRAY_REALLOC(vm->native_packages, HyNativePackage);
	HyNativePackage *package = &vm->native_packages[index];
	package->vm = vm;
	ARRAY_INIT(package->functions, NativeFn *, 4);

	// Make a heap allocated copy of the string, because God knows what the user
	// has given us
	package->name = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(package->name, name);

	return package;
}


// Frees a native package.
void native_package_free(HyNativePackage *package) {
	free(package->name);
	free(package->functions);
}


// Finds a native package, returning its index, or -1 if it couldn't be found.
int native_package_find(VirtualMachine *vm, char *name, size_t length) {
	for (uint32_t i = 0; i < vm->native_packages_count; i++) {
		HyNativePackage *pkg = &vm->native_packages[i];
		if (length == strlen(pkg->name) &&
				strncmp(pkg->name, name, length) == 0) {
			return i;
		}
	}
	return -1;
}



//
//  Native Functions
//

// Frees a native function.
void native_fn_free(NativeFn *fn) {
	free(fn->name);
}


// Finds a native function in a native package, returning its index, or -1 if
// no such function could be found.
int native_fn_find(HyNativePackage *package, char *name, size_t length) {
	for (uint32_t i = 0; i < package->functions_count; i++) {
		NativeFn *fn = package->functions[i];
		if (length == strlen(fn->name) && strncmp(name, fn->name, length) == 0) {
			// Find the index of the function in the VM's functions list
			return fn - package->vm->native_fns;
		}
	}
	return -1;
}


// Defines a native function on a package with the given name and number of
// arguments. If `arity` is -1, then the function can accept an arbitrary
// number of arguments.
void hy_fn_new(HyNativePackage *package, char *name, int arity, HyNativeFn fn) {
	int index = package->vm->native_fns_count++;
	ARRAY_REALLOC(package->vm->native_fns, NativeFn);
	NativeFn *native = &package->vm->native_fns[index];
	native->arity = arity;
	native->fn = fn;

	// Make a heap allocated copy of the name
	native->name = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(native->name, name);

	// Add the function to the package's function list
	index = package->functions_count++;
	ARRAY_REALLOC(package->functions, NativeFn *);
	package->functions[index] = native;
}


// Returns the number of arguments supplied to a function.
uint32_t hy_args_count(HyArgs *args) {
	return args->arity;
}


// Returns the `n`th argument supplied to a native function.
HyValue hy_arg(HyArgs *args, uint32_t n) {
	if (n >= args->arity) {
		return NIL_VALUE;
	} else {
		return args->stack[args->stack_start + n];
	}
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
	fn->defined_upvalues_count = 0;
	ARRAY_INIT(fn->bytecode, uint64_t, 64);

	// Add the function to the package's function list
	if (package != NULL) {
		uint32_t package_index = package->functions_count++;
		ARRAY_REALLOC(package->functions, Function *);
		package->functions[package_index] = fn;
	}

	return fn;
}


// Frees resources allocated by a function.
void fn_free(Function *fn) {
	free(fn->bytecode);
}


// Returns the index of a function with the given name in the VM's function
// list, or -1 if one couldn't be found.
int fn_find(VirtualMachine *vm, char *name, size_t length) {
	// Functions that are defined recently are more likely to be used sooner
	// (maybe?), so search the array in reverse order
	for (int i = vm->functions_count; i >= 0; i--) {
		Function *fn = &vm->functions[i];

		// Check if the length of the function's name matches, along with the
		// name itself
		if (fn->length == length && strncmp(fn->name, name, length) == 0) {
			return i;
		}
	}

	// Couldn't find a function with the given name
	return -1;
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
	upvalue->fn_stack_start = 0;
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
StructDefinition * struct_new(VirtualMachine *vm, Package *package) {
	int index = vm->structs_count++;
	ARRAY_REALLOC(vm->structs, StructDefinition);

	// Initialise the struct definition
	StructDefinition *def = &vm->structs[index];
	def->name = NULL;
	def->package = package;
	def->length = 0;
	def->constructor = -1;
	ARRAY_INIT(def->fields, Identifier, 2);
	ARRAY_INIT(def->values, HyValue, 2);

	// Add the package to the struct's package list
	if (package != NULL) {
		int index = package->structs_count++;
		ARRAY_REALLOC(package->structs, StructDefinition *);
		package->structs[index] = def;
	}

	return def;
}


// Frees a struct.
void struct_free(StructDefinition *def) {
	free(def->fields);
	free(def->values);
}


// Creates a new field on a struct definition, returning its index.
int struct_new_field(StructDefinition *def) {
	int index = def->fields_count++;
	def->values_count++;
	ARRAY_REALLOC(def->fields, Identifier);
	ARRAY_REALLOC(def->values, HyValue);

	Identifier *field = &def->fields[index];
	field->start = NULL;
	field->length = 0;
	return index;
}


// Returns the index of a struct definition in the VM's struct definition list
// with the given name, or -1 if one doesn't exist.
int struct_find(VirtualMachine *vm, char *name, size_t length) {
	for (int i = vm->structs_count - 1; i >= 0; i--) {
		StructDefinition *def = &vm->structs[i];
		if (def->length == length && strncmp(def->name, name, length) == 0) {
			// Found requested struct
			return i;
		}
	}

	// Couldn't find the struct
	return -1;
}



//
//  Execution
//

// The maximum number of locals that can be defined on the stack.
#define MAX_STACK_SIZE 2048

// The maximum number of function frames that can exist on the call stack. This
// is the recursion depth limit.
#define MAX_CALL_STACK_SIZE 1024


// Evaluates to the opcode of an instruction.
#define INSTR_OPCODE(instr) ((instr) & 0xff)

// Evaluates to the `n`th argument of an instruction.
#define INSTR_ARG0(instr) (((instr) & (uint64_t) 0xff00) >> 8)
#define INSTR_ARG1(instr) (((instr) & (uint64_t) 0xffff0000) >> 16)
#define INSTR_ARG2(instr) (((instr) & (uint64_t) 0xffff00000000) >> 32)
#define INSTR_ARG3(instr) (((instr) & (uint64_t) 0xffff000000000000) >> 48)

// Shortcut for the `n`th argument of the current instruction.
#define ARG0 INSTR_ARG0(*ip)
#define ARG1 INSTR_ARG1(*ip)
#define ARG2 INSTR_ARG2(*ip)
#define ARG3 INSTR_ARG3(*ip)

// Shortcut for the `n`th argument, treating it as a local.
#define ARG1_L (stack[stack_start + ARG1])
#define ARG2_L (stack[stack_start + ARG2])
#define ARG3_L (stack[stack_start + ARG3])


// Converts an argument (uint16) to a number (double).
#define INTEGER_TO_DOUBLE(integer) \
	number_to_value((double) uint16_to_int16(integer))


// Triggers an error if an argument is not a number.
#define ENSURE_NUMBER(arg)            \
	if (!IS_NUMBER_VALUE(arg)) {      \
		err = ERR_INVALID_OPERAND; \
		goto error;                   \
	}

#define ENSURE_NUMBERS(arg1, arg2)                          \
	if (!IS_NUMBER_VALUE(arg1) || !IS_NUMBER_VALUE(arg2)) { \
		err = ERR_INVALID_OPERAND;                          \
		goto error;                                         \
	}


// Triggers an error if an argument is not a string.
#define ENSURE_STR(arg)            \
	if (!IS_STRING_VALUE(arg)) {   \
		err = ERR_INVALID_OPERAND; \
		goto error;                \
	}

#define ENSURE_STRS(arg1, arg2)                              \
	if  (!IS_STRING_VALUE(arg1) || !IS_STRING_VALUE(arg2)) { \
		err = ERR_INVALID_OPERAND;                           \
		goto error;                                          \
	}


// Triggers an error if an argument isn't a function.
#define ENSURE_FN(arg)        \
	if (!IS_FN_VALUE(arg)) {  \
		err = ERR_INVALID_FN; \
		goto error;           \
	}


// Triggers an error if an argument isn't an object.
#define ENSURE_OBJECT(arg)                                        \
	if (!IS_PTR_VALUE(arg) ||                                     \
			((Object *) value_to_ptr(arg))->type != OBJ_STRUCT) { \
		err = ERR_INVALID_FIELD_ACCESS;                           \
		goto error;                                               \
	}


// Shorthand for defining a set of arithmetic operations.
#define ARITHMETIC_OPERATION(prefix, op)                    \
	case prefix ## _LL:                                     \
		ENSURE_NUMBERS(ARG2_L, ARG3_L);                     \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			value_to_number(ARG3_L));                       \
		NEXT();                                             \
	case prefix ## _LI:                                     \
		ENSURE_NUMBER(ARG2_L);                              \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			INTEGER_TO_DOUBLE(ARG3));                       \
		NEXT();                                             \
	case prefix ## _LN:                                     \
		ENSURE_NUMBER(ARG2_L);                              \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			numbers[ARG3]);                                 \
		NEXT();                                             \
	case prefix ## _IL:                                     \
		ENSURE_NUMBER(ARG3_L);                              \
		ARG1_L = number_to_value(INTEGER_TO_DOUBLE(ARG2) op \
			value_to_number(ARG3_L));                       \
		NEXT();                                             \
	case prefix ## _NL:                                     \
		ENSURE_NUMBER(ARG3_L);                              \
		ARG1_L = number_to_value(numbers[ARG2] op           \
			value_to_number(ARG3_L));                       \
		NEXT();


// Shorthand for defining a set of equality operations.
#define EQUALITY_OPERATION(prefix, binary, unary)                            \
	case prefix ## _LL:                                                      \
		if ((ARG1_L binary ARG2_L) ||                                        \
				unary (IS_STRING_VALUE(ARG1_L) && IS_STRING_VALUE(ARG2_L) && \
					strcmp(TO_STR(ARG1_L), TO_STR(ARG2_L)) == 0) ||          \
				unary (IS_PTR_VALUE(ARG1_L) && IS_PTR_VALUE(ARG1_L) &&       \
					FIELDS_COUNT(ARG1_L) == FIELDS_COUNT(ARG2_L) &&          \
					memcmp(TO_FIELDS(ARG1_L), TO_FIELDS(ARG2_L),             \
						FIELDS_COUNT(ARG1_L) * sizeof(uint64_t)) == 0)) {    \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                              \
	case prefix ## _LI:                                                      \
		if (ARG1_L binary (double) uint16_to_int16(ARG2)) {                  \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                              \
	case prefix ## _LN:                                                      \
		if (ARG1_L binary numbers[ARG2]) {                                   \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                              \
	case prefix ## _LS:                                                      \
		if (unary (IS_STRING_VALUE(ARG1_L) &&                                \
				strcmp(TO_STR(ARG2_L), TO_STR(strings[ARG3])) == 0)) {       \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                              \
	case prefix ## _LP:                                                      \
		if (ARG1_L binary PRIMITIVE_FROM_TAG(ARG2)) {                        \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                              \
	case prefix ## _LF:                                                      \
		if (unary (IS_FN_VALUE(ARG1_L) &&                                    \
				VALUE_TO_INDEX(ARG1_L, FN_TAG) == ARG2)) {                   \
			ip++;                                                            \
		}                                                                    \
		NEXT();


// Shorthand for defining an order operation.
#define ORDER_OPERATION(prefix, operator)                     \
	case prefix ## _LL:                                       \
		ENSURE_NUMBERS(ARG1_L, ARG2_L);                       \
		if (ARG1_L operator ARG2_L) {                         \
			ip++;                                             \
		}                                                     \
		NEXT();                                               \
	case prefix ## _LI:                                       \
		ENSURE_NUMBER(ARG1_L);                                \
		if (ARG1_L operator (double) uint16_to_int16(ARG2)) { \
			ip++;                                             \
		}                                                     \
		NEXT();                                               \
	case prefix ## _LN:                                       \
		ENSURE_NUMBER(ARG1_L);                                \
		if (ARG1_L operator numbers[ARG2]) {                  \
			ip++;                                             \
		}                                                     \
		NEXT();


// Call a function.
#define CALL(fn_index, call_arity, argument_start, fn_return_slot)         \
	frames[frames_count - 1].ip = ip;                                      \
	fn = &functions[fn_index];                                             \
	if (fn->arity != (call_arity)) {                                       \
		err = ERR_INCORRECT_ARITY;                                         \
		goto error;                                                        \
	}                                                                      \
	frames_count++;                                                        \
	ip = fn->bytecode;                                                     \
	frames[frames_count - 1].stack_start = stack_start + (argument_start); \
	frames[frames_count - 1].return_slot = stack_start + (fn_return_slot); \
	stack_start = frames[frames_count - 1].stack_start;                    \
	for (uint32_t i = 0; i < fn->defined_upvalues_count; i++) {            \
		fn->defined_upvalues[i]->fn_stack_start = stack_start;             \
	}


// Return from a function.
#define RETURN(value)                                   \
	frames_count--;                                     \
	if (frames_count == 0) {                            \
		goto finish;                                    \
	}                                                   \
	ip = frames[frames_count - 1].ip;                   \
	stack_start = frames[frames_count - 1].stack_start; \
	stack[frames[frames_count - 1].return_slot] = (value);


// Sets the field of a struct.
#define STRUCT_SET_FIELD(value) {                                             \
	ENSURE_OBJECT(ARG1_L);                                                    \
	Identifier *ident = &struct_fields[ARG2];                                 \
	Object *obj = (Object *) value_to_ptr(ARG1_L);                            \
	StructDefinition *def = obj->obj.definition;                              \
	for (uint32_t i = 0; i < def->fields_count; i++) {                        \
		if (ident->length == def->fields[i].length &&                         \
				strncmp(ident->start, def->fields[i].start, ident->length)) { \
			obj->obj.fields[i] = (value);                                     \
			NEXT();                                                           \
		}                                                                     \
	}                                                                         \
	err = ERR_NO_SUCH_FIELD;                                                  \
	goto error;                                                               \
}


// Goes to the next instruction.
#define NEXT() \
	ip++;      \
	goto instruction;


// The stack slot of an open upvalue.
#define UPVALUE_STACK_SLOT(index) \
	(upvalues[index].fn_stack_start + upvalues[index].slot)


// Concatenates two strings.
char * concat_str(char *left, char *right) {
	int length = strlen(left);
	char *str = malloc(sizeof(char) * (length + strlen(right) + 1));
	strcpy(str, left);
	strcpy(&str[length], right);
	return str;
}


// A function frame on the call stack.
typedef struct {
	// The start of the function's locals on the stack (absolute stack position)
	uint32_t stack_start;

	// The absolute position on the stack where the function's return value
	// should be stored.
	uint32_t return_slot;

	// The saved instruction pointer, pointing to the next bytecode instruction
	// to be executed in this function.
	uint64_t *ip;
} Frame;


// Possible runtime errors.
typedef enum {
	ERR_INVALID_OPERAND,
	ERR_INVALID_FN,
	ERR_INCORRECT_ARITY,
	ERR_INVALID_FIELD_ACCESS,
	ERR_NO_SUCH_FIELD,
} RuntimeError;


// Executes a compiled function on the virtual machine.
HyError * fn_exec(VirtualMachine *vm, uint16_t main_fn) {
	Function *fn = &vm->functions[main_fn];
	// debug_print_bytecode(fn);

	// The variable stack
	HyValue *stack = malloc(sizeof(HyValue) * MAX_STACK_SIZE);

	// The function frame stack
	Frame *frames = malloc(sizeof(Frame) * MAX_CALL_STACK_SIZE);
	uint32_t frames_count = 0;

	// Cache from the VM
	Package *packages = vm->packages;
	NativeFn *native_fns = vm->native_fns;
	Upvalue *upvalues = vm->upvalues;
	HyValue *numbers = vm->numbers;
	HyValue *strings = vm->strings;
	Function *functions = vm->functions;
	StructDefinition *structs = vm->structs;
	Identifier *struct_fields = vm->fields;

	// The instruction pointer for the currently executing function (the top
	// most on the call frame stack)
	uint64_t *ip = NULL;

	// The start of the currently executing function's locals on the stack
	uint32_t stack_start = 0;

	// The type of a runtime error, if one is triggered
	RuntimeError err;

	// Push the main function's call frame
	frames_count++;
	ip = fn->bytecode;
	frames[frames_count - 1].stack_start = 0;

	// Main execution loop
instruction:
	switch (INSTR_OPCODE(*ip)) {

	//
	//  Storage
	//

	case MOV_LL:
		ARG1_L = ARG2_L;
		NEXT();
	case MOV_LI:
		ARG1_L = INTEGER_TO_DOUBLE(ARG2);
		NEXT();
	case MOV_LN:
		ARG1_L = numbers[ARG2];
		NEXT();
	case MOV_LS:
		ARG1_L = strings[ARG2];
		NEXT();
	case MOV_LP:
		ARG1_L = PRIMITIVE_FROM_TAG(ARG2);
		NEXT();
	case MOV_LF:
		ARG1_L = INDEX_TO_VALUE(ARG2, FN_TAG);
		NEXT();

	case MOV_LU:
		if (upvalues[ARG2].open) {
			ARG1_L = stack[UPVALUE_STACK_SLOT(ARG2)];
		} else {
			ARG1_L = upvalues[ARG2].value;
		}
		NEXT();
	case MOV_UL:
		if (upvalues[ARG1].open) {
			stack[UPVALUE_STACK_SLOT(ARG1)] = ARG2_L;
		} else {
			upvalues[ARG1].value = ARG2_L;
		}
		NEXT();

	case MOV_LT:
		ARG1_L = packages[ARG2].values[ARG3];
		NEXT();
	case MOV_TL:
		packages[ARG2].values[ARG1] = ARG3_L;
		NEXT();


	//
	//  Math
	//

	ARITHMETIC_OPERATION(ADD, +)
	ARITHMETIC_OPERATION(SUB, -)
	ARITHMETIC_OPERATION(MUL, *)
	ARITHMETIC_OPERATION(DIV, / )

	case MOD_LL:
		ENSURE_NUMBERS(ARG2_L, ARG3_L);
		ARG1_L = number_to_value(fmod(value_to_number(ARG2_L),
			value_to_number(ARG3_L)));
		NEXT();
	case MOD_LI:
		ENSURE_NUMBER(ARG2_L);
		ARG1_L = number_to_value(fmod(value_to_number(ARG2_L),
			INTEGER_TO_DOUBLE(ARG3)));
		NEXT();
	case MOD_LN:
		ENSURE_NUMBER(ARG2_L);
		ARG1_L = number_to_value(fmod(value_to_number(ARG2_L),
			numbers[ARG3]));
		NEXT();
	case MOD_IL:
		ENSURE_NUMBER(ARG3_L);
		ARG1_L = number_to_value(fmod(INTEGER_TO_DOUBLE(ARG2),
			value_to_number(ARG3_L)));
		NEXT();
	case MOD_NL:
		ENSURE_NUMBER(ARG3_L);
		ARG1_L = number_to_value(fmod(
			numbers[ARG2],
			value_to_number(ARG3_L)
		));
		NEXT();

	case CONCAT_LL:
		ENSURE_STRS(ARG2_L, ARG3_L);
		ARG1_L = ptr_to_value(concat_str(
			value_to_ptr(ARG2_L),
			value_to_ptr(ARG3_L)
		));
		NEXT();
	case CONCAT_LS:
		ENSURE_STR(ARG2_L);
		ARG1_L = ptr_to_value(concat_str(
			value_to_ptr(ARG2_L),
			TO_STR(strings[ARG3])
		));
		NEXT();
	case CONCAT_SL:
		ENSURE_STR(ARG2_L);
		ARG1_L = ptr_to_value(concat_str(
			TO_STR(strings[ARG2]),
			value_to_ptr(ARG3_L)
		));
		NEXT();

	case NEG_L:
		ENSURE_NUMBER(ARG1_L);
		ARG1_L = number_to_value(-value_to_number(ARG1_L));
		NEXT();


	//
	//  Comparison
	//

	// Since all comparisons are followed by a jump instruction, and the jump
	// instruction must be executed only if the comparison is true, just skip
	// the jump instruction (by incrementing the instruction pointer), if the
	// comparison is false.

	case IS_TRUE_L:
		if (ARG1_L != TRUE_VALUE) {
			ip++;
		}
		NEXT();
	case IS_FALSE_L:
		if (ARG1_L == TRUE_VALUE) {
			ip++;
		}
		NEXT();

	EQUALITY_OPERATION(EQ, !=, !)
	EQUALITY_OPERATION(NEQ, ==, )
	ORDER_OPERATION(LT, >=)
	ORDER_OPERATION(LE, >)
	ORDER_OPERATION(GT, <=)
	ORDER_OPERATION(GE, <)


	//
	//  Control Flow
	//

	case JMP:
		ip += ARG2;
		NEXT();
	case LOOP:
		ip -= ARG2;
		ip++;
		NEXT();


	//
	//  Functions
	//

	case CALL_L:
		ENSURE_FN(ARG1_L);
		CALL(ARG0, VALUE_TO_INDEX(ARG1_L, FN_TAG), ARG2, ARG3);
		goto instruction;
	case CALL_F:
		CALL(ARG0, ARG1, ARG2, ARG3);
		goto instruction;
	case CALL_NATIVE: {
		HyArgs args;
		args.arity = ARG0;
		args.stack = stack;
		args.stack_start = stack_start + ARG2;
		ARG3_L = native_fns[ARG1].fn(vm, &args);
		NEXT();
	}

	case RET:
		RETURN(NIL_VALUE);
		NEXT();
	case RET_L:
		RETURN(ARG1_L);
		NEXT();
	case RET_I:
		RETURN(INTEGER_TO_DOUBLE(ARG1));
		NEXT();
	case RET_N:
		RETURN(numbers[ARG1]);
		NEXT();
	case RET_S:
		RETURN(strings[ARG1]);
		NEXT();
	case RET_P:
		RETURN(PRIMITIVE_FROM_TAG(ARG1));
		NEXT();
	case RET_F:
		RETURN(INDEX_TO_VALUE(ARG1, FN_TAG));
		NEXT();


	//
	//  Upvalues
	//

	case UPVALUE_CLOSE:
		upvalues[ARG1].open = false;
		upvalues[ARG1].value = stack[UPVALUE_STACK_SLOT(ARG1)];
		NEXT();


	//
	//  Structs
	//

	case STRUCT_NEW: {
		StructDefinition *def = &structs[ARG2];
		Object *obj = malloc(sizeof(Object) + sizeof(HyValue) *
			def->fields_count);
		obj->type = OBJ_STRUCT;
		obj->obj.definition = def;
		memcpy(obj->obj.fields, def->values, sizeof(HyValue) *
			def->fields_count);
		ARG1_L = ptr_to_value(obj);
		NEXT();
	}

	case STRUCT_FIELD: {
		ENSURE_OBJECT(ARG2_L);
		Identifier *ident = &struct_fields[ARG3];
		Object *obj = (Object *) value_to_ptr(ARG2_L);
		StructDefinition *def = obj->obj.definition;

		// Look for the field
		for (uint32_t i = 0; i < def->fields_count; i++) {
			if (ident->length == def->fields[i].length &&
					strncmp(ident->start, def->fields[i].start, ident->length)) {
				ARG1_L = obj->obj.fields[i];
				NEXT();
			}
		}

		// Couldn't find the field on the struct
		err = ERR_NO_SUCH_FIELD;
		goto error;
	}

	case STRUCT_SET_L:
		STRUCT_SET_FIELD(ARG3_L);
	case STRUCT_SET_I:
		STRUCT_SET_FIELD(INTEGER_TO_DOUBLE(ARG3));
	case STRUCT_SET_N:
		STRUCT_SET_FIELD(numbers[ARG3]);
	case STRUCT_SET_S:
		STRUCT_SET_FIELD(strings[ARG3]);
	case STRUCT_SET_P:
		STRUCT_SET_FIELD(PRIMITIVE_FROM_TAG(ARG3));
	case STRUCT_SET_F:
		STRUCT_SET_FIELD(INDEX_TO_VALUE(ARG3, FN_TAG));
	}

finish:
	return NULL;

error:
	printf("FAILED! %d\n", err);
	return NULL;
}
