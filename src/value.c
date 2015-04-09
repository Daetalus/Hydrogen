
//
//  Value
//


#include "value.h"


// A union to convert between doubles and unsigned 8 byte
// integers without implicit value conversion (ie. directly
// converts the bits).
typedef union {
	double number;
	uint64_t value;
	void *ptr;
} Convert;


// Converts a value into a number.
double value_to_number(uint64_t value) {
	Convert convert;
	convert.value = value;
	return convert.number;
}


// Converts a number into a value.
uint64_t number_to_value(double number) {
	Convert convert;
	convert.number = number;
	return convert.value;
}


// Converts a value into a pointer.
void * value_to_ptr(uint64_t value) {
	Convert convert;
	convert.value = (value & ~(QUIET_NAN | SIGN));
	return convert.ptr;
}


// Converts a pointer into a value.
uint64_t ptr_to_value(void *ptr) {
	Convert convert;
	convert.ptr = ptr;
	return (convert.value) ^ (QUIET_NAN | SIGN);
}
