
//
//  Debug
//


#ifndef DEBUG_H
#define DEBUG_H

#include "bytecode.h"


// Pretty prints out the contents of a bytecode array.
void pretty_print_bytecode(Bytecode *bytecode);

// Prints an instruction.
uint8_t * print_instruction(uint8_t *ip, long position);

// Pretty prints out the contents of the stack.
void pretty_print_stack(uint64_t *stack, int stack_size);


#endif
