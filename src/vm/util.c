
//
//  Utilities
//

#include <stdio.h>

#include "util.h"


// TODO: Convert to pointer casting rather than unions


// Converter for 64 bit values.
typedef union {
	double double_value;
	uint64_t uint64_value;
} Converter64Bit;


// Converter for 16 bit values.
typedef union {
	uint16_t unsigned_value;
	int16_t signed_value;
} Converter16Bit;


// Converts a double into an unsigned 64 bit integer.
uint64_t double_to_uint64(double value) {
	Converter64Bit converter;
	converter.double_value = value;
	return converter.uint64_value;
}


// Converts an unsigned 64 bit integer into a double.
double uint64_t_to_double(uint64_t value) {
	Converter64Bit converter;
	converter.uint64_value = value;
	return converter.double_value;
}


// Converts an unsigned 16 bit integer to a signed 16 bit integer.
int16_t uint16_to_int16(uint16_t value) {
	Converter16Bit converter;
	converter.unsigned_value = value;
	return converter.signed_value;
}


// Converts a signed 16 bit integer to an unsigned 16 bit integer.
uint16_t int16_to_uint16(int16_t value) {
	Converter16Bit converter;
	converter.signed_value = value;
	return converter.unsigned_value;
}


// Returns the contents of a file as a heap allocated string.
char * read_file(char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		return NULL;
	}

	// Get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);

	// Read its contents
	char *contents = malloc(sizeof(char) * (length + 1));
	fread(contents, sizeof(char), length, f);
	fclose(f);
	contents[length] = '\0';
	return contents;
}
