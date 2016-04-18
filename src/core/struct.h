
//
//  Struct Definitions
//

#ifndef STRUCT_H
#define STRUCT_H

#include <hydrogen.h>
#include <value.h>
#include <vec.h>


// A struct definition, containing the fields and methods present on a struct.
typedef struct {
	// The name of the struct.
	char *name;
	uint32_t length;

	// The package and source code object the struct was defined in.
	Index package;
	Index source;

	// The line of source code the struct was defined on.
	uint32_t line;

	// The struct's constructor function. If there is no constructor, this value
	// is set to NOT_FOUND.
	Index constructor;

	// The name of all fields on this struct, in the order they were defined.
	Vec(Identifier) fields;

	// For each field (in the order they appear in the `fields` array), if the
	// field is a method, then this will be the index of the function containing
	// the method's bytecode. Otherwise, the value will be NOT_FOUND.
	Vec(Index) methods;
} StructDefinition;


// Create a new struct definition on the package `pkg`.
Index struct_new(HyState *state, Index pkg);

// Free resources allocated by a struct definition.
void struct_free(StructDefinition *def);

// Return the index of the struct with the name `name` in the package `pkg`, or
// NOT_FOUND if one couldn't be found.
Index struct_find(HyState *state, Index pkg, char *name, uint32_t length);

// Create a new field on the struct. Return the index of the field.
Index struct_field_new(StructDefinition *def, char *name, uint32_t length);

// Create a new method on the struct with the function defined at `fn`. Return
// the index of the field the method was created at.
Index struct_method_new(StructDefinition *def, char *name, uint32_t length,
	Index fn);

// Return the index of a field with the name `name`, or NOT_FOUND if one
// couldn't be found.
Index struct_field_find(StructDefinition *def, char *name, uint32_t length);

#endif
