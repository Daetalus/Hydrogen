
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


// The sign bit. Only set if the value is a pointer
#define SIGN ((uint64_t) 1 << 63)

// Bits that, when set, indicate a quiet NaN value
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

// Primitive value tags
#define TAG_TRUE  1
#define TAG_FALSE 2
#define TAG_NIL   3

// Primitive values
#define VALUE_NIL   (QUIET_NAN | TAG_NIL)
#define VALUE_FALSE (QUIET_NAN | TAG_FALSE)
#define VALUE_TRUE  (QUIET_NAN | TAG_TRUE)

// Mask used to indicate a value is a function. Index of function is stored in
// first 16 bits, so set the first bit above this (the 17th)
#define TAG_FN 0x10000
#define TAG_NATIVE 0x20000


// The type of an object stored on the heap
typedef enum {
	OBJ_STRING,
	OBJ_STRUCT,
	OBJ_METHOD,
	OBJ_ARRAY,
} ObjectType;


// Objects are stored in values as pointers to heap allocated blocks of memory
// Since these pointers don't contain any type information about what the object
// is (ie. string, struct, etc), each object must have a header containing that
// information
//
// This only acts as a header for other, type specific objects, which all
// require this general information to be accessible in a consistent manner
#define ObjectHeader \
	ObjectType type;


// Since we require a general pointer to objects (ignoring specific types),
// create a struct containing common information between objects
typedef struct {
	ObjectHeader;
} Object;


// A string stored as a heap allocated object. The size of the string object
// depends on the string's length, as we use the C struct hack to store its
// contents. This is where we allocate more memory than the size of the struct
// at runtime, then use a zero length array as the last field of the struct to
// access this extra memory, where we store the string's contents
typedef struct {
	// The object header
	ObjectHeader;

	// The length of the string, so we can avoid calling `strlen` every time we
	// need to find the actual size of this object, or perform an operation on
	// the string
	uint32_t length;

	// The contents of the string, NULL terminated
	char contents[0];
} String;


// Calculates the size of a string in bytes
static inline uint32_t string_size(String *str) {
	// Add 1 for the NULL terminator
	return sizeof(String) + str->length + 1;
}


// Allocates a new string as a copy of another
static inline String * string_copy(String *original) {
	// Allocate memory for the new string
	uint32_t size = string_size(original);
	String *copy = malloc(size);

	// Copy the old string into the new one
	memcpy(copy, original, size);
	return copy;
}


// Allocates a new string and populates it with the concatenation of `left` and
// `right`
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


// Similar to strings, struct fields are stored using the struct hack
typedef struct {
	// The object header
	ObjectHeader;

	// The struct definition object holds more information that we need at
	// runtime which we don't want to duplicate on each instance of the struct
	// in order to save memory (like the names of each field on the struct,
	// which we need when indexing it). So store a pointer back to the original
	// definition
	Index definition;

	// The values of each field on the struct. When the struct is first
	// instantiated, normal fields will be set to nil, and methods will have a
	// method value created for them
	HyValue fields[0];
} Struct;


// Methods on structs are stored as their own heap allocated objects, since we
// need to store a reference back to the parent struct. Methods are garbage
// collected the same way as any other object
typedef struct {
	// The object header
	ObjectHeader;

	// A pointer to the parent struct to which this method belongs
	HyValue parent;

	// The index of the function containing this method's bytecode
	Index fn;
} Method;


// A Hydrogen array object
typedef struct {
	// The object header
	ObjectHeader;

	// The length, capacity, and contents of the array
	uint32_t length, capacity;
	HyValue *contents;
} Array;



//
//  Bitwise Type Conversion
//

// A union to convert between doubles and unsigned 8 byte integers without
// implicit conversion
typedef union {
	double number;
	uint64_t value;
	void *ptr;
} Converter64Bit;


// Converts a double into a value
static inline HyValue num_to_val(double number) {
	Converter64Bit converter;
	converter.number = number;
	return converter.value;
}


// Converts a value into a double
static inline double val_to_num(HyValue val) {
	Converter64Bit converter;
	converter.value = val;
	return converter.number;
}


// Converts a pointer into a value
static inline HyValue ptr_to_val(void *ptr) {
	Converter64Bit converter;
	converter.ptr = ptr;
	return converter.value | (QUIET_NAN | SIGN);
}


// Converts a value into a pointer
static inline void * val_to_ptr(HyValue val) {
	Converter64Bit converter;
	converter.value = val & ~(QUIET_NAN | SIGN);
	return converter.ptr;
}


// Converter for 16 bit values
typedef union {
	uint16_t unsigned_value;
	int16_t signed_value;
} Converter16Bit;


// Converts an unsigned 16 bit integer into a signed 16 bit integer
static inline int16_t unsigned_to_signed(uint16_t val) {
	Converter16Bit converter;
	converter.unsigned_value = val;
	return converter.signed_value;
}


// Converts a signed 16 bit integer into an unsigned 16 bit integer
static inline uint16_t signed_to_unsigned(int16_t val) {
	Converter16Bit converter;
	converter.signed_value = val;
	return converter.unsigned_value;
}


// Converts an integer into a value
static inline HyValue int_to_val(uint16_t integer) {
	return num_to_val((double) unsigned_to_signed(integer));
}



//
//  Value Manipulation
//

// Returns true if a value is a number (quiet NaN bits are not set)
static inline bool val_is_num(HyValue val) {
	return (val & QUIET_NAN) != QUIET_NAN;
}


// Returns true if a value is a pointer (quiet NaN bits and sign bit are set)
static inline bool val_is_ptr(HyValue val) {
	return (val & (QUIET_NAN | SIGN)) == (QUIET_NAN | SIGN);
}


// Returns true if a value is a string
static inline bool val_is_str(HyValue val) {
	return val_is_ptr(val) && ((Object *) val_to_ptr(val))->type == OBJ_STRING;
}


// Returns true if a value is a struct
static inline bool val_is_struct(HyValue val) {
	return val_is_ptr(val) && ((Object *) val_to_ptr(val))->type == OBJ_STRUCT;
}


// Returns true if a value is a method on a struct
static inline bool val_is_method(HyValue val) {
	return val_is_ptr(val) && ((Object *) val_to_ptr(val))->type == OBJ_METHOD;
}


// Returns true if a value is a method on a struct
static inline bool val_is_array(HyValue val) {
	return val_is_ptr(val) && ((Object *) val_to_ptr(val))->type == OBJ_ARRAY;
}


// Returns true if a value is a function
static inline bool val_is_fn(HyValue val) {
	return (val & (QUIET_NAN | SIGN | TAG_FN)) == (QUIET_NAN | TAG_FN);
}


// Returns true if a value is a native function
static inline bool val_is_native(HyValue val) {
	return (val & (QUIET_NAN | SIGN | TAG_NATIVE)) == (QUIET_NAN | TAG_NATIVE);
}


// Creates a value from a primitive tag
static inline HyValue prim_to_val(uint16_t tag) {
	return QUIET_NAN | tag;
}


// Create a function from an index
static inline HyValue fn_to_val(uint16_t index) {
	return QUIET_NAN | TAG_FN | index;
}


// Returns the index of a function from its value
static inline uint16_t val_to_fn(HyValue val) {
	return val & ~(QUIET_NAN | TAG_FN);
}


// Create a native function value from an index
static inline HyValue native_to_val(uint16_t index) {
	return QUIET_NAN | TAG_NATIVE | index;
}


// Returns the index of a native function from its value
static inline uint16_t val_to_native(HyValue val) {
	return val & ~(QUIET_NAN | TAG_NATIVE);
}

#endif
