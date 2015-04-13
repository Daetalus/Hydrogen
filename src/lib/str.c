
//
//  String
//


#include <stdlib.h>
#include <string.h>

#include "str.h"


// Heap allocate a new string with `capacity`.
String * string_new(int capacity) {
	String *string = malloc(sizeof(String) + capacity * sizeof(char));
	string->length = 0;
	string->capacity = capacity;
	string->contents[0] = '\0';
	return string;
}


// Copy this string, allocating more memory for the new string.
String * string_copy(String *original) {
	String *copy = string_new(original->length + 1);
	copy->length = original->length;
	strcpy(copy->contents, original->contents);
	return copy;
}


// Free a string.
void string_free(String *string) {
	free(string);
}


// Resize a string to fit a minimum of the given size.
void string_resize(String **string, int length) {
	if ((*string)->capacity < length) {
		// Increase the capacity
		(*string)->capacity *= 2;
		if ((*string)->capacity < length) {
			(*string)->capacity = length * 2;
		}

		// Reallocate memory
		int size = sizeof(String) + (*string)->capacity * sizeof(char);
		*string = realloc(*string, size);
	}
}


// Append `ch` onto the end of the string.
void string_append_char(String *string, char ch) {
	// Add 1 for the new character, and another for the NULL terminator.
	string_resize(&string, string->length + 2);
	string->length += 1;
	string->contents[string->length - 1] = ch;

	// Add the NULL terminator
	string->contents[string->length] = '\0';
}


// Append the whole of `source` onto the end of `destination`.
void string_append(String *destination, String *source) {
	// Resize the destination string to fit the result.
	// Add 1 for the NULL terminator.
	string_resize(&destination, destination->length + source->length + 1);

	// Copy the source string into the destination's contents
	// array.
	char *last = &destination->contents[destination->length];
	strncpy(last, source->contents, source->length);

	// Add the NULL terminator
	destination->contents[destination->length] = '\0';

	// Increase the destination's length
	destination->length += source->length;
}


// Concatenate `right` onto the end of `left`, returning a new
// string.
String * string_concat(String *left, String *right) {
	// Allocate a new string with enough space.
	// Add 1 for the NULL terminator.
	String *result = string_new(left->length + right->length + 1);

	// Copy the left and right strings into the new string.
	// `strcpy` adds the NULL terminator for us.
	strcpy(result->contents, left->contents);
	strcpy(&result->contents[left->length], right->contents);

	// Set the new string's length
	result->length = left->length + right->length;

	return result;
}
