
//
//  Virtual Machine
//

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "err.h"


// Executes a file by creating a new interpreter state, reading the contents of
// the file, and executing the source code. Acts as a wrapper around other API
// functions. Returns an error if one occurred, or NULL otherwise. The error
// must be freed by calling `hy_err_free`.
HyError * hy_run_file(char *path) {
	HyState *state = hy_new();
	char *name = hy_package_name(path);
	HyPackage pkg = hy_package_new(state, name);
	HyError *err = hy_package_run_file(state, pkg, path);
	free(name);
	hy_free(state);
	return err;
}


// Executes some source code from a string. Returns an error if one occurred, or
// NULL otherwise. The error must be freed by calling `hy_err_free`.
HyError * hy_run_string(char *source) {
	HyState *state = hy_new();
	HyPackage pkg = hy_package_new(state, NULL);
	HyError *err = hy_package_run_string(state, pkg, source);
	hy_free(state);
	return err;
}


// Create a new interpreter state.
HyState * hy_new(void) {
	HyState *state = malloc(sizeof(HyState));
	vec_new(state->packages, Package, 4);
	vec_new(state->functions, Function, 8);
	vec_new(state->natives, NativeFunction, 8);
	vec_new(state->structs, StructDefinition, 8);
	vec_new(state->constants, HyValue, 32);
	vec_new(state->strings, String *, 16);
	vec_new(state->fields, Identifier, 16);
	state->error = NULL;
	return state;
}


// Release all resources allocated by an interpreter state.
void hy_free(HyState *state) {
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

	vec_free(state->packages);
	vec_free(state->functions);
	vec_free(state->natives);
	vec_free(state->structs);
	vec_free(state->constants);
	vec_free(state->strings);
	vec_free(state->fields);
	free(state);
}


// Resets an interpreter state's error, returning the current error.
HyError * vm_reset_error(HyState *state) {
	HyError *err = state->error;
	state->error = NULL;
	return err;
}


// Parses and runs some source code.
HyError * vm_parse_and_run(HyState *state, Package *pkg, Index source) {
	// Parse the source code
	Index main_fn;
	HyError *err = pkg_parse(pkg, source, &main_fn);
	if (err != NULL) {
		return err;
	}

	// Execute the main function
	return vm_run_fn(state, main_fn);
}


// Execute a file on a package. The file's contents will be read and executed
// as source code. The file's path will be used in relevant errors. An error
// object is returned if one occurs, otherwise NULL is returned.
HyError * hy_package_run_file(HyState *state, HyPackage index, char *path) {
	Package *pkg = &vec_at(state->packages, index);
	Index source = pkg_add_file(pkg, path);

	// Check we could find the file
	if (source == NOT_FOUND) {
		// Failed to open file
		return err_failed_to_open_file(path);
	}

	return vm_parse_and_run(state, pkg, source);
}


// Execute some source code on a package. An error object is returned if one
// occurs, otherwise NULL is returned.
HyError * hy_package_run_string(HyState *state, HyPackage index, char *source) {
	Package *pkg = &vec_at(state->packages, index);
	Index source_index = pkg_add_string(pkg, source);
	return vm_parse_and_run(state, pkg, source_index);
}


// Adds a constant to the interpreter state, returning its index.
Index state_add_constant(HyState *state, HyValue constant) {
	vec_add(state->constants);
	vec_last(state->constants) = constant;
	return vec_len(state->constants) - 1;
}


// Creates a new string constant that is `length` bytes long.
Index state_add_string(HyState *state, uint32_t length) {
	vec_add(state->strings);
	vec_last(state->strings) = malloc(sizeof(String) + length);
	String *string = vec_last(state->strings);
	string->length = length;
	string->contents[0] = '\0';
	return vec_len(state->strings) - 1;
}


// Adds a field name to the interpreter state's fields list. If a field matching
// `ident` already exists, then it returns the index of the existing field.
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
	vec_add(state->fields);
	Identifier *last = &vec_last(state->fields);
	last->name = ident.name;
	last->length = ident.length;
	return vec_len(state->fields) - 1;
}



