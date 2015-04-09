
//
//  Value
//


#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>


// The sign bit. Only set if the value is an object.
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value.
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Static values.
#define TRUE_VALUE  (QUIET_NAN | 0x1)
#define FALSE_VALUE (QUIET_NAN | 0x2)
#define NIL_VALUE   (QUIET_NAN | 0x3)

// Evaluates to true when `value` is a number. `value` a number
// when not all of the quiet NaN bits are set.
#define IS_NUMBER(value) (((value) & QUIET_NAN) != QUIET_NAN)

// Evaluates to true if `value` is an object, which is when all
// quiet NaN bits and the sign bit are set.
#define IS_PTR(value) (((value) & (QUIET_NAN | SIGN)) == (QUIET_NAN | SIGN))

// Compares the value against static constant values.
#define IS_TRUE(value)  ((value) == TRUE_VALUE)
#define IS_FALSE(value) ((value) == FALSE_VALUE)
#define IS_NIL(value)   ((value) == NIL_VALUE)


// Converts a value into a number.
double value_to_number(uint64_t value);

// Converts a number into a value.
uint64_t number_to_value(double number);

// Converts a value into a pointer.
void * value_to_ptr(uint64_t value);

// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr);

#endif
