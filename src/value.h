
//
//  Value
//


#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>


//
//  Constants
//

// The sign bit. Only set if the value is a pointer.
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value.
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Bitwise mask set on pointers.
#define POINTER_MASK (SIGN | QUIET_NAN)

// Bitwise masks for values representing different types of
// functions.
//
// Functions and natives are stored as a 2 byte index in the
// VM's functions list or natives list respectively.
#define FUNCTION_MASK (QUIET_NAN | 0x100000000)
#define NATIVE_MASK   (QUIET_NAN | 0x200000000)
#define METHOD_MASK   (POINTER_MASK | (uint64_t) 1 << 49)

// Bitwise representation for constant language values.
#define TRUE_VALUE  (QUIET_NAN | 0x1)
#define FALSE_VALUE (QUIET_NAN | 0x2)
#define NIL_VALUE   (QUIET_NAN | 0x3)



//
//  Type Checking
//

// Evaluates to true when `value` is a number. `value` a number
// when not all of the quiet NaN bits are set.
#define IS_NUMBER(value) (((value) & QUIET_NAN) != QUIET_NAN)

// Evaluates to true if `value` is an object, which is when all
// quiet NaN bits and the sign bit are set.
#define IS_PTR(value) (((value) & POINTER_MASK) == POINTER_MASK)

// Evaluates to true if `value` is of a particular function
// type.
#define IS_FUNCTION(value) (((value) & FUNCTION_MASK) == FUNCTION_MASK)
#define IS_NATIVE(value)   (((value) & NATIVE_MASK) == NATIVE_MASK)
#define IS_METHOD(value)   (((value) & METHOD_MASK) == METHOD_MASK)

// Compare `value` against static constants.
#define IS_TRUE(value)  ((value) == TRUE_VALUE)
#define IS_FALSE(value) ((value) == FALSE_VALUE)
#define IS_NIL(value)   ((value) == NIL_VALUE)



//
//  Conversion
//

// Convert different types of functions to values and back.
#define FUNCTION_TO_VALUE(index) ((index) | FUNCTION_MASK)
#define VALUE_TO_FUNCTION(value) ((value) ^ FUNCTION_MASK)

#define NATIVE_TO_VALUE(index)   ((index) | NATIVE_MASK)
#define VALUE_TO_NATIVE(value)   ((value) ^ NATIVE_MASK)

#define METHOD_TO_VALUE(ptr)     (ptr_to_value((ptr)) | METHOD_MASK)
#define VALUE_TO_METHOD(value)   value_to_ptr((value) ^ METHOD_MASK)

// Converts a value into a number.
double value_to_number(uint64_t value);

// Converts a number into a value.
uint64_t number_to_value(double number);

// Converts a value into a pointer.
void * value_to_ptr(uint64_t value);

// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr);

#endif
