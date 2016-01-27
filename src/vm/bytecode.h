
//
//  Bytecode Opcodes
//

#ifndef BYTECODE_H
#define BYTECODE_H


// * A function's bytecode is a list of instructions
// * Each instruction is a 64 bit unsigned integer
// * Each instruction has an operation code (opcode) and 4 arguments
// * The opcode is stored in the lowest 1 byte
// * The 0th argument is stored in the next lowest byte (8 bits long)
// * The 1st, 2nd and 3rd arguments are stored in the next 6 bytes (16 bits
//   each)
//
// * There are a maximum of 256 opcodes (since the opcode must fit in 1 byte)


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

	// Arguments:
	// * `local`: stack slot to place top level variable in
	// * `var`: index of top level local in package
	// * `package`: index of package containing top level local
	MOV_LT,


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
	EQ_LV,

	NEQ_LL,
	NEQ_LI,
	NEQ_LN,
	NEQ_LS,
	NEQ_LP,
	NEQ_LF,
	NEQ_LV,

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
	// * `base`: the stack slot containing the function to call and arguments to
	//   the function after it
	// * `arity`: the number of arguments to pass to the function
	// * `return_slot`: the slot in which to store the return value of the
	//   function
	CALL,

	// Returns nothing from a function.
	RET0,

	// Returns a value from a function.
	//
	// Arguments:
	// * `value`: the slot of the return value
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
	// * `field_name`: the name of the field, as an index into the VM's struct
	//   field name list
	// * `value`: set the field to this value
	// * `slot`: the stack slot of the struct
	STRUCT_SET_L,
	STRUCT_SET_I,
	STRUCT_SET_N,
	STRUCT_SET_S,
	STRUCT_SET_P,
	STRUCT_SET_F,
	STRUCT_SET_V,


	//
	//  No Operation
	//

	NO_OP,
} BytecodeOpcode;

#endif
