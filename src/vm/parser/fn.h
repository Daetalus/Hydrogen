
//
//  Function Parsing
//

#ifndef FN_H
#define FN_H

#include "../bytecode.h"

#include "parser.h"
#include "expr.h"

// Parses a function or method definition.
void parse_fn_definition(Parser *parser);

// Parses a function definition body (starting at the arguments list) for a
// function with the name `name`. `is_method` should be true if this function
// is a method on a struct. In this case, `self` will be added as the first
// local on the child function's local stack. Creates a new function on the
// VM and returns its index.
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length,
	bool is_method);

// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
// If the `self` argument has its `is_method` field set to true, then the
// we are calling a method on a struct, and a `self` argument is pushed onto
// the argument's list. The `self` value is reconstructed from the data in the
// given argument.
void parse_fn_call_self(Parser *parser, Opcode call, uint16_t slot,
	uint16_t return_slot, OperandSelf *self);

// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
	uint16_t return_slot);

// Parses a function call, starting at the opening parenthesis of the arguments
// list.
void parse_fn_call(Parser *parser, Identifier *list, int count);

// Parses a call to a function in a native package. `index` is the index of the
// native package in the VM's native package list. `return_slot` is the location
// to store the return value of the function call.
void parse_native_fn_call(Parser *parser, uint32_t index, uint16_t return_slot);

// Parses a return statement.
void parse_return(Parser *parser);

#endif
