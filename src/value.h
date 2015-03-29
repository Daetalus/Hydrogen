
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

// Bits to set on a quiet NaN value to give it
// different values.
#define NAN_MASK   (0x0)
#define TRUE_MASK  (0x1)
#define FALSE_MASK (0x2)
#define NIL_MASK   (0x3)

// Values.
#define NAN_VALUE   (QUIET_NAN | NAN_MASK)
#define TRUE_VALUE  (QUIET_NAN | TRUE_MASK)
#define FALSE_VALUE (QUIET_NAN | FALSE_MASK)
#define NIL_VALUE   (QUIET_NAN | NIL_MASK)

// Evaluates to true if the given value is a number.
// The value is a number when not all of the quiet NaN
// bits are set.
#define IS_NUMBER(value) ((value) & QUIET_NAN != QUIET_NAN)

// Evaluates to true if the given value is an object,
// which is when all quiet NaN bits and the sign bit
// are set.
#define IS_OBJ(value) ((value) & (QUIET_NAN | SIGN) == (QUIET_NAN | SIGN))

// Compares the value against.
#define IS_NAN(value)   ((value) == NAN_VALUE)
#define IS_TRUE(value)  ((value) == TRUE_VALUE)
#define IS_FALSE(value) ((value) == FALSE_VALUE)
#define IS_NIL(value)   ((value) == NIL_VALUE)

// Converts the value to an object pointer.
#define AS_OBJ(value) ((Obj *) ((uint64_t) ((value) & ~(QUIET_NAN | SIGN))))


// Converts a value into a number.
double as_number(uint64_t value);

// Converts a number into a value.
uint64_t to_number(double number);

#endif
