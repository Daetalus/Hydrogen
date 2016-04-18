
//
//  Core Libraries
//

#include <stdio.h>

#include "lib.h"
#include "value.h"


// Will evaluate to the largest of two numbers.
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


// Find a core method with the given name.
Index core_method_find(CoreMethod *methods, uint32_t methods_count, char *name,
		uint32_t length) {
	for (uint32_t i = 0; i < methods_count; i++) {
		CoreMethod *method = &methods[i];
		if (length == strlen(method->name) &&
				strncmp(name, method->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}



//
//  Strings
//

// Return the number of characters in a string.
HyValue string_len(HyState *state, void *obj, HyArgs *args) {
	return num_to_val((double) ((String *) obj)->length);
}



//
//  Arrays
//

// Return the number of elements in an array.
HyValue array_len(HyState *state, void *obj, HyArgs *args) {
	return num_to_val((double) ((Array *) obj)->length);
}


// Increase the capacity of an array to hold at least `minimum` elements.
static void array_resize(Array *array, uint32_t minimum) {
	if (array->capacity < minimum) {
		array->capacity = MAX(array->capacity *= 2, minimum);
		uint32_t new_size = sizeof(HyValue) * array->capacity;
		array->contents = realloc(array->contents, new_size);
	}
}


// Append an element to the end of an array.
HyValue array_push(HyState *state, void *obj, HyArgs *args) {
	Array *array = (Array *) obj;

	// Resize the array to hold the required number of additional elements
	array_resize(array, array->length + hy_args_count(args));

	// Increment the length
	uint32_t start = array->length;
	array->length += hy_args_count(args);

	// Add each element in the arguments
	for (uint32_t i = 0; i < hy_args_count(args); i++) {
		array->contents[start + i] = hy_arg(args, i);
	}

	return VALUE_NIL;
}


// Insert an element into an array at the specified index.
HyValue array_insert(HyState *state, void *obj, HyArgs *args) {
	// Get the index to insert at
	int32_t index = (int32_t) hy_expect_number(hy_arg(args, 0));
	if (index < 0) {
		// TODO: Trigger error
	}

	Array *array = (Array *) obj;

	// Resize the array to hold the additional element
	array_resize(array, array->length + 1);

	// Move everything after the index right one element
	int32_t size = (array->length - index) * sizeof(HyValue);
	array->length++;
	if (size > 0) {
		memmove(&array->contents[index + 1], &array->contents[index], size);
	}

	// Set the element
	array->contents[index] = hy_arg(args, 1);

	return VALUE_NIL;
}


// Remove an element from an array at a specified index.
HyValue array_remove(HyState *state, void *obj, HyArgs *args) {
	// Get the index to remove the value at
	int32_t index = (int32_t) hy_expect_number(hy_arg(args, 0));
	if (index < 0) {
		// TODO: Trigger error
	}

	Array *array = (Array *) obj;

	// Move everything to the right of the index left one element
	int32_t size = (array->length - index - 1) * sizeof(HyValue);
	if (size > 0) {
		memmove(&array->contents[index], &array->contents[index + 1], size);
	}
	array->length--;

	return VALUE_NIL;
}


// Remove the last element from the array, and return it.
HyValue array_pop(HyState *state, void *obj, HyArgs *args) {
	Array *array = (Array *) obj;

	// Save the element
	HyValue value = array->contents[array->length - 1];

	// Decrement the length
	array->length--;

	// Return the last element
	return value;
}
