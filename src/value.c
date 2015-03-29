
//
//  Value
//


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
