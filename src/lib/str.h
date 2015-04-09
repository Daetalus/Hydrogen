
//
//  String
//


#ifndef STR_H
#define STR_H

#include "../value.h"


// A heap allocated string.
typedef struct {
	// The string's length.
	size_t length;

	// The allocated capacity for the string.
	size_t capacity;

	// The string's contents, stored using the C struct "hack".
	// This is where we have an empty array at the end of the
	// struct, and allocate more space than the minimum required
	// for the struct on the heap. We then use this array to
	// access that extra memory after the rest of the struct's
	// data.
	char contents[0];
} String;


// Heap allocate a new string with `capacity`.
String * string_new(size_t capacity);

// Copy this string, allocating more memory for the new string.
String * string_copy(String *original);

// Free a string.
void string_free(String *string);

// Append `ch` onto the end of the string.
void string_append_char(String *string, char ch);

// Append the whole of `source` onto the end of `destination`.
void string_append(String *destination, String *source);

// Concatenate `right` onto the end of `left`, returning a new
// string.
String * string_concat(String *left, String *right);

#endif
