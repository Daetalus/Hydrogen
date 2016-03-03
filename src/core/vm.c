
//
//  Virtual Machine
//

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "err.h"
#include "debug.h"


// The maximum stack size
#define MAX_STACK_SIZE 2048

// The maximum call stack size storing data for function calls
#define MAX_CALL_STACK_SIZE 2048


// Executes a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`
HyError * hy_run_file(HyState *state, char *path) {
	char *name = hy_pkg_name(path);
	HyPackage pkg = hy_add_pkg(state, name);
	HyError *err = hy_pkg_run_file(state, pkg, path);
	free(name);
	return err;
}


// Executes some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`
HyError * hy_run_string(HyState *state, char *source) {
	HyPackage pkg = hy_add_pkg(state, NULL);
	HyError *err = hy_pkg_run_string(state, pkg, source);
	return err;
}


// Create a new interpreter state
HyState * hy_new(void) {
	HyState *state = malloc(sizeof(HyState));

	vec_new(state->sources, Source, 4);
	vec_new(state->packages, Package, 4);
	vec_new(state->functions, Function, 8);
	vec_new(state->natives, NativeFunction, 8);
	vec_new(state->structs, StructDefinition, 8);

	vec_new(state->constants, HyValue, 32);
	vec_new(state->strings, String *, 16);
	vec_new(state->fields, Identifier, 16);

	state->stack = malloc(sizeof(HyValue) * MAX_STACK_SIZE);
	state->call_stack = malloc(sizeof(Frame) * MAX_CALL_STACK_SIZE);
	state->call_stack_count = 0;

	state->error = NULL;
	return state;
}


// Release all resources allocated by an interpreter state
void hy_free(HyState *state) {
	for (uint32_t i = 0; i < vec_len(state->sources); i++) {
		Source *src = &vec_at(state->sources, i);
		free(src->file);
		free(src->contents);
	}
	for (uint32_t i = 0; i < vec_len(state->packages); i++) {
		pkg_free(&vec_at(state->packages, i));
	}
	for (uint32_t i = 0; i < vec_len(state->functions); i++) {
		fn_free(&vec_at(state->functions, i));
	}
	for (uint32_t i = 0; i < vec_len(state->natives); i++) {
		native_free(&vec_at(state->natives, i));
	}
	for (uint32_t i = 0; i < vec_len(state->structs); i++) {
		struct_free(&vec_at(state->structs, i));
	}
	for (uint32_t i = 0; i < vec_len(state->strings); i++) {
		free(vec_at(state->strings, i));
	}

	vec_free(state->sources);
	vec_free(state->packages);
	vec_free(state->functions);
	vec_free(state->natives);
	vec_free(state->structs);
	vec_free(state->constants);
	vec_free(state->strings);
	vec_free(state->fields);
	free(state->stack);
	free(state->call_stack);
	free(state);
}


// Resets an interpreter state's error, returning the current error
HyError * state_reset_error(HyState *state) {
	HyError *err = state->error;
	state->error = NULL;
	return err;
}


// Parses and runs some source code
HyError * vm_parse_and_run(HyState *state, HyPackage pkg_index, Index source) {
	Package *pkg = &vec_at(state->packages, pkg_index);

	// Parse the source code
	Index main_fn = 0;
	HyError *err = pkg_parse(pkg, source, &main_fn);

	// Execute the main function if no error occurred
	if (err == NULL) {
		err = vm_run_fn(state, main_fn);
	}
	return err;
}


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned
HyError * hy_pkg_run_file(HyState *state, HyPackage pkg, char *path) {
	Index source = state_add_source_file(state, path);

	// Check we could find the file
	if (source == NOT_FOUND) {
		// Failed to open file
		return err_failed_to_open_file(path);
	}

	return vm_parse_and_run(state, pkg, source);
}


// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned
HyError * hy_pkg_run_string(HyState *state, HyPackage pkg, char *source) {
	Index source_index = state_add_source_string(state, source);
	return vm_parse_and_run(state, pkg, source_index);
}


