
//
//  Bytecode
//

#ifndef BYTECODE_H
#define BYTECODE_H

#include "vm.h"


// * A function's bytecode is a list of instructions
// * Each instruction is a 64 bit unsigned integer
// * Each instruction has an operation code (opcode) and 4 arguments
// * The opcode is stored in the lowest 1 byte
// * The 0th argument is stored in the next lowest byte
// * The 1st argument is stored in the next 2 bytes
// * The 2nd and 3rd arguments are each 2 bytes
//
// * There are a maximum of 256 opcodes (since it must fit in 1 byte)


// Instruction operation codes.
//
// Postfixes:
// * L: local
// * I: integer
// * N: number
// * S: string
// * P: primitive (true, false, nil)
// * F: function
// * U: upvalue
// * T: top level local in a package
typedef enum {

	//
	//  Storage
	//

	MOV_LL,
	MOV_LI,
	MOV_LN,
	MOV_LS, // 3rd argument is stack size (for the GC)
	MOV_LP,
	MOV_LF,

	MOV_LU,
	MOV_UL,
	UPVALUE_CLOSE,

	// Arguments:
	// * `local`: stack slot to place top level variable in
	// * `package`: index of package containing top level local
	// * `var`: index of top level local in package
	MOV_LT,

	// Arguments:
	// * `var`: index of top level local in package
	// * `package`: index of package containing top level local to set
	// * `value`: stack slot of local
	MOV_TL,


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

	// 3rd argument is stack size (for the GC)
	CONCAT_LL,
	CONCAT_LS,
	CONCAT_SL,

	NEG_L,


	//
	//  Comparison
	//

	// * A comparison instruction must be followed by a JMP instruction
	// * The following JMP instruction will only be executed if the comparison
	//   is true

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

	// Calls a function, where the index into the VM's function list is
	// specified by the contents of a local. Arguments to the function must be
	// placed in consecutive positions on the stack.
	//
	// Arguments:
	// * `arity`: number of arguments given to the function call
	// * `slot`: the local the function's index is taken from
	// * `argument_start`: the stack slot of the first argument
	// * `return_slot`: the stack slot to store the return value of the
	//   function into
	CALL_L,

	// Calls a function, where the index is specified in the instruction
	// itself.
	CALL_F,

	// Calls a C function in a native package.
	//
	// Arguments:
	// * `arity`: number of arguments given to the function call
	// * `fn`: index of the function in the VM's native function list
	// * `argument_start`: the stack slot of the first argument
	// * `return_slot`: the stack slot to store the return value of the function
	CALL_NATIVE,

	// Return nothing from a function (moves nil into the return slot).
	RET0,

	// Return a value in a stack slot.
	RET1,


	//
	//  Structs
	//

	// Creates a new struct described by `struct_index` in `slot`.
	//
	// Arguments:
	// * `slot`: where to store the new struct on the stack
	// * `struct_index`: the index of the struct's definition in the VM's list
	// * `stack_size`: the number of elements on the stack (for the GC if it's
	//   triggered)
	STRUCT_NEW,

	// Moves the contents of a struct's field into a local slot
	//
	// Arguments:
	// * `slot`: where to store the contents of the field
	// * `struct_slot`: the slot the struct is in
	// * `field_name`: the name of the field, as an index into the VM's struct
	//   field name list
	STRUCT_FIELD,

	// Sets the contents of a struct's field.
	//
	// Arguments:
	// * `slot`: the stack slot of the struct
	// * `field_name`: the name of the field, as an index into the VM's struct
	//   field name list
	// * `value`: the stack slot of the value to set
	STRUCT_SET,


	//
	//  No operation
	//

	NO_OP,
} Opcode;


//
//  Instructions
//

// Creates an instruction from an opcode and 3 arguments. Sets the 0th argument
// to 0.
uint64_t instr_new(Opcode opcode, uint16_t arg1, uint16_t arg2, uint16_t arg3);

// Creates an instruction from an opcode and 4 arguments.
uint64_t instr_new_4(Opcode opcode, uint8_t arg0, uint16_t arg1, uint16_t arg2,
	uint16_t arg3);

// Returns an instruction's opcode.
Opcode instr_opcode(uint64_t instruction);

// Returns the `n`th argument of an instruction.
uint16_t instr_argument(uint64_t instruction, int n);

// Returns `instruction` with a modified opcode.
uint64_t instr_modify_opcode(uint64_t instruction, Opcode new_opcode);

// Returns `instruction` with the `n`th argument modified.
uint64_t instr_modify_argument(uint64_t instruction, int n,
	uint16_t new_argument);


//
//  Bytecode
//

// Appends an instruction to the end of a function's bytecode. Returns the index
// of the instruction in the function's bytecode.
int emit(Function *fn, uint64_t instruction);

// Appends an empty jump instruction (with no target set) to the end of a
// function's bytecode. Returns the index of the jump instruction.
int jmp_new(Function *fn);

#endif