//
//  Execution
//

// The maximum stack size.
#define MAX_STACK_SIZE 2048

// The maximum call stack size storing data for function calls.
#define MAX_CALL_STACK_SIZE 2048

// Triggers the goto call for the next instruction.
#define DISPATCH() goto *dispatch_table[ins_arg(*ip, 0)];

// Increments the instruction pointer and dispatches the next instruction.
#define NEXT() ip++; DISPATCH();

// Evaluates to the value on the stack at the `n`th argument.
#define STACK_GET(n) stack[stack_start + ins_arg(*ip, (n))]


// Data required for a function call.
typedef struct {
	// A pointer to the function being executed in this frame.
	Function *fn;

	// The start of the function's locals on the stack (absolute stack position)
	uint32_t stack_start;

	// The absolute position on the stack where the function's return value
	// should be stored.
	uint32_t return_slot;

	// The saved instruction pointer, pointing to the next bytecode instruction
	// to be executed in this function.
	Instruction *ip;
} Frame;


// Pushes a call frame onto the call frame stack.
static inline void frame_push(Frame *stack, uint32_t *size, Function *fn,
		uint32_t stack_start, uint32_t return_slot, Instruction *ip) {
	Index index = (*size)++;
	stack[index].fn = fn;
	stack[index].stack_start = stack_start;
	stack[index].return_slot = return_slot;
	stack[index].ip = ip;
}


// Ensures a value is a number, triggering an error if this is not the case.
static inline double ensure_num(HyValue value) {
	return value;
}


// Ensures a value is a string, triggering an error if this is not the case.
static inline String * ensure_str(HyValue value) {
	return (String *) val_to_ptr(value);
}


// Compares two strings for equality.
static inline bool string_comp(String *left, String *right) {
	return strcmp(left->contents, right->contents) == 0;
}


// Forward declaration.
static inline bool val_comp(StructDefinition *structs, HyValue left,
	HyValue right);


// Compares two structs for equality.
static inline bool struct_comp(StructDefinition *structs, Struct *left,
		Struct *right) {
	// Only equal if both are instances of the same struct
	if (left->definition != right->definition) {
		return false;
	}

	// Compare each field
	for (uint32_t i = 0; i < vec_len(structs[left->definition].fields); i++) {
		if (!val_comp(structs, left->fields[i], right->fields[i])) {
			return false;
		}
	}

	// All fields are equal
	return true;
}


// Compares two values for equality.
static inline bool val_comp(StructDefinition *structs, HyValue left,
		HyValue right) {
	return left == right ||
		(val_is_str(left) && val_is_str(right) &&
			string_comp(val_to_ptr(left), val_to_ptr(right))) ||
		(val_is_struct(left) && val_is_struct(right) &&
			struct_comp(structs, val_to_ptr(left), val_to_ptr(right)));
}