// Adds a constant to the interpreter state, returning its index
Index state_add_constant(HyState *state, HyValue constant) {
	vec_inc(state->constants);
	vec_last(state->constants) = constant;
	return vec_len(state->constants) - 1;
}


// Creates a new string constant that is `length` bytes long
Index state_add_literal(HyState *state, uint32_t length) {
	vec_inc(state->strings);
	vec_last(state->strings) = malloc(sizeof(String) + length + 1);
	String *string = vec_last(state->strings);
	string->type = OBJ_STRING;
	string->length = length;
	string->contents[0] = '\0';
	return vec_len(state->strings) - 1;
}


// Adds a field name to the interpreter state's fields list. If a field matching
// `ident` already exists, then it returns the index of the existing field
Index state_add_field(HyState *state, Identifier ident) {
	// Check for an existing field first (reverse order)
	for (int i = vec_len(state->fields) - 1; i >= 0; i--) {
		Identifier *match = &vec_at(state->fields, (uint32_t) i);
		if (ident.length == match->length &&
				strncmp(ident.name, match->name, ident.length) == 0) {
			return i;
		}
	}

	// No existing field, so add a new one
	vec_inc(state->fields);
	Identifier *last = &vec_last(state->fields);
	last->name = ident.name;
	last->length = ident.length;
	return vec_len(state->fields) - 1;
}


// Returns the contents of a file
static char * file_contents(char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		return NULL;
	}

	// Get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);

	// Read its contents
	char *contents = malloc(sizeof(char) * (length + 1));
	fread(contents, sizeof(char), length, f);
	fclose(f);
	contents[length] = '\0';
	return contents;
}


// Adds a file as a source code object on the interpreter
Index state_add_source_file(HyState *state, char *path) {
	// Read the contents of the file
	char *contents = file_contents(path);
	if (contents == NULL) {
		return NOT_FOUND;
	}

	vec_inc(state->sources);
	Source *src = &vec_last(state->sources);
	src->contents = contents;

	// Copy the file path into our own heap allocated string
	src->file = malloc(strlen(path) + 1);
	strcpy(src->file, path);
	return vec_len(state->sources) - 1;
}


// Adds a string as a source code object on the interpreter
Index state_add_source_string(HyState *state, char *source) {
	vec_inc(state->sources);
	Source *src = &vec_last(state->sources);
	src->file = NULL;

	// Copy the source code into our own heap allocated string
	src->contents = malloc(strlen(source) + 1);
	strcpy(src->contents, source);
	return vec_len(state->sources) - 1;
}



//
//  Execution
//

// Triggers the goto call for the next instruction
#define DISPATCH() goto *dispatch_table[ins_arg(*ip, 0)];

// Increments the instruction pointer and dispatches the next instruction
#define NEXT() ip++; DISPATCH();

// Evaluates to the value in the `n`th stack slot, relative to the current
// function's stack start
#define STACK_GET(n) stack[stack_start + (n)]

// Evaluates to the value on the stack at the `n`th argument of the current
// instruction
#define STACK_GET_INS(n) STACK_GET(ins_arg(*ip, n))


// Ensures a value is a number, triggering an error if this is not the case
static inline double ensure_num(HyValue value) {
	if (!val_is_num(value)) {
		printf("Ensure num failed!\n");
		exit(1);
	}
	return val_to_num(value);
}


// Ensures a value is a string, triggering an error if this is not the case
static inline String * ensure_str(HyValue value) {
	if (!val_is_str(value)) {
		printf("Ensure str failed!\n");
		exit(1);
	}
	return (String *) val_to_ptr(value);
}


// Compares two strings for equality
static inline bool string_comp(String *left, String *right) {
	return strcmp(left->contents, right->contents) == 0;
}


// Forward declaration
static inline bool val_comp(StructDefinition *structs, HyValue left,
	HyValue right);


