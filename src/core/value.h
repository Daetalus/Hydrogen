
//
//  Value
//

#ifndef VALUE_H
#define VALUE_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <hydrogen.h>
#include <vec.h>

// * Values during runtime are stored as NaN tagged 64 bit unsigned integers
// * An IEEE double precision floating point number can represent "not a
//   number" (NaN)
// * When this is done, only 11 of the 64 bits are used, so we can store extra
//   information in the other 53
// * This allows for fast arithmetic of numbers and extraction of other
//   information at runtime
// * Pointers, even on a 64 bit system, only ever use the first 48 bits,
//   allowing us to fit them into NaN tagged doubles
// * Values are stored as follows:
//     * Numbers: NaN bits are not all set
//     * Pointers: sign bit is set, pointer stored in first 48 bits
//     * Functions: sign bit unset, 17th bit set, index stored in first 16 bits
//     * Primitives (nil, false, true): sign bit unset, tag in first 2 bits
//
// * Objects are stored as pointers to heap allocated structs
// * All objects have basic information located at the start of their struct,
//   with type-specific information after this
// * Objects are stored using the "C struct hack", where we use a 0 length
//   array as the last member of a struct definition, then heap allocate as
//   much memory as we need at runtime



//
//  General Values
//

// The sign bit. Only set if the value is a pointer.
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value.
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Primitive value tags.
#define TAG_TRUE  1
#define TAG_FALSE 2
#define TAG_NIL   3

// Primitive values.
#define VALUE_NIL   (QUIET_NAN | TAG_NIL)
#define VALUE_FALSE (QUIET_NAN | TAG_FALSE)
#define VALUE_TRUE  (QUIET_NAN | TAG_TRUE)

// Mask used to indicate a value is a function. Index of function is stored in
// first 16 bits, so set the first bit above this (the 17th).
#define TAG_FN 0x10000
#define TAG_NATIVE 0x20000



//
//  Garbage Collected Objects
//

// The type of an object stored on the heap.
typedef enum {
	OBJ_STRING,
	OBJ_STRUCT,
	OBJ_METHOD,
	OBJ_ARRAY,
} ObjType;


// Objects are stored in values as pointers to heap allocated blocks of memory
// Since these pointers don't contain any type information about what the object
// is (ie. string, struct, etc), each object must have a header containing that
// information.
//
// This only acts as a header for other, type specific objects, which all
// require this general information to be accessible in a consistent manner
#define ObjHeader \
	ObjType type;


// Since we require a general pointer to objects (ignoring specific types),
// create a struct containing common information between objects.
typedef struct {
	ObjHeader;
} Object;


// A string stored as a heap allocated object. The size of the string object
// depends on the string's length, as we use the C struct hack to store its
// contents. This is where we allocate more memory than the size of the struct
// at runtime, then use a zero length array as the last field of the struct to
// access this extra memory, where we store the string's contents.
typedef struct {
	// The object header.
	ObjHeader;

	// The length of the string, so we can avoid calling `strlen` every time we
	// need to find the actual size of this object, or perform an operation on
	// the string.
	uint32_t length;

	// The contents of the string, NULL terminated.
	char contents[0];
} String;


// Similar to strings, struct fields are stored using the struct hack.
typedef struct {
	// The object header.
	ObjHeader;

	// The struct definition object holds more information that we need at
	// runtime which we don't want to duplicate on each instance of the struct
	// in order to save memory (like the names of each field on the struct,
	// which we need when indexing it). So store a pointer back to the original
	// definition.
	Index definition;

	// The number of fields on this struct.
	uint32_t fields_count;

	// The values of each field on the struct. When the struct is first
	// instantiated, normal fields will be set to nil, and methods will have a
	// method value created for them.
	HyValue fields[0];
} Struct;


// Methods on structs are stored as their own heap allocated objects, since we
// need to store a reference back to the parent struct. Methods are garbage
// collected the same way as any other object.
typedef struct {
	// The object header.
	ObjHeader;

	// A pointer to the parent struct to which this method belongs.
	HyValue parent;

	// The index of the function containing this method's bytecode.
	Index fn;
} Method;


// A Hydrogen array object.
typedef struct hy_array {
	// The object header.
	ObjHeader;

	// The length, capacity, and contents of the array.
	uint32_t length, capacity;
	HyValue *contents;
} Array;



//
//  Strings
//

// Calculate the size of a string in bytes.
static inline uint32_t string_size(String *str) {
	// Add 1 for the NULL terminator
	return sizeof(String) + str->length + 1;
}


// Allocate a new string as a copy of another.
static inline String * string_copy(String *original) {
	// Allocate memory for the new string
	uint32_t size = string_size(original);
	String *copy = malloc(size);

	// Copy the old string into the new one
	memcpy(copy, original, size);
	return copy;
}


// Allocate a new string and populate it with the concatenation of `left` and
// `right`.
static inline String * string_concat(String *left, String *right) {
	// Allocate memory for new string
	uint32_t length = left->length + right->length;
	String *concat = malloc(sizeof(String) + length + 1);
	concat->type = OBJ_STRING;
	concat->length = length;
	strncpy(&concat->contents[0], left->contents, left->length);
	strncpy(&concat->contents[left->length], right->contents, right->length);
	concat->contents[length] = '\0';

	// No need to insert NULL terminator since we used calloc
	return concat;
}



//
//  Bitwise Type Conversion
//

