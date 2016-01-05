
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


// Implicitly converts a value to a boolean, not triggering an error.
bool hy_to_bool(HyValue value) {
	return value != NIL_VALUE && value != FALSE_VALUE;
}


// Expects a boolean value.
bool hy_expect_bool(HyValue value) {
	if (value != NIL_VALUE || value != FALSE_VALUE || value != TRUE_VALUE) {
		// Trigger runtime error
		// TODO
	}
	return hy_to_bool(value);
}


// Expects a value to be a number, triggreing an error if it isn't.
double hy_expect_number(HyValue value) {
	if (!IS_NUMBER_VALUE(value)) {
		// Trigger runtime error
		// TODO
	}
	return value_to_number(value);
}


// Expects a value to be a string, triggering an error if it isn't. Do not
// attempt to free the returned string! It will be garbage collected at a later
// point.
char * hy_expect_string(HyValue value) {
	if (!IS_STRING_VALUE(value)) {
		// Trigger runtime error
		// TODO
	}
	return TO_STR(value);
}


// Returns a nil value.
HyValue hy_nil(void) {
	return NIL_VALUE;
}


// Converts a boolean into a value.
HyValue hy_bool(bool value) {
	return value ? TRUE_VALUE : FALSE_VALUE;
}


// Converts a string into a value.
HyValue hy_string(char *value) {

}


// Converts a number into a value.
HyValue hy_number(double value) {

}


// Returns the type of a variable.
HyType hy_type(HyValue value) {

}