// Executes a function on the interpreter state.
HyError * vm_run_fn(HyState *state, Index fn_index) {
	// Indexed labels for computed gotos, used to increase performance by using
	// the CPU's branch predictor
	static void *dispatch_table[] = {
		// Storage
		&&BC_MOV_LL, &&BC_MOV_LI, &&BC_MOV_LN, &&BC_MOV_LS, &&BC_MOV_LP,
		&&BC_MOV_LF, &&BC_MOV_LV,
		// &&BC_MOV_UL, &&BC_MOV_UI, &&BC_MOV_UN, &&BC_MOV_US, &&BC_MOV_UP,
		// &&BC_MOV_UF, &&BC_MOV_UV,
		// &&BC_MOV_LU, &&BC_UPVALUE_CLOSE,
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
		&&BC_CALL, &&BC_RET0, &&BC_RET,

		// Structs
		// &&BC_STRUCT_NEW, &&BC_STRUCT_FIELD,
		// &&BC_STRUCT_SET_L, &&BC_STRUCT_SET_I, &&BC_STRUCT_SET_N,
		// &&BC_STRUCT_SET_S, &&BC_STRUCT_SET_P, &&BC_STRUCT_SET_F,
		// &&BC_STRUCT_SET_V,
	};

	// Cache pointers to arrays on the interpreter state
	Package *packages = &vec_at(state->packages, 0);
	Function *functions = &vec_at(state->functions, 0);
	NativeFunction *natives = &vec_at(state->natives, 0);
	StructDefinition *structs = &vec_at(state->structs, 0);

	HyValue *constants = &vec_at(state->constants, 0);
	String **strings = &vec_at(state->strings, 0);
	Identifier *fields = &vec_at(state->fields, 0);

	// Get a pointer to the function we're executing
	Function *fn = &functions[fn_index];

	// Allocate the variable and function call stack
	// Stack overflow checks on the variable stack are done during compile time,
	// so we only need to watch the function call stack
	HyValue *stack = malloc(sizeof(HyValue) * MAX_STACK_SIZE);
	Frame *call_stack = malloc(sizeof(Frame) * MAX_CALL_STACK_SIZE);
	uint32_t call_stack_size = 0;

	// The current instruction we're executing
	Instruction *ip = &vec_at(fn->instructions, 0);

	// The starting location of the current function's local variables on the
	// stack
	uint32_t stack_start = 0;

	// Add the call frame for the main function
	frame_push(call_stack, &call_stack_size, fn, 0, 0, ip);

	// Execute the first instruction
	DISPATCH();


	//
	//  Stack Storage
	//

BC_MOV_LL:
	STACK_GET(1) = STACK_GET(2);
	NEXT();

BC_MOV_LI:
	STACK_GET(1) = int_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_LN:
	STACK_GET(1) = constants[ins_arg(*ip, 2)];
	NEXT();

BC_MOV_LS:
	STACK_GET(1) = ptr_to_val(string_copy(strings[ins_arg(*ip, 2)]));
	NEXT();

BC_MOV_LP:
	STACK_GET(1) = prim_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_LF:
	STACK_GET(1) = fn_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_LV:
	STACK_GET(1) = native_to_val(ins_arg(*ip, 2));
	NEXT();


	//
	//  Upvalue Storage
	//

	// TODO


	//
	//  Top Level Local Storage
	//

BC_MOV_TL:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) = STACK_GET(2);
	NEXT();

BC_MOV_TI:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		int_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_TN:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		constants[ins_arg(*ip, 2)];
	NEXT();

BC_MOV_TS:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		ptr_to_val(string_copy(strings[ins_arg(*ip, 2)]));
	NEXT();

BC_MOV_TP:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		prim_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_TF:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		fn_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_TV:
	vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 1)) =
		native_to_val(ins_arg(*ip, 2));
	NEXT();

BC_MOV_LT:
	STACK_GET(1) = vec_at(packages[ins_arg(*ip, 3)].locals, ins_arg(*ip, 2));
	NEXT();


	//
	//  Arithmetic
	//

	// Since arithmetic instructions are all in the same form, use a define to
	// generate code for each operator
