
//
//  Value
//

#ifndef VALUE_H
#define VALUE_H

#include <hydrogen.h>

#include "vec.h"


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


// The type of an object stored on the heap.
typedef enum {
	OBJ_STRING,
	OBJ_STRUCT,
} ObjectType;


// Objects are stored in values as pointers to heap allocated blocks of memory.
// Since these pointers don't contain any type information about what the object
// is (ie. string, struct, etc), each object must have a header containing that
// information.
//
// This only acts as a header for other, type specific objects, which all
// require this general information to be accessible in a consistent manner.
typedef struct {
	// The type of the object.
	ObjectType type;
} Object;


// A string stored as a heap allocated object. The size of the string object
// depends on the string's length, as we use the "C struct hack" to store its
// contents. This is where we allocate more memory than the size of the struct
// at runtime, then use a zero length array as the last field of the struct to
// access this extra memory, where we store the string's contents.
typedef struct {
	// The object header.
	Object;

	// The length of the string, so we can avoid calling `strlen` every time we
	// need to find the actual size of this object, or perform an operation on
	// the string.
	uint32_t length;

	// The contents of the string, NULL terminated.
	char contents[0];
} String;


// Calculates the size of a string in bytes.
static inline uint32_t string_size(String *str) {
	// Add 1 for the NULL terminator
	return sizeof(Object) + str->length + 1;
}


// Similar to strings, struct fields are stored using the "C struct hack".
typedef struct {
	// The object header.
	Object;

	// The struct definition object holds more information that we need at
	// runtime which we don't want to duplicate on each instance of the struct
	// in order to save memory (like the names of each field on the struct,
	// which we need when indexing it). So store a pointer back to the original
	// definition.
	Index definition;

	// The values of each field on the struct.
	HyValue fields[0];
} Struct;

#endif
