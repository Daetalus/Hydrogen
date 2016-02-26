
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
#define vec_new(array, type, initial_capacity)                           \
	(array).values = (type *) malloc(sizeof(type) * (initial_capacity)); \
	(array).element_size = sizeof(type);                                 \
	(array).length = 0;                                                  \
	(array).capacity = (initial_capacity);


// Frees a vector.
#define vec_free(array) (free((array).values))


// Evaluates to the element at an index (used for setting or getting an
// element).
#define vec_at(array, index) ((array).values[(index)])


// Returns the length of a vector.
#define vec_len(array) ((array).length)


// Returns a pointer to the last element in the vector.
#define vec_last(array) (vec_at(array, vec_len(array) - 1))


// Returns the capacity of a vector.
#define vec_capacity(array) ((array).capacity)


// Increases the capacity of a vector if the length of the array plus 1 exceeds
// the current capacity.
#define vec_resize(array, limit, new_capacity)                          \
	if ((limit) > vec_capacity(array)) {                                \
		vec_capacity(array) = (new_capacity);                           \
		uint32_t new_size = vec_capacity(array) * (array).element_size; \
		void **values = (void **) &(array).values;                      \
		*values = (void *) realloc((array).values, new_size);           \
	}


// Increases the length of the vector by 1.
#define vec_inc(array)                                              \
	vec_resize(array, vec_len(array) + 1, vec_capacity(array) * 2); \
	(array).length++;


// Inserts an element into the vector at `index`.
#define vec_insert(array, index, value) {                                  \
	int32_t size = (vec_len(array) - index) * (array).element_size;        \
	vec_inc(array);                                                        \
	if (size > 0) {                                                        \
		memmove(&vec_at(array, (index) + 1), &vec_at(array, index), size); \
	}                                                                      \
	vec_at(array, index) = (value);                                        \
}


// Removes the element at `index`.
#define vec_remove(array, index) {                                         \
	int32_t size = (vec_len(array) - (index) - 1) * (array).element_size;  \
	if (size > 0) {                                                        \
		memmove(&vec_at(array, index), &vec_at(array, (index) + 1), size); \
	}                                                                      \
	vec_len(array)--;                                                      \
}

#endif
