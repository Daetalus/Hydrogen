
//
//  Vector
//

#ifndef VEC_H
#define VEC_H

#include <stdlib.h>


// An index into a vector.
typedef uint32_t Index;


// A string with an associated length.
typedef struct {
	char *name;
	uint32_t length;
} Identifier;


// Value representing when an element in a vector isn't found.
#define NOT_FOUND (~((uint32_t) 0))


// Generates the type signature for a vector where each element is of type
// `type`.
#define Vec(type)              \
	struct {                   \
		type *values;          \
		uint32_t element_size; \
		uint32_t length;       \
		uint32_t capacity;     \
	}


// Initialises an undefined vector.
#define vec_new(array, type, initial_capacity)                  \
	(array).values = malloc(sizeof(type) * (initial_capacity)); \
	(array).element_size = sizeof(type);                        \
	(array).length = 0;                                         \
	(array).capacity = (initial_capacity);


// Frees a vector.
#define vec_free(array) (free((array).values))


// Evaluates to the element at an index (used for setting or getting an
// element).
#define vec_at(array, index) ((array).values[(index)])


// Returns the length of a vector.
#define vec_len(array) ((array).length)


// Returns a pointer to the last element in the vector.
#define vec_last(array) (&vec_at(array, vec_len(array) - 1))


// Returns the capacity of a vector.
#define vec_capacity(array) ((array).capacity)


// Increases the capacity of a vector if the length of the array plus 1 exceeds
// the current capacity.
#define vec_check_capacity(array)                                       \
	if (vec_len(array) + 1 > vec_capacity(array)) {                     \
		vec_capacity(array) *= 2;                                       \
		uint32_t new_size = vec_capacity(array) * (array).element_size; \
		(array).values = realloc((array).values, new_size);             \
	}


// Increases the length of the vector by 1.
#define vec_add(array)          \
	vec_check_capacity(array);  \
	(array).length++;

#endif
