
//
//  Value
//

#include "value.h"


// A union to convert between doubles and unsigned 8 byte integers without
// implicit conversion.
typedef union {
	double number;
	uint64_t value;
	void *ptr;
} Converter;


// Converts a value into a number.
double value_to_number(uint64_t value) {
	Converter convert;
	convert.value = value;
	return convert.number;
}


// Converts a number into a value.
uint64_t number_to_value(double number) {
	Converter convert;
	convert.number = number;
	return convert.value;
}


// Converts a value into a pointer.
void * value_to_ptr(uint64_t value) {
	Converter convert;
	convert.value = (value & ~(QUIET_NAN | SIGN));
	return convert.ptr;
}


// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr) {
	Converter convert;
	convert.ptr = ptr;
	return (convert.value) ^ (QUIET_NAN | SIGN);
}