// A union to convert between doubles and unsigned 8 byte integers, bypassing
// C's implicit conversion.
typedef union {
	double number;
	uint64_t value;
	void *ptr;
} Converter64Bit;


// Convert a double into a value.
static inline HyValue num_to_val(double number) {
	Converter64Bit converter;
	converter.number = number;
	return converter.value;
}


// Convert a value into a double.
static inline double val_to_num(HyValue val) {
	Converter64Bit converter;
	converter.value = val;
	return converter.number;
}


// Convert a pointer into a value.
static inline HyValue ptr_to_val(void *ptr) {
	Converter64Bit converter;
	converter.ptr = ptr;
	return converter.value | (QUIET_NAN | SIGN);
}


// Convert a value into a pointer.
static inline void * val_to_ptr(HyValue val) {
	Converter64Bit converter;
	converter.value = val & ~(QUIET_NAN | SIGN);
	return converter.ptr;
}


// Converter for 16 bit values.
typedef union {
	uint16_t unsigned_value;
	int16_t signed_value;
} Converter16Bit;


// Convert an unsigned 16 bit integer into a signed 16 bit integer.
static inline int16_t unsigned_to_signed(uint16_t val) {
	Converter16Bit converter;
	converter.unsigned_value = val;
	return converter.signed_value;
}


// Convert a signed 16 bit integer into an unsigned 16 bit integer.
static inline uint16_t signed_to_unsigned(int16_t val) {
	Converter16Bit converter;
	converter.signed_value = val;
	return converter.unsigned_value;
}


// Convert an integer into a value.
static inline HyValue int_to_val(uint16_t integer) {
	return num_to_val((double) unsigned_to_signed(integer));
}



//
//  Value Classification
//

// Return true if a value is a number (quiet NaN bits are not set).
static inline bool val_is_num(HyValue val) {
	return (val & QUIET_NAN) != QUIET_NAN;
}


// Return true if a value is a pointer (quiet NaN bits and sign bit are set).
static inline bool val_is_ptr(HyValue val) {
	return (val & (QUIET_NAN | SIGN)) == (QUIET_NAN | SIGN);
}


// Return true if a value is a garbage collected object with the given type..
static inline bool val_is_gc(HyValue val, ObjType type) {
	return val_is_ptr(val) && ((Object *) val_to_ptr(val))->type == type;
}


// Return true if a value is a function.
static inline bool val_is_fn(HyValue val, uint64_t tag) {
	return (val & (QUIET_NAN | SIGN | tag)) == (QUIET_NAN | tag);
}



//
//  Value Manipulation
//

// Create a value from a primitive tag.
static inline HyValue prim_to_val(uint16_t tag) {
	return QUIET_NAN | tag;
}


// Create a function from an index.
static inline HyValue fn_to_val(uint16_t index, uint64_t tag) {
	return QUIET_NAN | tag | index;
}


// Return the index of a function from its value.
static inline uint16_t val_to_fn(HyValue val, uint64_t tag) {
	return val & ~(QUIET_NAN | tag);
}


// Round a number up to the nearest power of 2.
static inline uint32_t ceil_power_of_2(uint32_t value) {
	uint32_t power = 2;
	while (value >>= 1) {
		power <<= 1;
	}
	return power;
}



//
//  Comparison
//

// Forward declaration.
static inline bool val_cmp(HyValue left, HyValue right);


// Compare two strings for equality.
static inline bool string_cmp(String *left, String *right) {
	return strcmp(left->contents, right->contents) == 0;
}


// Compare two structs for equality.
static bool struct_cmp(Struct *left, Struct *right) {
	// Only equal if both are instances of the same struct
	if (left->definition != right->definition) {
		return false;
	}

	// Compare each field recursively
	for (uint32_t i = 0; i < left->fields_count; i++) {
		// Since we're doing this recursively, we need to check if the user's
		// stored a reference to a struct in one of it's fields, so we don't
		// end up recursing infinitely
		// TODO
		if (!val_cmp(left->fields[i], right->fields[i])) {
			return false;
		}
	}

	// All fields are equal
	return true;
}


// Compare two methods for equality.
static inline bool method_cmp(Method *left, Method *right) {
	return left->parent == right->parent && left->fn == right->fn;
}


// Compare two arrays for equality.
static inline bool array_cmp(Array *left, Array *right) {
	// Lengths of arrays must be equal
	if (left->length != right->length) {
		return false;
	}

	// Check each element in the array
	for (uint32_t i = 0; i < left->length; i++) {
		if (!val_cmp(left->contents[i], right->contents[i])) {
			return false;
		}
	}

	// All elements match
	return true;
}


// Compare two values for equality.
static inline bool val_cmp(HyValue left, HyValue right) {
	if (left == right) {
		return true;
	}

	// If both are pointers
	if (val_is_ptr(left) && val_is_ptr(right)) {
		Object *left_obj = val_to_ptr(left);
		Object *right_obj = val_to_ptr(right);

		// Check the type of each GC object matches
		if (left_obj->type != right_obj->type) {
			return false;
		}

		// Compare based on type
		switch (left_obj->type) {
		case OBJ_STRING:
			return string_cmp((String *) left_obj, (String *) right_obj);
		case OBJ_STRUCT:
			return struct_cmp((Struct *) left_obj, (Struct *) right_obj);
		case OBJ_METHOD:
			return method_cmp((Method *) left_obj, (Method *) right_obj);
		case OBJ_ARRAY:
			return array_cmp((Array *) left_obj, (Array *) right_obj);
		default:
			return false;
		}
	}

	return false;
}

#endif
