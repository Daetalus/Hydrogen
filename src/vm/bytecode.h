
//
//  Bytecode Opcodes
//

#ifndef BYTECODE_H
#define BYTECODE_H

// Instruction operation codes for Hydrogen bytecode.
//
// Postfix meanings:
// * L: local
// * I: integer
// * N: number
// * S: string
// * P: primitive (true, false, nil)
// * F: function
// * V: native function
// * U: upvalue
// * T: top level local in a package
typedef enum {

	//
	//  Storage
	//

	MOV_LL,
	MOV_LI,
	MOV_LN,
	MOV_LS,
	MOV_LP,
	MOV_LF,
	MOV_LV,

	MOV_UL,
	MOV_UI,
	MOV_UN,
	MOV_US,
	MOV_UP,
	MOV_UF,
	MOV_UV,

	MOV_LU,
	UPVALUE_CLOSE,

	// Arguments:
	// * `local`: stack slot to place top level variable in
	// * `package`: index of package containing top level local
	// * `var`: index of top level local in package
	MOV_LT,

	// Arguments:
	// * `var`: index of top level local in package
	// * `value`: value to set top level local to
	// * `package`: index of package containing top level local to set
	MOV_TL,
	MOV_TI,
	MOV_TN,
	MOV_TS,
	MOV_TP,
	MOV_TF,
	MOV_TV,


	//
	//  Math
	//

	ADD_LL,
	ADD_LI,
	ADD_LN,
	ADD_IL,
	ADD_NL,

	SUB_LL,
	SUB_LI,
	SUB_LN,
	SUB_IL,
	SUB_NL,

	MUL_LL,
	MUL_LI,
	MUL_LN,
	MUL_IL,
	MUL_NL,

	DIV_LL,
	DIV_LI,
	DIV_LN,
	DIV_IL,
	DIV_NL,

	MOD_LL,
	MOD_LI,
	MOD_LN,
	MOD_IL,
	MOD_NL,

	CONCAT_LL,
	CONCAT_LS,
	CONCAT_SL,

	NEG_L,


	//
	//  Comparison
	//

	// * A comparison instruction must be followed by a JMP instruction
	// * The JMP instruction will only be executed if the comparison is true

	IS_TRUE_L,
	IS_FALSE_L,

	EQ_LL,
	EQ_LI,
	EQ_LN,
	EQ_LS,
	EQ_LP,
	EQ_LF,

	NEQ_LL,
	NEQ_LI,
	NEQ_LN,
	NEQ_LS,
	NEQ_LP,
	NEQ_LF,

	LT_LL,
	LT_LI,
	LT_LN,

	LE_LL,
	LE_LI,
	LE_LN,

	GT_LL,
	GT_LI,
	GT_LN,

	GE_LL,
	GE_LI,
	GE_LN,


	//
	//  Control flow
	//

	// Jumps forwards by `amount` instructions.
	JMP,

	// Jumps backwards by `amount` instructions (used for loops).
	LOOP,


	//
	//  Functions
	//

	// The function to call must be in the slot specified by `base`. All
	// arguments to the function are placed after this first slot (the number
	// of arguments specified by `arity`). The return values of the function
	// replace the arguments and function object (they start in slot `base`).
	// The number of return values expected is given by `return`.
	//
	// Arguments:
	// * `base`: the base stack slot
	// * `arity`: the number of arguments to pass to the function
	// * `return`: the number of return values expected by the caller
	CALL,

	// Returns nothing from a function.
	RET0,

	// Returns values from a function. They must be stored sequentially.
	//
	// Arguments:
	// * `base`: the starting slot of the return values
	// * `count`: the number of return values
	RET,


	//
	//  Structs
	//

	// Creates an instance of a struct.
	//
	// Arguments:
	// * `slot`: where to store the new struct on the stack
	// * `struct_index`: the index of the struct's definition in the VM's list
	STRUCT_NEW,

	// Moves the contents of a struct's field into a local slot
	//
	// Arguments:
	// * `slot`: where to store the contents of the field
	// * `struct_slot`: the stack slot of the struct
	// * `field_name`: the name of the field to get, as an index into the VM's
	//   struct field name list
	STRUCT_FIELD,

	// Sets the contents of a struct's field.
	//
	// Arguments:
	// * `slot`: the stack slot of the struct
	// * `field_name`: the name of the field, as an index into the VM's struct
	//   field name list
	// * `value`: set the field to this value
	STRUCT_SET_L,
	STRUCT_SET_I,
	STRUCT_SET_N,
	STRUCT_SET_S,
	STRUCT_SET_P,
	STRUCT_SET_F,
	STRUCT_SET_V,
} BytecodeOpcode;

#endif
