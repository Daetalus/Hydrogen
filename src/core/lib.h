
//
//  Core Libraries
//

#ifndef LIB_H
#define LIB_H

#include <hydrogen.h>
#include <vec.h>


//
//  Strings
//

// Return the number of characters in a string.
HyValue string_len(HyState *state, void *string, HyArgs *args);



//
//  Arrays
//

// Return the number of elements in an array.
HyValue array_len(HyState *state, void *obj, HyArgs *args);

// Append an element to the end of an array.
HyValue array_push(HyState *state, void *obj, HyArgs *args);

// Insert an element into an array at the specified index.
HyValue array_insert(HyState *state, void *obj, HyArgs *args);

// Remove an element from an array at a specified index.
HyValue array_remove(HyState *state, void *obj, HyArgs *args);

// Remove the last element from the array, and return it.
HyValue array_pop(HyState *state, void *obj, HyArgs *args);



//
//  Core Methods
//

// The nubmer of methods defined on strings.
#define STRING_CORE_METHODS_COUNT 1

// The number of methods defined on arrays.
#define ARRAY_CORE_METHODS_COUNT 5


// A method available on a core data type.
typedef struct {
	// The name of the method.
	char *name;

	// The number of arguments the method accepts.
	uint32_t arity;

	// The native method to call.
	HyNativeMethod fn;
} CoreMethod;


// A list of core methods on strings.
static CoreMethod string_core_methods[STRING_CORE_METHODS_COUNT] = {
	{"len", 0, string_len},
};


// A list of core methods on arrays.
static CoreMethod array_core_methods[ARRAY_CORE_METHODS_COUNT] = {
	{"len", 0, array_len},
	{"push", HY_VAR_ARG, array_push},
	{"insert", 2, array_insert},
	{"remove", 1, array_remove},
	{"pop", 0, array_pop},
};


// Find a core method with the given name.
Index core_method_find(CoreMethod *methods, uint32_t methods_count, char *name,
	uint32_t length);

#endif
