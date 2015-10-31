
//
//  Utilities
//


#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>


// Converts a double into an unsigned 64 bit integer.
uint64_t double_to_uint64(double value);

// Converts an unsigned 64 bit integer into a double.
double uint64_t_to_double(uint64_t value);

// Converts an unsigned 16 bit integer to a signed 16 bit integer.
int16_t uint16_to_int16(uint16_t value);

// Converts a signed 16 bit integer to an unsigned 16 bit integer.
uint16_t int16_to_uint16(int16_t value);


// Creates a new array in a struct.
#define ARRAY(type, name)       \
    type name;                  \
    uint32_t name ## _count;    \
    uint32_t name ## _capacity; \


// Initialises an array.
#define ARRAY_INIT(array, type, initial_capacity) \
	array ## _count = 0;                          \
	array ## _capacity = (initial_capacity);      \
	array = malloc(array ## _capacity * sizeof(type));


// Increases the capacity of the array if its size is
// larger than it's capacity. You need to increase the
// size of the array before actually setting any data.
#define ARRAY_REALLOC(array, type)                                 \
    if (array ## _count > array ## _capacity) {                    \
		if (array ## _count <= array ## _capacity * 2) {           \
			array ## _capacity *= 2;                               \
		} else {                                                   \
			array ## _capacity = array ## _count + 4;              \
		}                                                          \
		array = realloc(array, array ## _capacity * sizeof(type)); \
    }

#endif
