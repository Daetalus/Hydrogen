
//
//  Expression Parsing
//

#ifndef EXPR_H
#define EXPR_H

#include "parser.h"


// The type of an operand used in a binary expression.
typedef enum {
	OP_LOCAL,
	OP_INTEGER,
	OP_NUMBER,
	OP_STRING,
	OP_PRIMITIVE,
	OP_FN,
	OP_JUMP,
	OP_PACKAGE,
	OP_NONE,
} OperandType;


// An operand in an expression.
typedef struct {
	// The type of the operand.
	OperandType type;

	// If this operand was created by indexing a struct, this will store the
	// stack slot of the struct that was indexed, or -1 otherwise. Used when
	// calling this local as a function, so we know which value to push as
	// `self`.
	int struct_slot;

	// The value of the operand. Numbers and strings are stored as indices into
	// the VM's number/string list.
	union {
		int16_t integer;
		uint16_t number;
		uint16_t string;
		uint16_t primitive;
		uint16_t slot;
		uint16_t fn_index;
		uint16_t value;
		uint16_t index;

		uint32_t jump;
	};
} Operand;


// Parses an expression into `slot`, returning the value of the expression.
// For some expressions, nothing may need to be stored (ie. expressions
// consisting of only a constant), so `slot` will remain unused.
Operand expr(Parser *parser, uint16_t slot);

// Stores the value of an operand into `slot`.
void expr_discharge(Parser *parser, uint16_t slot, Operand operand);

// Parses an expression, storing the result into `slot`.
void expr_emit(Parser *parser, uint16_t slot);

// Parses an expression, storing the result into the local with the given name.
// Triggers an error if the local doesn't exist.
void expr_emit_local(Parser *parser, char *name, size_t length);

// Returns true if `token` can begin an expression.
bool expr_exists(TokenType token);

// Modifies the targets of the jump instructions in a conditional expression
// to update the location of the false case to `false_case`.
void expr_patch_false_case(Parser *parser, Operand operand, int false_case);

#endif