// Compares two structs for equality
static bool struct_comp(StructDefinition *structs, Struct *left,
		Struct *right) {
	// Only equal if both are instances of the same struct
	if (left->definition != right->definition) {
		return false;
	}

	// Compare each field recursively
	for (uint32_t i = 0; i < vec_len(structs[left->definition].fields); i++) {
		// Since we're doing this recursively, we need to check if the user's
		// stored a reference to a struct in one of it's fields, so we don't
		// end up recursing infinitely
		// TODO
		if (!val_comp(structs, left->fields[i], right->fields[i])) {
			return false;
		}
	}

	// All fields are equal
	return true;
}


// Compares two values for equality
static inline bool val_comp(StructDefinition *structs, HyValue left,
		HyValue right) {
	return left == right ||
		(val_is_str(left) && val_is_str(right) &&
			string_comp(val_to_ptr(left), val_to_ptr(right))) ||
		(val_is_struct(left) && val_is_struct(right) &&
			struct_comp(structs, val_to_ptr(left), val_to_ptr(right)));
}


// Executes a function on the interpreter state
HyError * vm_run_fn(HyState *state, Index fn_index) {
	// Indexed labels for computed gotos, used to increase performance by using
	// the CPU's branch predictor
	static void *dispatch_table[] = {
		// Storage
		&&BC_MOV_LL, &&BC_MOV_LI, &&BC_MOV_LN, &&BC_MOV_LS, &&BC_MOV_LP,
		&&BC_MOV_LF, &&BC_MOV_LV,
		&&BC_MOV_UL, &&BC_MOV_UI, &&BC_MOV_UN, &&BC_MOV_US, &&BC_MOV_UP,
		&&BC_MOV_UF, &&BC_MOV_UV,
		&&BC_MOV_LU, &&BC_UPVALUE_CLOSE,
		&&BC_MOV_TL, &&BC_MOV_TI, &&BC_MOV_TN, &&BC_MOV_TS, &&BC_MOV_TP,
		&&BC_MOV_TF, &&BC_MOV_TV, &&BC_MOV_LT,

		// Operators
		&&BC_ADD_LL, &&BC_ADD_LI, &&BC_ADD_LN, &&BC_ADD_IL, &&BC_ADD_NL,
		&&BC_SUB_LL, &&BC_SUB_LI, &&BC_SUB_LN, &&BC_SUB_IL, &&BC_SUB_NL,
		&&BC_MUL_LL, &&BC_MUL_LI, &&BC_MUL_LN, &&BC_MUL_IL, &&BC_MUL_NL,
		&&BC_DIV_LL, &&BC_DIV_LI, &&BC_DIV_LN, &&BC_DIV_IL, &&BC_DIV_NL,
		&&BC_MOD_LL, &&BC_MOD_LI, &&BC_MOD_LN, &&BC_MOD_IL, &&BC_MOD_NL,
		&&BC_CONCAT_LL, &&BC_CONCAT_LS, &&BC_CONCAT_SL,
		&&BC_NEG_L,

		// Comparison
		&&BC_IS_TRUE_L, &&BC_IS_FALSE_L,
		&&BC_EQ_LL, &&BC_EQ_LI, &&BC_EQ_LN, &&BC_EQ_LS, &&BC_EQ_LP, &&BC_EQ_LF,
		&&BC_EQ_LV,
		&&BC_NEQ_LL, &&BC_NEQ_LI, &&BC_NEQ_LN, &&BC_NEQ_LS, &&BC_NEQ_LP,
		&&BC_NEQ_LF, &&BC_NEQ_LV,
		&&BC_LT_LL, &&BC_LT_LI, &&BC_LT_LN,
		&&BC_LE_LL, &&BC_LE_LI, &&BC_LE_LN,
		&&BC_GT_LL, &&BC_GT_LI, &&BC_GT_LN,
		&&BC_GE_LL, &&BC_GE_LI, &&BC_GE_LN,

		// Control flow
		&&BC_JMP, &&BC_LOOP,

		// Function calls
		&&BC_CALL, &&BC_RET0, &&BC_RET_L, &&BC_RET_I, &&BC_RET_N, &&BC_RET_S,
		&&BC_RET_P, &&BC_RET_F, &&BC_RET_V,

		// Structs
		&&BC_STRUCT_NEW, &&BC_STRUCT_FIELD,
		&&BC_STRUCT_SET_L, &&BC_STRUCT_SET_I, &&BC_STRUCT_SET_N,
		&&BC_STRUCT_SET_S, &&BC_STRUCT_SET_P, &&BC_STRUCT_SET_F,
		&&BC_STRUCT_SET_V,
	};

	// Cache pointers to arrays on the interpreter state
	Package *packages = &vec_at(state->packages, 0);
	Function *functions = &vec_at(state->functions, 0);
	NativeFunction *natives = &vec_at(state->natives, 0);
	StructDefinition *structs = &vec_at(state->structs, 0);

	HyValue *constants = &vec_at(state->constants, 0);
	String **strings = &vec_at(state->strings, 0);

	HyValue *stack = state->stack;
	Frame *call_stack = state->call_stack;
	uint32_t *call_stack_count = &state->call_stack_count;
	*call_stack_count = 0;

	// Get a pointer to the function we're executing
	Function *fn = &functions[fn_index];
	// debug_fn(state, fn);

	// The current instruction we're executing
	Instruction *ip = &vec_at(fn->instructions, 0);

	// The starting location of the current function's local variables on the
	// stack
	uint32_t stack_start = 0;

	// Execute the first instruction
	DISPATCH();


	//
	//  Stack Storage
	//

BC_MOV_LL:
	STACK_GET_INS(1) = STACK_GET_INS(2);
	NEXT();
BC_MOV_LI:
	STACK_GET_INS(1) = int_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_LN:
	STACK_GET_INS(1) = constants[ins_arg(*ip, 2)];
	NEXT();
BC_MOV_LS:
	STACK_GET_INS(1) = ptr_to_val(string_copy(strings[ins_arg(*ip, 2)]));
	NEXT();
BC_MOV_LP:
	STACK_GET_INS(1) = prim_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_LF:
	STACK_GET_INS(1) = fn_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_LV:
	STACK_GET_INS(1) = native_to_val(ins_arg(*ip, 2));
	NEXT();


	//
	//  Upvalue Storage
	//

BC_MOV_UL:
	NEXT();
BC_MOV_UI:
	NEXT();
BC_MOV_UN:
	NEXT();
BC_MOV_US:
	NEXT();
BC_MOV_UP:
	NEXT();
BC_MOV_UF:
	NEXT();
BC_MOV_UV:
	NEXT();

BC_MOV_LU:
	NEXT();
BC_UPVALUE_CLOSE:
	NEXT();


	//
	//  Top Level Local Storage
	//

// Shorthand method to retrive a top level local on a package
#define PKG_GET(pkg, field) \
	vec_at(packages[ins_arg(*ip, (pkg))].locals, ins_arg(*ip, (field)))

BC_MOV_TL:
	PKG_GET(3, 1) = STACK_GET_INS(2);
	NEXT();
BC_MOV_TI:
	PKG_GET(3, 1) = int_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_TN:
	PKG_GET(3, 1) = constants[ins_arg(*ip, 2)];
	NEXT();
BC_MOV_TS:
	PKG_GET(3, 1) = ptr_to_val(string_copy(strings[ins_arg(*ip, 2)]));
	NEXT();
BC_MOV_TP:
	PKG_GET(3, 1) = prim_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_TF:
	PKG_GET(3, 1) = fn_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_TV:
	PKG_GET(3, 1) = native_to_val(ins_arg(*ip, 2));
	NEXT();
BC_MOV_LT:
	STACK_GET_INS(1) = vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 2));
	NEXT();


	//
	//  Arithmetic
	//

	// Since arithmetic instructions are all in the same form, use a define to
	// generate code for each operator
