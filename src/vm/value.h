
//
//  Values
//

#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>

// Integer tags for primitive values.
#define NIL_TAG   0
#define FALSE_TAG 1
#define TRUE_TAG  2

// The sign bit. Only set if the value is a pointer.
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value.
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Static values.
#define NIL_VALUE   (QUIET_NAN | 1)
#define FALSE_VALUE (QUIET_NAN | 2)
#define TRUE_VALUE  (QUIET_NAN | 3)

// Mask used to indicate a value is a function. Function indices are stored in
// the first 2 bytes of a value, so set the first bit above those two bytes to
// indicate a value is a function.
#define FN_TAG 0x10000

// Evaluates to true when `value` is a number, when not all of the quiet NaN
// bits are set.
#define IS_NUMBER_VALUE(value) (((value) & QUIET_NAN) != QUIET_NAN)

// Evaluates to true if `value` is an object, when all quiet NaN bits and the
// sign bit are set.
#define IS_PTR_VALUE(value) \
	(((value) & (QUIET_NAN | SIGN)) == (QUIET_NAN | SIGN))

// Evaluates to true if `value` is a function, when all quiet NaN bits and the
// closure mask (not the sign bit) are set.
#define IS_FN_VALUE(value) \
	(((value) & (QUIET_NAN | FN_TAG)) == (QUIET_NAN | FN_TAG))

// Creates a function value from a function index.
#define VALUE_FROM_FN(index) (((uint64_t) (index)) | QUIET_NAN | FN_TAG)

// Evaluates to a function index from a function value.
#define FN_FROM_VALUE(value) ((uint16_t) ((value) ^ (QUIET_NAN | FN_TAG)))

// Converts a value into a number.
double value_to_number(uint64_t value);

// Converts a number into a value.
uint64_t number_to_value(double number);

// Converts a value into a pointer.
void * value_to_ptr(uint64_t value);

// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr);

#endif
