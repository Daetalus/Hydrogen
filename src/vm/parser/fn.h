
//
//  Function Parsing
//

#ifndef FN_H
#define FN_H

#include "parser.h"
#include "../bytecode.h"

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
// If we're calling a method on a struct, the `self` argument is supplied in
// the stack slot specified by `self_slot` (-1 otherwise).
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
		uint16_t return_slot, int self_slot);

// Parses a function call, starting at the opening parenthesis of the arguments
// list. `ident` is the name of the function to call.
void parse_fn_call(Parser *parser, Token ident);

// Parses a return statement.
void parse_return(Parser *parser);

#endif
