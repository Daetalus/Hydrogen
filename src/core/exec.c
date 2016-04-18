
//
//  Execution
//

#include <math.h>

#include "exec.h"
#include "debug.h"


// Trigger the goto call for the next instruction.
#define DISPATCH() goto *dispatch_table[INS(0)];

// Increment the instruction pointer and dispatches the next instruction.
#define NEXT() ip++; DISPATCH();

// Will evaluate to the `n`th argument of the current instruction.
#define INS(n) (ins_arg(*ip, (n)))

// Will evaluates to the value in the `n`th stack slot, relative to the current
// function's stack start.
#define STACK(n) stack[stack_start + (n)]


// Ensure a value is a number, triggering an error if this is not the case.
static inline double ensure_num(HyValue value) {
	if (!val_is_num(value)) {
		// TODO: Proper error handling
		printf("Ensure num failed!\n");
		exit(1);
	}
	return val_to_num(value);
}


// Ensure a value is a string, triggering an error if this is not the case.
static inline String * ensure_str(HyValue value) {
	if (!val_is_gc(value, OBJ_STRING)) {
		// TODO: Proper error handling
		printf("Ensure str failed!\n");
		exit(1);
	}
	return (String *) val_to_ptr(value);
}


// Create a new instance of a struct.
static inline HyValue struct_instantiate(StructDefinition *structs,
		uint16_t index) {
	StructDefinition *def = &structs[index];

	// Create the instance
	uint32_t fields_size = sizeof(HyValue) * vec_len(def->fields);
	Struct *instance = malloc(sizeof(Struct) + fields_size);
	instance->type = OBJ_STRUCT;
	instance->definition = index;
	instance->fields_count = vec_len(def->fields);

	// Set the instance's fields
	HyValue parent = ptr_to_val(instance);
	for (uint32_t i = 0; i < vec_len(def->fields); i++) {
		Index fn_index = vec_at(def->methods, i);

		// Check if the field is a method on the struct
		if (fn_index != NOT_FOUND) {
			// Create the method
			Method *method = malloc(sizeof(Method));
			method->type = OBJ_METHOD;
			method->parent = parent;
			method->fn = fn_index;

			// Set the field
			instance->fields[i] = ptr_to_val(method);
		} else {
			// If it's not a method, set the field to nil
			instance->fields[i] = VALUE_NIL;
		}
	}

	return ptr_to_val(instance);
}


// Execute a function on the interpreter state.
HyError * exec_fn(HyState *state, Index fn_index) {
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
		&&BC_MOV_TF, &&BC_MOV_TV, &&BC_MOV_LT, &&BC_MOV_SELF,

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
		&&BC_STRUCT_NEW, &&BC_STRUCT_CALL_CONSTRUCTOR, &&BC_STRUCT_FIELD,
		&&BC_STRUCT_SET_L, &&BC_STRUCT_SET_I, &&BC_STRUCT_SET_N,
		&&BC_STRUCT_SET_S, &&BC_STRUCT_SET_P, &&BC_STRUCT_SET_F,
		&&BC_STRUCT_SET_V,

		// Arrays
		&&BC_ARRAY_NEW,
		&&BC_ARRAY_GET_L, &&BC_ARRAY_GET_I,
		&&BC_ARRAY_I_SET_L, &&BC_ARRAY_I_SET_I, &&BC_ARRAY_I_SET_N,
		&&BC_ARRAY_I_SET_S, &&BC_ARRAY_I_SET_P, &&BC_ARRAY_I_SET_F,
		&&BC_ARRAY_I_SET_V,
		&&BC_ARRAY_L_SET_L, &&BC_ARRAY_L_SET_I, &&BC_ARRAY_L_SET_N,
		&&BC_ARRAY_L_SET_S, &&BC_ARRAY_L_SET_P, &&BC_ARRAY_L_SET_F,
		&&BC_ARRAY_L_SET_V,
	};

	// Cache pointers to arrays on the interpreter state
	Package *packages = &vec_at(state->packages, 0);
	Function *functions = &vec_at(state->functions, 0);
	NativeFunction *native_fns = &vec_at(state->native_fns, 0);
	StructDefinition *structs = &vec_at(state->structs, 0);
	Identifier *fields = &vec_at(state->fields, 0);
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

	// Shorthand method for a set of 7 set instructions.