#define COMMA ,
#define ARITH(ins, operator, fn)                                  \
	BC_ ## ins ## _LL:                                            \
		STACK_GET(1) = num_to_val(fn(                             \
			ensure_num(STACK_GET(2)) operator                     \
			ensure_num(STACK_GET(3))                              \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _LI:                                            \
		STACK_GET(1) = num_to_val(fn(                             \
			ensure_num(STACK_GET(2)) operator                     \
			(double) unsigned_to_signed(ins_arg(*ip, 3))          \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _LN:                                            \
		STACK_GET(1) = num_to_val(fn(                             \
			ensure_num(STACK_GET(2)) operator                     \
			val_to_num(constants[ins_arg(*ip, 3)])                \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _IL:                                            \
		STACK_GET(1) = num_to_val(fn(                             \
			(double) unsigned_to_signed(ins_arg(*ip, 3)) operator \
			ensure_num(STACK_GET(2))                              \
		));                                                       \
		NEXT();                                                   \
                                                                  \
	BC_ ## ins ## _NL:                                            \
		STACK_GET(1) = num_to_val(fn(                             \
			val_to_num(constants[ins_arg(*ip, 3)]) operator       \
			ensure_num(STACK_GET(2))                              \
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
	STACK_GET(1) = ptr_to_val(string_concat(
		ensure_str(ins_arg(*ip, 2)), ensure_str(ins_arg(*ip, 3))
	));
	NEXT();

BC_CONCAT_LS:
	STACK_GET(1) = ptr_to_val(string_concat(
		ensure_str(ins_arg(*ip, 2)), strings[ins_arg(*ip, 3)]
	));
	NEXT();

BC_CONCAT_SL:
	STACK_GET(1) = ptr_to_val(string_concat(
		strings[ins_arg(*ip, 2)], ensure_str(ins_arg(*ip, 3))
	));
	NEXT();


	//
	//  Negation
	//

BC_NEG_L:
	STACK_GET(1) = num_to_val(-ensure_num(STACK_GET(2)));
	NEXT();


	//
	//  Equality
	//

BC_IS_TRUE_L: {
	HyValue arg = STACK_GET(1);
	if (arg == VALUE_FALSE || arg == VALUE_NIL) {
		ip++;
	}
	NEXT();
}

BC_IS_FALSE_L: {
	HyValue arg = STACK_GET(1);
	if (arg != VALUE_FALSE && arg != VALUE_NIL) {
		ip++;
	}
	NEXT();
}

	// Since equality and inequality comparisons are nearly identical, generate
	// the code for each using a macro.
#define EQ(ins, op)                                                     \
	BC_ ## ins ## _LL: {                                                \
		if (op val_comp(structs, STACK_GET(1), STACK_GET(2))) {         \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
	}                                                                   \
                                                                        \
	BC_ ## ins ## _LI:                                                  \
		if (op (STACK_GET(1) == int_to_val(ins_arg(*ip, 2)))) {         \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LN:                                                  \
		if (op (STACK_GET(1) == constants[ins_arg(*ip, 3)])) {          \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LS:                                                  \
		if (op (val_is_str(STACK_GET(1)) && string_comp(                \
				val_to_ptr(STACK_GET(1)), strings[ins_arg(*ip, 2)]))) { \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LP:                                                  \
		if (op (STACK_GET(1) == prim_to_val(ins_arg(*ip, 2)))) {        \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LF:                                                  \
		if (op (val_to_fn(STACK_GET(1)) == ins_arg(*ip, 2))) {          \
			ip++;                                                       \
		}                                                               \
		NEXT();                                                         \
                                                                        \
	BC_ ## ins ## _LV:                                                  \
		if (op (val_to_native(STACK_GET(1)) == ins_arg(*ip, 2)))  {     \
			ip++;                                                       \
		}                                                               \
		NEXT();

	// Use the opposite comparison operation because we want to execute the jump
	// only if the comparison is true
	EQ(EQ, !);
	EQ(NEQ, );


	//
	//  Ordering
	//

	// Use a macro to generate code for each of the order instructions.
#define ORD(ins, op)                                                \
	BC_ ## ins ## _LL:                                              \
		if (ensure_num(STACK_GET(1)) op ensure_num(STACK_GET(2))) { \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LI:                                              \
		if (ensure_num(STACK_GET(1)) op                             \
				(double) unsigned_to_signed(ins_arg(*ip, 2))) {     \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LN:                                              \
		if (ensure_num(STACK_GET(1)) op                             \
				val_to_num(constants[ins_arg(*ip, 2)])) {           \
			ip++;                                                   \
		}                                                           \
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

BC_CALL:
BC_RET0:
	goto finish;
BC_RET:
	goto finish;

finish:
	printf("success!!\n");
	return NULL;
}
