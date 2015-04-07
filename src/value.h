
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
#define AS_FN(value) (void (*)(void)) ((uint64_t) ((value) & ~(QUIET_NAN | SIGN)))


// Converts a value into a number.
double as_number(uint64_t value);

// Converts a number into a value.
uint64_t as_value(double number);


// A string in the source code.
typedef struct {
	// The length of the string in the source code.
	int length;

	// The pointer into the source code specifying the
	// start of the string.
	char *location;
} SourceString;



//
//  Object
//

// An object.
typedef struct _obj {
	// A pointer to the next object in the linked list of
	// all allocated objects. Used when collecting garbage
	// to loop over all allocated objects.
	struct _obj *next;
} Obj;



//
//  String
//

// A heap allocated string.
typedef struct {
	// The string's underlying object.
	Obj obj;

	// The string's length.
	int length;

	// The allocated capacity for the string.
	int capacity;

	// The string's contents, stored using the C struct "hack",
	// where we allocate more space in the heap for the struct than
	// we need, and just use the unused part of memory after the
	// other struct fields for the string.
	char contents[0];
} String;


// Allocate a new string with the given capacity.
String * string_new(int capacity);

// Duplicates a string, allocating new space on the heap
// for the second one.
String * string_duplicate(String *original);

// Free a string.
void string_free(String *string);

// Append a character onto the given string.
void string_append_char(String *string, char ch);

// Append a string onto the given string.
void string_append(String *destination, String *source);

#endif