#define COMMA ,
#define ARITH(ins, operator, fn)                                  \
	BC_ ## ins ## _LL:                                            \
		STACK_GET_INS(1) = num_to_val(fn(                         \
			ensure_num(STACK_GET_INS(2)) operator                 \
			ensure_num(STACK_GET_INS(3))                          \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _LI:                                            \
		STACK_GET_INS(1) = num_to_val(fn(                         \
			ensure_num(STACK_GET_INS(2)) operator                 \
			(double) unsigned_to_signed(ins_arg(*ip, 3))          \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _LN:                                            \
		STACK_GET_INS(1) = num_to_val(fn(                         \
			ensure_num(STACK_GET_INS(2)) operator                 \
			val_to_num(constants[ins_arg(*ip, 3)])                \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _IL:                                            \
		STACK_GET_INS(1) = num_to_val(fn(                         \
			(double) unsigned_to_signed(ins_arg(*ip, 2)) operator \
			ensure_num(STACK_GET_INS(3))                          \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _NL:                                            \
		STACK_GET_INS(1) = num_to_val(fn(                         \
			val_to_num(constants[ins_arg(*ip, 2)]) operator       \
			ensure_num(STACK_GET_INS(3))                          \
		));                                                       \
		NEXT();

	ARITH(ADD, +, );
	ARITH(SUB, -, );
	ARITH(MUL, *, );
	ARITH(DIV, /, );
	ARITH(MOD, COMMA, fmod);


	//
	//  Concatenation
	//

BC_CONCAT_LL:
	STACK_GET_INS(1) = ptr_to_val(string_concat(
		ensure_str(STACK_GET_INS(2)), ensure_str(STACK_GET_INS(3))
	));
	NEXT();

BC_CONCAT_LS:
	STACK_GET_INS(1) = ptr_to_val(string_concat(
		ensure_str(STACK_GET_INS(2)), strings[ins_arg(*ip, 3)]
	));
	NEXT();

BC_CONCAT_SL:
	STACK_GET_INS(1) = ptr_to_val(string_concat(
		strings[ins_arg(*ip, 2)], ensure_str(STACK_GET_INS(3))
	));
	NEXT();


	//
	//  Negation
	//

BC_NEG_L:
	STACK_GET_INS(1) = num_to_val(-ensure_num(STACK_GET_INS(2)));
	NEXT();


	//
	//  Equality
	//

BC_IS_TRUE_L: {
	HyValue arg = STACK_GET_INS(1);
	if (arg == VALUE_FALSE || arg == VALUE_NIL) {
		ip++;
	}
	NEXT();
}

BC_IS_FALSE_L: {
	HyValue arg = STACK_GET_INS(1);
	if (arg != VALUE_FALSE && arg != VALUE_NIL) {
		ip++;
	}
	NEXT();
}

	// Since equality and inequality comparisons are nearly identical, generate
	// the code for each using a macro
#define EQ(ins, op)                                                     \
	BC_ ## ins ## _LL: {                                                \
		if (op val_comp(structs, STACK_GET_INS(1), STACK_GET_INS(2))) { \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
	}                                                                   \
                                                                        \
	BC_ ## ins ## _LI:                                                  \
		if (op (STACK_GET_INS(1) == int_to_val(ins_arg(*ip, 2)))) {     \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LN:                                                  \
		if (op (STACK_GET_INS(1) == constants[ins_arg(*ip, 2)])) {      \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LS:                                                  \
		if (op (val_is_str(STACK_GET_INS(1)) &&                         \
				string_comp(val_to_ptr(STACK_GET_INS(1)),               \
					strings[ins_arg(*ip, 2)]))) {                       \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LP:                                                  \
		if (op (STACK_GET_INS(1) == prim_to_val(ins_arg(*ip, 2)))) {    \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LF:                                                  \
		if (op (val_to_fn(STACK_GET_INS(1)) == ins_arg(*ip, 2))) {      \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LV:                                                  \
		if (op (val_to_native(STACK_GET_INS(1)) == ins_arg(*ip, 2)))  { \
			ip++;                                                       \
		}                                                               \
		NEXT();

	// Use the opposite comparison operation because we want to execute the
	// jump only if the comparison is true
	EQ(EQ, !);
	EQ(NEQ, );


	//
	//  Ordering
	//

	// Use a macro to generate code for each of the order instructions
#define ORD(ins, op)                                                        \
	BC_ ## ins ## _LL:                                                      \
		if (ensure_num(STACK_GET_INS(1)) op ensure_num(STACK_GET_INS(2))) { \
			ip++;                                                           \
		}                                                                   \
		NEXT();                                                             \
                                                                            \
	BC_ ## ins ## _LI:                                                      \
		if (ensure_num(STACK_GET_INS(1)) op                                 \
				(double) unsigned_to_signed(ins_arg(*ip, 2))) {             \
			ip++;                                                           \
		}                                                                   \
		NEXT();                                                             \
                                                                            \
	BC_ ## ins ## _LN:                                                      \
		if (ensure_num(STACK_GET_INS(1)) op                                 \
				val_to_num(constants[ins_arg(*ip, 2)])) {                   \
			ip++;                                                           \
		}                                                                   \
		NEXT();

	// Again, use the opposite comparison operation
	ORD(LT, >=);
	ORD(LE, >);
	ORD(GT, <=);
	ORD(GE, <);


	//
	//  Control Flow
	//

BC_JMP:
	ip += ins_arg(*ip, 1);
	DISPATCH();

BC_LOOP:
	ip -= ins_arg(*ip, 1);
	DISPATCH();


	//
	//  Function Calls
	//

BC_CALL: {
	HyValue fn_value = STACK_GET_INS(1);
	if (val_is_fn(fn_value)) {
		// Create a stack frame for the calling function to save the required
		// state
		Index index = (*call_stack_count)++;
		call_stack[index].fn = fn;
		call_stack[index].stack_start = stack_start;
		call_stack[index].return_slot = stack_start + ins_arg(*ip, 3);
		call_stack[index].ip = ip;

		// Set up state for the called function
		stack_start = stack_start + ins_arg(*ip, 1) + 1;
		fn = &functions[val_to_fn(fn_value)];
		ip = &vec_at(fn->instructions, 0);
		DISPATCH();
	} else if (val_is_native(fn_value)) {
		// Create a set of arguments to pass to the native function
		HyArgs args;
		args.stack = stack;
		args.start = stack_start + ins_arg(*ip, 1) + 1;
		args.arity = ins_arg(*ip, 2);
		STACK_GET_INS(3) = natives[val_to_native(fn_value)].fn(state, &args);
		NEXT();
	} else {
		// TODO: trigger attempt to call non-function error
		printf("attempt to call non-function\n");
	}
}


// Shorthand for returning a value
#define RET(return_value) {                                \
	Index index = --(*call_stack_count);                   \
	stack[call_stack[index].return_slot] = (return_value); \
	stack_start = call_stack[index].stack_start;           \
	fn = call_stack[index].fn;                             \
	ip = call_stack[index].ip;                             \
	NEXT();                                                \
}

BC_RET0:
	// Check if we're returning from the main function (only needs to be done
	// from the RET0 instruction, because execution will stop on this
	// instruction when the main function finished)
	if (*call_stack_count == 0) {
		goto finish;
	}
	RET(VALUE_NIL);

BC_RET_L:
	RET(STACK_GET_INS(2));
BC_RET_I:
	RET(int_to_val(ins_arg(*ip, 2)));
BC_RET_N:
	RET(constants[ins_arg(*ip, 2)]);
BC_RET_S:
	RET(ptr_to_val(string_copy(strings[ins_arg(*ip, 2)])));
BC_RET_P:
	RET(prim_to_val(ins_arg(*ip, 2)));
BC_RET_F:
	RET(fn_to_val(ins_arg(*ip, 2)));
BC_RET_V:
	RET(native_to_val(ins_arg(*ip, 2)));


	//
	//  Structs
	//

BC_STRUCT_NEW:
	NEXT();
BC_STRUCT_FIELD:
	NEXT();
BC_STRUCT_SET_L:
	NEXT();
BC_STRUCT_SET_I:
	NEXT();
BC_STRUCT_SET_N:
	NEXT();
BC_STRUCT_SET_S:
	NEXT();
BC_STRUCT_SET_P:
	NEXT();
BC_STRUCT_SET_F:
	NEXT();
BC_STRUCT_SET_V:
	NEXT();

finish:
	return NULL;
}
