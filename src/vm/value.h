
//
//  Values
//

#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>

#include "vm.h"


// Integer tags for primitive values.
#define NIL_TAG   1
#define FALSE_TAG 2
#define TRUE_TAG  3

// The sign bit. Only set if the value is a pointer.
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value.
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Static values.
#define NIL_VALUE   (QUIET_NAN | NIL_TAG)
#define FALSE_VALUE (QUIET_NAN | FALSE_TAG)
#define TRUE_VALUE  (QUIET_NAN | TRUE_TAG)

// Masks used to indicate a value is of a particular type. Values are stored in
// the first 2 bytes, so set the first bit above those two bytes.
#define FN_TAG 0x10000

// Evaluates to true when `value` is a number, when not all of the quiet NaN
// bits are set.
#define IS_NUMBER_VALUE(value) (((value) & QUIET_NAN) != QUIET_NAN)

// Evaluates to true if `value` is an object, when all quiet NaN bits and the
// sign bit are set.
#define IS_PTR_VALUE(value) \
	(((value) & (QUIET_NAN | SIGN)) == (QUIET_NAN | SIGN))

// Evaluates to true if `value` is a string.
#define IS_STRING_VALUE(value) \
	(IS_PTR_VALUE(value) && ((Object *) value_to_ptr(value))->type == OBJ_STRING)

// Evaluates to true if `value` is an object.
#define IS_OBJ_VALUE(value) \
	(IS_PTR_VALUE(value) && ((Object *) value_to_ptr(value))->type == OBJ_STRUCT)

// Evaluates to true if `value` is a function, when all quiet NaN bits and the
// closure mask (not the sign bit) are set.
#define IS_FN_VALUE(value) \
	(((value) & (QUIET_NAN | FN_TAG)) == (QUIET_NAN | FN_TAG))

// Creates a function value from a function index.
#define INDEX_TO_VALUE(index, tag) (((uint64_t) (index)) | QUIET_NAN | (tag))

// Evaluates to a function index from a function value.
#define VALUE_TO_INDEX(value, tag) ((uint16_t) ((value) ^ (QUIET_NAN | (tag))))

// Creates a value from a primitive tag.
#define PRIMITIVE_FROM_TAG(tag) (((uint64_t) (tag)) | QUIET_NAN)

// Evaluates to a string from a value.
#define TO_STR(value) \
	(&(((Object *) value_to_ptr((value)))->string[0]))

// Evaluates to a struct's fields array.
#define TO_FIELDS(value) \
	(&(((Object *) value_to_ptr((value)))->obj.fields[0]))

// Evaluates to the number of fields in a struct.
#define FIELDS_COUNT(value) \
	(((Object *) value_to_ptr((value)))->obj.definition->fields_count)


// The type of an object.
typedef enum {
	OBJ_STRING,
	OBJ_STRUCT,
} ObjectType;


// Used to represent all structs and strings during runtime.
typedef struct {
	// The type of this object (a struct or string).
	ObjectType type;

	union {
		struct {
			// A pointer to the struct definition.
			StructDefinition *definition;

			// The fields of the struct.
			uint64_t fields[0];
		} obj;

		// The contents of a string.
		char string[0];
	};
} Object;


// Converts a value into a number.
double value_to_number(uint64_t value);

// Converts a number into a value.
uint64_t number_to_value(double number);

// Converts a value into a pointer.
void * value_to_ptr(uint64_t value);

// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr);

#endif