#define SET(prefix, fn)                               \
	BC_ ## prefix ## L:                               \
		fn(STACK(INS(2)));                            \
	BC_ ## prefix ## I:                               \
		fn(int_to_val(INS(2)));                       \
	BC_ ## prefix ## N:                               \
		fn(constants[INS(2)]);                        \
	BC_ ## prefix ## S:                               \
		fn(ptr_to_val(string_copy(strings[INS(2)]))); \
	BC_ ## prefix ## P:                               \
		fn(prim_to_val(INS(2)));                      \
	BC_ ## prefix ## F:                               \
		fn(fn_to_val(INS(2), TAG_FN));                \
	BC_ ## prefix ## V:                               \
		fn(fn_to_val(INS(2), TAG_NATIVE));            \

	// The set function for MOV_L* instructions.
#define MOV_L(value)       \
	STACK(INS(1)) = value; \
	NEXT();

	// All MOV_L* instructions.
	SET(MOV_L, MOV_L);

BC_MOV_SELF:
	STACK(INS(1)) = call_stack[*call_stack_count - 1].self;
	NEXT();


	//
	//  Upvalue Storage
	//

	// The set function for MOV_U* instructions.
#define MOV_U(value) \
	NEXT();

	// All MOV_U* instructions.
	SET(MOV_U, MOV_U);

BC_MOV_LU:
	NEXT();
BC_UPVALUE_CLOSE:
	NEXT();


	//
	//  Top Level Local Storage
	//

	// The set function for MOV_T* instructions.
#define MOV_T(value)                                 \
	vec_at(packages[INS(3)].locals, INS(1)) = value; \
	NEXT();

	// All MOV_T* instructions.
	SET(MOV_T, MOV_T);

BC_MOV_LT:
	STACK(INS(1)) = vec_at(packages[INS(3)].locals, INS(2));
	NEXT();


	//
	//  Arithmetic
	//

	// Since arithmetic instructions are all in the same form, use a define to
	// generate code for each operator.
#define COMMA ,
#define ARITH(ins, operator, fn)                         \
	BC_ ## ins ## _LL:                                   \
		STACK(INS(1)) = num_to_val(fn(                   \
			ensure_num(STACK(INS(2))) operator           \
			ensure_num(STACK(INS(3)))                    \
		));                                              \
		NEXT();                                          \
                                                         \
	BC_ ## ins ## _LI:                                   \
		STACK(INS(1)) = num_to_val(fn(                   \
			ensure_num(STACK(INS(2))) operator           \
			(double) unsigned_to_signed(INS(3))          \
		));                                              \
		NEXT();                                          \
                                                         \
	BC_ ## ins ## _LN:                                   \
		STACK(INS(1)) = num_to_val(fn(                   \
			ensure_num(STACK(INS(2))) operator           \
			val_to_num(constants[INS(3)])                \
		));                                              \
		NEXT();                                          \
                                                         \
	BC_ ## ins ## _IL:                                   \
		STACK(INS(1)) = num_to_val(fn(                   \
			(double) unsigned_to_signed(INS(2)) operator \
			ensure_num(STACK(INS(3)))                    \
		));                                              \
		NEXT();                                          \
                                                         \
	BC_ ## ins ## _NL:                                   \
		STACK(INS(1)) = num_to_val(fn(                   \
			val_to_num(constants[INS(2)]) operator       \
			ensure_num(STACK(INS(3)))                    \
		));                                              \
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
	STACK(INS(1)) = ptr_to_val(string_concat(
		ensure_str(STACK(INS(2))), ensure_str(STACK(INS(3)))
	));
	NEXT();

BC_CONCAT_LS:
	STACK(INS(1)) = ptr_to_val(string_concat(
		ensure_str(STACK(INS(2))), strings[INS(3)]
	));
	NEXT();

BC_CONCAT_SL:
	STACK(INS(1)) = ptr_to_val(string_concat(
		strings[INS(2)], ensure_str(STACK(INS(3)))
	));
	NEXT();


	//
	//  Negation
	//

BC_NEG_L:
	STACK(INS(1)) = num_to_val(-ensure_num(STACK(INS(2))));
	NEXT();


	//
	//  Equality
	//

BC_IS_TRUE_L:
	if (STACK(INS(1)) == VALUE_FALSE || STACK(INS(1)) == VALUE_NIL) {
		ip++;
	}
	NEXT();

BC_IS_FALSE_L:
	if (STACK(INS(1)) != VALUE_FALSE && STACK(INS(1)) != VALUE_NIL) {
		ip++;
	}
	NEXT();

	// Since equality and inequality comparisons are nearly identical, generate
	// the code for each using a macro.
