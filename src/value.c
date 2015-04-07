
//
//  Value
//


#include <stdlib.h>
#include <string.h>

#include "value.h"


// A union to convert between doubles and unsigned
// 8 byte integers.
typedef union {
	double number;
	uint64_t value;
} Convert;


// Converts a value into a number.
double as_number(uint64_t value) {
	Convert convert;
	convert.value = value;
	return convert.number;
}


// Converts a number into a value.
uint64_t as_value(double number) {
	Convert convert;
	convert.number = number;
	return convert.value;
}



//
//  Strings
//

// Allocate a new string with the given capacity.
String * string_new(int capacity) {
	String *string = malloc(sizeof(String) + capacity * sizeof(char));
	string->length = 0;
	string->capacity = capacity;
	string->contents[0] = '\0';
	return string;
}


// Duplicates a string, allocating new space on the heap
// for the second one.
String * string_duplicate(String *original) {
	size_t size = sizeof(String) + original->capacity * sizeof(char);
	String *copy = malloc(size);
	memcpy(copy, original, size);
	return copy;
}


// Free a string.
void string_free(String *string) {
	free(string->contents);
}


// Resize a string to fix a minimum of the given size.
void string_resize(String **string, int length) {
	if ((*string)->capacity < length) {
		(*string)->capacity = (*string)->capacity * 1.5;
		if ((*string)->capacity < length) {
			(*string)->capacity = length * 1.5;
		}

		size_t size = sizeof(String) + (*string)->capacity * sizeof(char);
		*string = realloc(*string, size);
	}
}


// Append a character onto the given string.
void string_append_char(String *string, char ch) {
	string_resize(&string, string->length + 1);
	string->length += 1;
	string->contents[string->length - 1] = ch;

	// Shift the null terminator
	string->contents[string->length] = '\0';
}


// Append a string onto the given string.
void string_append(String *destination, String *source) {
	string_resize(&destination, destination->length + source->length);
	char *last = &destination->contents[destination->length];
	strncpy(last, source->contents, source->length);
	destination->length += source->length;

	// Add the null terminator
	destination->contents[destination->length] = '\0';
}
