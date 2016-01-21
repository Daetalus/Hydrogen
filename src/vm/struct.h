
//
//  Struct Definitions
//

#ifndef STRUCT_H
#define STRUCT_H

#include <hydrogen.h>

#include "vec.h"


// A struct definition, specifying the fields and methods present on a struct.
typedef struct {
	// The name of the struct, used when searching through definitions to
	// resolve an identifier during compilation.
	char *name;
	uint32_t length;

	// The package the struct was defined in.
	Index package;

	// The struct's constructor function, or -1 if no constructor is assigned.
	Index constructor;

	// The hash of the name of all fields contained in this struct, and their
	// default values. The default values are `memcpy`'d into instances of the
	// struct when they are instantiated.
	Array(Identifier) fields;
	Array(HyValue) values;
} StructDefinition;


// Creates a new struct definition on the interpreter state.
Index struct_new(HyState *state);

// Returns the index of the struct with the name `name` that is in the package
// `pkg`.
Index struct_find(HyState *state, Index pkg, char *name, uint32_t length);

// Frees resources allocated by a struct definition.
void struct_free(StructDefinition *def);

// Returns the index of a field with the name `name`.
Index struct_field_find(StructDefinition *def, char *name, uint32_t length);

#endif
