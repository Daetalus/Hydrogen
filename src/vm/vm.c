
//
//  Virtual Machine
//

#include <math.h>
#include <string.h>

#include "vm.h"
#include "parser.h"
#include "error.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"


// Create a new interpreter state.
HyVM * hy_new(void) {
	VirtualMachine *vm = malloc(sizeof(VirtualMachine));

	// Error
	vm->err.description = NULL;
	vm->err.line = 0;

	// Allocate memory for arrays
	ARRAY_INIT(vm->functions, Function, 4);
	ARRAY_INIT(vm->packages, Package, 4);
	ARRAY_INIT(vm->numbers, uint64_t, 16);
	ARRAY_INIT(vm->strings, uint64_t, 16);
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
	free(vm->numbers);
	free(vm->strings);
	free(vm->upvalues);
	free(vm->structs);
	free(vm->fields);

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
	ARRAY_REALLOC(vm->strings, uint64_t);

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
	ARRAY_REALLOC(vm->numbers, uint64_t);
	vm->numbers[index] = number_to_value(number);
	return index;
}


// Adds a field name to the VM's struct field names list. Returns the index of
// the added name.
uint16_t vm_add_field(VirtualMachine *vm, Identifier field) {
	uint16_t index = vm->fields_count++;
	ARRAY_REALLOC(vm->fields, Identifier);
	vm->fields[index] = field;
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
	ARRAY_INIT(def->values, uint64_t, 2);
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
	ARRAY_REALLOC(def->values, uint64_t);

	Identifier *field = &def->fields[index];
	field->start = NULL;
	field->length = 0;
	return index;
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


// Shorthand for defining a set of arithmetic operations.
#define ARITHMETIC_OPERATION(prefix, op)                    \
	case prefix ## _LL:                                     \
		ENSURE_NUMBERS(ARG2_L, ARG3_L);                     \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			value_to_number(ARG3_L));                       \
		NEXT();                                   \
	case prefix ## _LI:                                     \
		ENSURE_NUMBER(ARG2_L);                              \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			INTEGER_TO_DOUBLE(ARG3));                       \
		NEXT();                                   \
	case prefix ## _LN:                                     \
		ENSURE_NUMBER(ARG2_L);                              \
		ARG1_L = number_to_value(value_to_number(ARG2_L) op \
			numbers[ARG3]);                                 \
		NEXT();                                   \
	case prefix ## _IL:                                     \
		ENSURE_NUMBER(ARG3_L);                              \
		ARG1_L = number_to_value(INTEGER_TO_DOUBLE(ARG2) op \
			value_to_number(ARG3_L));                       \
		NEXT();                                   \
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
		NEXT();                                                    \
	case prefix ## _LI:                                                      \
		if (ARG1_L binary (double) uint16_to_int16(ARG2)) {                  \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                    \
	case prefix ## _LN:                                                      \
		if (ARG1_L binary numbers[ARG2]) {                                   \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                    \
	case prefix ## _LS:                                                      \
		if (unary (IS_STRING_VALUE(ARG1_L) &&                                \
				strcmp(TO_STR(ARG2_L), TO_STR(strings[ARG3])) == 0)) {       \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                    \
	case prefix ## _LP:                                                      \
		if (ARG1_L binary PRIMITIVE_FROM_TAG(ARG2)) {                        \
			ip++;                                                            \
		}                                                                    \
		NEXT();                                                    \
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
		NEXT();                                     \
	case prefix ## _LI:                                       \
		ENSURE_NUMBER(ARG1_L);                                \
		if (ARG1_L operator (double) uint16_to_int16(ARG2)) { \
			ip++;                                             \
		}                                                     \
		NEXT();                                     \
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
	stack_start += (argument_start);


// Return from a function.
#define RETURN(value)                                   \
	frames_count--;                                     \
	if (frames_count == 0) {                            \
		goto finish;                                    \
	}                                                   \
	ip = frames[frames_count - 1].ip;                   \
	stack_start = frames[frames_count - 1].stack_start; \
	stack[frames[frames_count - 1].return_slot] = (value);


// Goes to the next instruction.
#define NEXT() \
	ip++;      \
	goto instruction;


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
} RuntimeError;


// Executes a compiled function on the virtual machine.
HyResult fn_exec(VirtualMachine *vm, uint16_t main_fn) {
	Function *fn = &vm->functions[main_fn];
	debug_print_bytecode(fn);

	// The variable stack
	uint64_t *stack = malloc(sizeof(uint64_t) * MAX_STACK_SIZE);

	// The function frame stack
	Frame *frames = malloc(sizeof(Frame) * MAX_CALL_STACK_SIZE);
	uint32_t frames_count = 0;

	// Cache from the VM
	uint64_t *numbers = vm->numbers;
	uint64_t *strings = vm->strings;
	Function *functions = vm->functions;

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
		// TODO
		NEXT();
	case MOV_UL:
		// TODO
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

	case CONCAT_LL: {
		ENSURE_STRS(ARG2_L, ARG3_L);
		ARG1_L = ptr_to_value(concat_str(
			value_to_ptr(ARG2_L),
			value_to_ptr(ARG3_L)
		));
		NEXT();
	}
	case CONCAT_LS: {
		ENSURE_STR(ARG2_L);
		ARG1_L = ptr_to_value(concat_str(
			value_to_ptr(ARG2_L),
			TO_STR(strings[ARG3])
		));
		NEXT();
	}
	case CONCAT_SL: {
		ENSURE_STR(ARG2_L);
		ARG1_L = ptr_to_value(concat_str(
			TO_STR(strings[ARG2]),
			value_to_ptr(ARG3_L)
		));
		NEXT();
	}

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
		NEXT();


	//
	//  Functions
	//

	case CALL_L:
		ENSURE_FN(ARG1_L);
		CALL(ARG0, VALUE_TO_INDEX(ARG1_L, FN_TAG), ARG2, ARG3);
		NEXT();
	case CALL_F:
		CALL(ARG0, ARG1, ARG2, ARG3);
		NEXT();

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
	}

finish:
	printf("YAYYYYYYYYYYYYYYYYYYYY!!!\n");
	return HY_SUCCESS;

error:
	printf("FAILED! %d\n", err);
	return HY_RUNTIME_ERROR;
}
