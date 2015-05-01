
//
//  IO Library
//


#ifndef IO_H
#define IO_H

#include <stdlib.h>


// Printing
void native_print(uint64_t *stack, int *stack_size);
void native_print_2(uint64_t *stack, int *stack_size);
void native_assert(uint64_t *stack, int *stack_size);
void native_print_stack(uint64_t *stack, int *stack_size);

#endif