#define EQ(ins, op)                                                 \
	BC_ ## ins ## _LL: {                                            \
		if (op val_cmp(STACK(INS(1)), STACK(INS(2)))) {             \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
	}                                                               \
                                                                    \
	BC_ ## ins ## _LI:                                              \
		if (op (STACK(INS(1)) == int_to_val(INS(2)))) {             \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LN:                                              \
		if (op (STACK(INS(1)) == constants[INS(2)])) {              \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LS:                                              \
		if (op (val_is_gc(STACK(INS(1)), OBJ_STRING) &&             \
				string_cmp(val_to_ptr(STACK(INS(1))),               \
					strings[INS(2)]))) {                            \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LP:                                              \
		if (op (STACK(INS(1)) == prim_to_val(INS(2)))) {            \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LF:                                              \
		if (op (val_to_fn(STACK(INS(1)), TAG_FN) == INS(2))) {      \
			ip++;                                                   \
		}                                                           \
		NEXT();                                                     \
                                                                    \
	BC_ ## ins ## _LV:                                              \
		if (op (val_to_fn(STACK(INS(1)), TAG_NATIVE) == INS(2)))  { \
			ip++;                                                   \
		}                                                           \
		NEXT();

	// Use the opposite comparison operation because we want to execute the
	// jump only if the comparison is true
	EQ(EQ, !);
	EQ(NEQ, );


	//
	//  Ordering
	//

	// Use a macro to generate code for each of the order instructions.
#define ORD(ins, op)                                                  \
	BC_ ## ins ## _LL:                                                \
		if (ensure_num(STACK(INS(1))) op ensure_num(STACK(INS(2)))) { \
			ip++;                                                     \
		}                                                             \
		NEXT();                                                       \
                                                                      \
	BC_ ## ins ## _LI:                                                \
		if (ensure_num(STACK(INS(1))) op                              \
				(double) unsigned_to_signed(INS(2))) {                \
			ip++;                                                     \
		}                                                             \
		NEXT();                                                       \
                                                                      \
	BC_ ## ins ## _LN:                                                \
		if (ensure_num(STACK(INS(1))) op                              \
				val_to_num(constants[INS(2)])) {                      \
			ip++;                                                     \
		}                                                             \
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
	ip += INS(1);
	DISPATCH();

BC_LOOP:
	ip -= INS(1);
	DISPATCH();


	//
	//  Function Calls
	//

BC_CALL: {
	HyValue fn_value = STACK(INS(1));
	if (val_is_fn(fn_value, TAG_FN) || val_is_gc(fn_value, OBJ_METHOD)) {
		// Create a stack frame for the calling function to save the required
		// state
		Index index = (*call_stack_count)++;
		call_stack[index].fn = fn;
		call_stack[index].stack_start = stack_start;
		call_stack[index].return_slot = stack_start + INS(3);
		call_stack[index].ip = ip;

		// Set the self argument
		stack_start = stack_start + INS(1) + 1;
		if (val_is_gc(fn_value, OBJ_METHOD)) {
			Method *method = (Method *) val_to_ptr(fn_value);
			call_stack[index].self = method->parent;
			fn = &functions[method->fn];
		} else {
			call_stack[index].self = VALUE_NIL;
			fn = &functions[val_to_fn(fn_value, TAG_FN)];
		}

		// Set up state for the called function
		ip = &vec_at(fn->instructions, 0);
		DISPATCH();
	} else if (val_is_fn(fn_value, TAG_NATIVE)) {
		// Create a set of arguments to pass to the native function
		HyArgs args;
		args.stack = stack;
		args.start = stack_start + INS(1) + 1;
		args.arity = INS(2);
		NativeFunction *native = &native_fns[val_to_fn(fn_value, TAG_NATIVE)];
		STACK(INS(3)) = native->fn(state, &args);
		NEXT();
	} else {
		// TODO: trigger attempt to call non-function error
		printf("attempt to call non-function\n");
		goto finish;
	}
}


	// Shorthand for returning a value.
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

	// All RET_* instructions.
	SET(RET_, RET);


	//
	//  Structs
	//

BC_STRUCT_NEW: {
	STACK(INS(1)) = struct_instantiate(structs, INS(2));
	NEXT();
}

BC_STRUCT_CALL_CONSTRUCTOR: {
	Struct *instance = val_to_ptr(STACK(INS(1)));
	StructDefinition *def = &structs[instance->definition];

	// Only call the constructor if it exists
	if (def->constructor == NOT_FOUND) {
		NEXT();
	}

	// Set up the new function's stack frame
	Index index = (*call_stack_count)++;
	call_stack[index].fn = fn;
	call_stack[index].stack_start = stack_start;
	call_stack[index].ip = ip;
	call_stack[index].self = STACK(INS(1));

	// Set the return slot to one after all the arguments to the function
	// This slot will not be used by anything
	// Do this because we don't care about the return value from a constructor
	call_stack[index].return_slot = stack_start + INS(2) + INS(3) + 1;

	stack_start = stack_start + INS(2);
	fn = &functions[def->constructor];
	ip = &vec_at(fn->instructions, 0);
	DISPATCH();
}

BC_STRUCT_FIELD: {
	Struct *instance = val_to_ptr(STACK(INS(2)));
	Identifier *field = &fields[INS(3)];
	Index field_index = struct_field_find(&structs[instance->definition],
		field->name, field->length);

	if (field_index != NOT_FOUND) {
		STACK(INS(1)) = instance->fields[field_index];
		NEXT();
	} else {
		printf("Undefined field on struct %.*s\n", field->length, field->name);
		goto finish;
	}
}


// The set function for STRUCT_SET_* instructions.
#define STRUCT_SET(value) {                                                    \
	Struct *instance = val_to_ptr(STACK(INS(3)));                              \
	Identifier *field = &fields[INS(1)];                                       \
	Index field_index = struct_field_find(&structs[instance->definition],      \
		field->name, field->length);                                           \
                                                                               \
	if (field_index != NOT_FOUND) {                                            \
		instance->fields[field_index] = (value);                               \
		NEXT();                                                                \
	} else {                                                                   \
		printf("Undefined field (struct %.*s)\n", field->length, field->name); \
		goto finish;                                                           \
	}                                                                          \
}

	// All STRUCT_SET_* instructions.
	SET(STRUCT_SET_, STRUCT_SET);


	//
	//  Arrays
	//

BC_ARRAY_NEW: {
	Array *array = malloc(sizeof(Array));
	array->type = OBJ_ARRAY;
	array->length = INS(2);
	array->capacity = ceil_power_of_2(array->length);
	array->contents = malloc(sizeof(HyValue) * array->capacity);
	STACK(INS(1)) = ptr_to_val(array);
	NEXT();
}


// Helper to index an array
#define ARRAY_GET(index) {                         \
	if (!val_is_gc(STACK(INS(3)), OBJ_ARRAY)) {    \
		printf("Attempt to index non-array\n");    \
		goto finish;                               \
	}                                              \
                                                   \
	Array *array = val_to_ptr(STACK(INS(3)));      \
	if ((index) < 0 || (index) >= array->length) { \
		printf("Array index out of bounds\n");     \
		goto finish;                               \
	}                                              \
                                                   \
	STACK(INS(1)) = array->contents[(index)];      \
	NEXT();                                        \
}

BC_ARRAY_GET_L: {
	// Check we're indexing by a number
	if (!val_is_num(STACK(INS(2)))) {
		printf("Expected integer when indexing array\n");
		goto finish;
	}

	int64_t index = (int64_t) val_to_num(STACK(INS(2)));
	ARRAY_GET(index);
}

BC_ARRAY_GET_I:
	ARRAY_GET(INS(2));


	// Helper to set an index in an array.
#define ARRAY_I_SET(index, value) {                \
	if (!val_is_gc(STACK(INS(3)), OBJ_ARRAY)) {    \
		printf("Attempt to index non-array\n");    \
		goto finish;                               \
	}                                              \
                                                   \
	Array *array = val_to_ptr(STACK(INS(3)));      \
	if ((index) < 0 || (index) >= array->length) { \
		printf("Array index out of bounds\n");     \
		goto finish;                               \
	}                                              \
                                                   \
	array->contents[(index)] = (value);            \
	NEXT();                                        \
}

	// Set function for ARRAY_I_SET_* instructions.
#define ARRAY_I_SET_INS(value) ARRAY_I_SET(INS(1), value);

	// All ARRAY_I_SET_* instructions.
	SET(ARRAY_I_SET_, ARRAY_I_SET_INS);


	// Set function for ARRAY_L_SET_* instructions.
#define ARRAY_L_SET(value) {                              \
	if (!val_is_num(STACK(INS(2)))) {                     \
		printf("Expected integer when indexing array\n"); \
		goto finish;                                      \
	}                                                     \
                                                          \
	int64_t index = (int64_t) val_to_num(STACK(INS(2)));  \
	ARRAY_I_SET(index, value);                            \
}

	// All ARRAY_L_SET_* instructions.
	SET(ARRAY_L_SET_, ARRAY_L_SET);


finish:
	return NULL;
}
