
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
uint64_t to_number(double number) {
	Convert convert;
	convert.number = number;
	return convert.value;
}



//
//  String
//

// Allocate a new string with the given capacity.
void string_new(String *string, int capacity) {
	string->length = 0;
	string->capacity = capacity;
	string->contents = malloc(capacity * sizeof(char));
	string->contents[0] = '\0';
}


// Resize a string to fix a minimum of the given size.
void string_resize(String *string, int length) {
	if (string->capacity < length) {
		string->capacity = string->capacity * 1.5;
		if (string->capacity < length) {
			string->capacity = length * 1.5;
		}

		int size_in_bytes = string->capacity * sizeof(char);
		string->contents = realloc(string->contents, size_in_bytes);
	}
}


// Copy a character array into a new string.
void string_copy(String *destination, char *source, int length) {
	destination->length = length;

	// Add 1 for the null terminator
	destination->capacity = length + 1;
	destination->contents = malloc((length + 1) * sizeof(char));
	strncpy(destination->contents, source, length);
	destination->contents[length] = '\0';
}


// Append a character onto the given string.
void string_append_char(String *string, char ch) {
	string_resize(string, string->length + 1);
	string->length += 1;
	string->contents[string->length - 1] = ch;

	// Shift the null terminator
	string->contents[string->length] = '\0';
}


// Append a string onto the given string.
void string_append(String *destination, String *source) {
	string_resize(destination, destination->length + source->length);
	char *last = &destination->contents[destination->length];
	strncpy(last, source->contents, source->length);
	destination->length += source->length;

	// Add the null terminator
	destination->contents[destination->length] = '\0';
}
