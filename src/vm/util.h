
//
//  Utilities
//

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>


// A string with a specified length (rather than NULL terminated).
typedef struct {
	char *start;
	size_t length;
} Identifier;


// Returns the contents of a file as a heap allocated string.
char * read_file(char *path);


// Creates a new array in a struct.
#define ARRAY(type, name)       \
    type *name;                 \
    uint32_t name ## _count;    \
    uint32_t name ## _capacity; \


// Initialises an array.
#define ARRAY_INIT(array, type, initial_capacity) \
	array ## _count = 0;                          \
	array ## _capacity = (initial_capacity);      \
	array = (type *) malloc(array ## _capacity * sizeof(type));


// Increases the capacity of the array if its size is larger than it's capacity.
// You need to increase the size of the array before actually setting any data.
#define ARRAY_REALLOC(array, type)                                          \
    if (array ## _count > array ## _capacity) {                             \
		if (array ## _count <= array ## _capacity * 2) {                    \
			array ## _capacity *= 2;                                        \
		} else {                                                            \
			array ## _capacity = array ## _count + 4;                       \
		}                                                                   \
		array = (type *) realloc(array, array ## _capacity * sizeof(type)); \
    }

#endif
