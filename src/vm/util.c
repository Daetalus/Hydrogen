
//
//  Utilities
//


#include "util.h"


// Converter for 64 bit values.
typedef union {
	double double_value;
	uint64_t uint64_value;
} 64BitConverter;


// Converter for 16 bit values.
typedef union {
	uint16_t unsigned_value;
	int16_t signed_value;
} 16BitConverter;


// Converts a double into an unsigned 64 bit integer.
uint64_t double_to_uint64(double value) {
	64BitConverter converter;
	converter.double_value = value;
	return converter.uint64_value;
}


// Converts an unsigned 64 bit integer into a double.
double uint64_t_to_double(uint64_t value) {
	64BitConverter converter;
	converter.uint64_value = value;
	return converter.double_value;
}


// Converts an unsigned 16 bit integer to a signed 16 bit integer.
int16_t uint16_to_int16(uint16_t value) {
	16BitConverter converter;
	converter.unsigned_value = value;
	return converter.signed_value;
}


// Converts a signed 16 bit integer to an unsigned 16 bit integer.
uint16_t int16_to_uint16(int16_t value) {
	16BitConverter converter;
	converter.signed_value = value;
	return converter.unsigned_value;
}
