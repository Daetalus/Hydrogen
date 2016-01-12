
//
//  Values
//

#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>


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
	(IS_PTR_VALUE(value) && ((Object *) val_to_ptr(value))->type == OBJ_STRING)

// Evaluates to true if `value` is an object.
#define IS_OBJ_VALUE(value) \
	(IS_PTR_VALUE(value) && ((Object *) val_to_ptr(value))->type == OBJ_STRUCT)

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
	(&(((String *) val_to_ptr((value)))->contents[0]))

// Evaluates to a struct's fields array.
#define TO_FIELDS(value) \
	(&(((Struct *) val_to_ptr((value)))->fields[0]))

// Evaluates to the number of fields in a struct.
#define FIELDS_COUNT(value) \
	(((Struct *) val_to_ptr((value)))->definition->fields_count)


// The type of an object.
typedef enum {
	OBJ_STRING,
	OBJ_STRUCT,
} ObjectType;


// A generic object.
typedef struct object {
	// The type of this object (struct or string).
	ObjectType type;

	// The next object in the linked list of all objects (for the GC).
	struct object *next;

	// Mark bit for the garbage collector.
	uint8_t mark;
} Object;


// Used to represent strings.
typedef struct string {
	// Values inherited from an object.
	ObjectType type;
	struct object *next;
	uint8_t mark;

	// The length of this string.
	size_t length;

	// The contents of the string (with a NULL terminator).
	char contents[0];
} String;


// Used to represent structs.
typedef struct {
	// Values inherited from an object.
	ObjectType type;
	struct object *next;
	uint8_t mark;

	// A pointer to the struct definition.
	struct struct_definition *definition;

	// The fields of the struct.
	uint64_t fields[0];
} Struct;



//
//  Type Conversion
//

// A union to convert between doubles and unsigned 8 byte integers without
// implicit conversion.
typedef union {
	double number;
	uint64_t value;
	void *ptr;
} Converter64Bit;


// Converter for 16 bit values.
typedef union {
	uint16_t unsigned_value;
	int16_t signed_value;
} Converter16Bit;


// Converts a value into a number.
static inline double val_to_num(uint64_t value) {
	Converter64Bit convert;
	convert.value = value;
	return convert.number;
}


// Converts a number into a value.
static inline uint64_t num_to_val(double number) {
	Converter64Bit convert;
	convert.number = number;
	return convert.value;
}


// Converts a value into a pointer.
static inline void * val_to_ptr(uint64_t value) {
	Converter64Bit convert;
	convert.value = (value & ~(QUIET_NAN | SIGN));
	return convert.ptr;
}


// Converts a pointer into a value.
static inline uint64_t ptr_to_val(void *ptr) {
	Converter64Bit convert;
	convert.ptr = ptr;
	return (convert.value) ^ (QUIET_NAN | SIGN);
}


// Converts an unsigned 16 bit integer to a signed 16 bit integer.
static inline int16_t uint16_to_int16(uint16_t value) {
	Converter16Bit converter;
	converter.unsigned_value = value;
	return converter.signed_value;
}


// Converts a signed 16 bit integer to an unsigned 16 bit integer.
static inline uint16_t int16_to_uint16(int16_t value) {
	Converter16Bit converter;
	converter.signed_value = value;
	return converter.unsigned_value;
}

#endif
