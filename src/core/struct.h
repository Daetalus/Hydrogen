
//
//  Struct Definitions
//

#ifndef STRUCT_H
#define STRUCT_H

#include <hydrogen.h>

#include <vec.h>


// A struct definition, specifying the fields and methods present on a struct
typedef struct {
	// The name of the struct, used when searching through definitions to
	// resolve an identifier during parsing
	char *name;
	uint32_t length;

	// The package and source code object the struct was defined in
	Index package;
	Index source;

	// The line of in the source code the struct was defined on
	uint32_t line;

	// The struct's constructor function, or `NOT_FOUND` if no constructor is
	// assigned
	Index constructor;

	// The name of all fields on this struct, in the order they were defined
	Vec(Identifier) fields;

	// For each field in the order they appear in the `fields` array, if the
	// field is a method, then this will be the index of the function containing
	// the method's bytecode. Otherwise, the value will be NOT_FOUND
	Vec(Index) methods;
} StructDefinition;


// Creates a new struct definition on the package `pkg`
Index struct_new(HyState *state, Index pkg);

// Frees resources allocated by a struct definition
void struct_free(StructDefinition *def);

// Returns the index of the struct with the name `name` that is in the package
// `pkg`
Index struct_find(HyState *state, Index pkg, char *name, uint32_t length);

// Creates a new field on the struct
Index struct_field_new(StructDefinition *def, char *name, uint32_t length);

// Creates a new method on the struct with the function defined at `fn`
Index struct_method_new(StructDefinition *def, char *name, uint32_t length,
	Index fn);

// Returns the index of a field with the name `name`
Index struct_field_find(StructDefinition *def, char *name, uint32_t length);

#endif
