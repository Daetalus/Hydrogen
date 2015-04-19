
//
//  Debug
//


#ifndef DEBUG_H
#define DEBUG_H

#include "bytecode.h"
#include "vm.h"


// Pretty prints out the contents of a bytecode array.
void print_bytecode(Bytecode *bytecode);

// Prints an instruction.
uint8_t * print_instruction(uint8_t *ip, long position);

// Pretty prints out the contents of the stack.
void print_stack(uint64_t *stack, int stack_size);

// Pretty prints out the virtual machine's upvalues.
void print_upvalues(VirtualMachine *vm);


#endif
