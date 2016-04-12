
//
//  Value
//

#include "value.h"
#include "fn.h"


// Returns a nil value
HyValue hy_nil(void) {
	return VALUE_NIL;
}


// Converts a boolean into a value
HyValue hy_bool(bool boolean) {
	return prim_to_val(boolean ? TAG_TRUE : TAG_FALSE);
}


// Converts a number into a value
HyValue hy_number(double number) {
	return num_to_val(number);
}


// Copies a string into a garbage collected value
HyValue hy_string(HyState *state, char *string) {
	uint32_t length = strlen(string);
	String *obj = malloc(sizeof(String) + length + 1);
	obj->type = OBJ_STRING;
	obj->length = length;
	strcpy(&obj->contents[0], string);
	return ptr_to_val(obj);
}


// Returns the type of a value
HyType hy_type(HyValue value) {
	if (value == VALUE_NIL) {
		return HY_NIL;
	} else if (value == VALUE_TRUE || value == VALUE_FALSE) {
		return HY_BOOL;
	} else if (val_is_num(value)) {
		return HY_NUMBER;
	} else if (val_is_str(value)) {
		return HY_STRING;
	} else if (val_is_struct(value)) {
		return HY_STRUCT;
	} else if (val_is_array(value)) {
		return HY_ARRAY;
	} else if (val_is_fn(value) || val_is_native(value)) {
		return HY_FUNCTION;
	} else {
		return HY_NIL;
	}
}


// Returns true if a value is nil or not
bool hy_is_nil(HyValue value) {
	return value == HY_NIL;
}


// Converts a value to a boolean, ignoring the type of the value
bool hy_to_bool(HyValue value) {
	return value != VALUE_FALSE && value != VALUE_NIL;
}


// Converts a value into a boolean, triggering an error if the value is not a
// boolean in type
bool hy_expect_bool(HyValue value) {
	if (value != VALUE_TRUE && value != VALUE_FALSE) {
		// TODO: Trigger error
		return false;
	}
	return value == VALUE_TRUE;
}


// Converts a value into a number, triggering an error if the value isn't a
// number
double hy_expect_number(HyValue value) {
	if (!val_is_num(value)) {
		// TODO: Trigger error
		return 0.0;
	}
	return val_to_num(value);
}


// Converts a value into a string, triggering an error if it isn't a string
//
// Do not try and free the returned string. It will be garbage collected later.
//
// Don't modify the returned string. It should be treated as read only, and a
// copy should be made if you want to modify it.
char * hy_expect_string(HyValue value) {
	if (!val_is_str(value)) {
		// TODO: Trigger error
		return NULL;
	}
	return &(((String *) val_to_ptr(value))->contents[0]);
}


// Converts a value into an array, triggering an error if it isn't one
HyArray * hy_expect_array(HyValue value) {
	if (!val_is_array(value)) {
		// TODO: Trigger error
		return NULL;
	}
	return (Array *) val_to_ptr(value);
}



//
//  Function Arguments
//

// Returns the number of arguments passed to a native function
uint32_t hy_args_count(HyArgs *args) {
	return args->arity;
}


// Returns the `index`th argument passed to a native function
HyValue hy_arg(HyArgs *args, uint32_t index) {
	if (index >= args->arity) {
		return VALUE_NIL;
	} else {
		return args->stack[args->start + index];
	}
}



//
//  Arrays
//

// Create an empty array with the suggested capacity
HyArray * hy_array_new(uint32_t capacity) {
	HyArray *array = malloc(sizeof(HyArray));
	array->type = OBJ_ARRAY;
	array->length = 0;
	array->capacity = ceil_power_of_2(capacity);
	array->contents = malloc(sizeof(HyValue) * array->capacity);
	return array;
}


// Returns the length of an array
uint32_t hy_array_len(HyArray *array) {
	return array->length;
}


// Fetches a value at an index in an array
HyValue hy_array_get(HyArray *array, uint32_t index) {
	if (index >= array->length) {
		// TODO: Trigger out of bounds error
		return VALUE_NIL;
	}
	return array->contents[index];
}


// Appends a value to the end of an array
void hy_array_append(HyArray *array, HyValue value) {
	// TODO
}


// Inserts a value into an array at the specified index
void hy_array_insert(HyArray *array, uint32_t index, HyValue value) {
	// TODO
}
