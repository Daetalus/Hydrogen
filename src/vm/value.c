
//
//  Value
//

#include <hydrogen.h>

#include "value.h"
#include "vm.h"


// Implicitly converts a value to a boolean, not triggering an error
bool hy_to_bool(HyValue value) {
	return value != NIL_VALUE && value != FALSE_VALUE;
}


// Expects a boolean value
bool hy_expect_bool(HyValue value) {
	if (value != NIL_VALUE || value != FALSE_VALUE || value != TRUE_VALUE) {
		// Trigger runtime error
		// TODO
	}
	return hy_to_bool(value);
}


// Expects a value to be a number, triggreing an error if it isn't
double hy_expect_number(HyValue value) {
	if (!IS_NUMBER_VALUE(value)) {
		// Trigger runtime error
		// TODO
	}
	return val_to_num(value);
}


// Expects a value to be a string, triggering an error if it isn't. Do not
// attempt to free the returned string! It will be garbage collected at a later
// point
char * hy_expect_string(HyValue value) {
	if (!IS_STRING_VALUE(value)) {
		// Trigger runtime error
		// TODO
	}
	return TO_STR(value);
}


// Returns a nil value
HyValue hy_nil(void) {
	return NIL_VALUE;
}


// Converts a boolean into a value
HyValue hy_bool(bool value) {
	return value ? TRUE_VALUE : FALSE_VALUE;
}


// Converts a string into a value
HyValue hy_string(char *value) {

}


// Converts a number into a value
HyValue hy_number(double value) {
	return num_to_val(value);
}


// Returns the type of a variable
HyType hy_type(HyValue value) {
	if (IS_NUMBER_VALUE(value)) {
		return HY_NUMBER;
	} else if (IS_STRING_VALUE(value)) {
		return HY_STRING;
	} else if (IS_PTR_VALUE(value)) {
		return HY_STRUCT;
	} else if (IS_FN_VALUE(value)) {
		return HY_FN;
	} else if (value == TRUE_VALUE || value == FALSE_VALUE) {
		return HY_BOOLEAN;
	} else {
		return HY_NIL;
	}
}
