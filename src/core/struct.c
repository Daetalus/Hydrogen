
//
//  Struct Definitions
//

#include <string.h>

#include "struct.h"
#include "state.h"


// Create a new struct definition on the package `pkg`.
Index struct_new(HyState *state, Index pkg) {
	vec_inc(state->structs);
	StructDefinition *def = &vec_last(state->structs);
	def->name = NULL;
	def->length = 0;
	def->package = pkg;
	def->line = 0;
	def->constructor = NOT_FOUND;
	vec_new(def->fields, Identifier, 8);
	vec_new(def->methods, Index, 8);
	return vec_len(state->structs) - 1;
}


// Free resources allocated by a struct definition
void struct_free(StructDefinition *def) {
	vec_free(def->fields);
	vec_free(def->methods);
}


// Return the index of the struct with the name `name` in the package `pkg`, or
// NOT_FOUND if one couldn't be found.
Index struct_find(HyState *state, Index pkg, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(state->structs); i++) {
		StructDefinition *def = &vec_at(state->structs, i);
		if (pkg == def->package && length == def->length &&
				strncmp(name, def->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}


// Create a new field or method on the struct, depending on the value of `fn`.
Index struct_field_or_method_new(StructDefinition *def, char *name,
		uint32_t length, Index fn) {
	// Name of the field
	vec_inc(def->fields);
	Identifier *ident = &vec_last(def->fields);
	ident->name = name;
	ident->length = length;

	// Method index
	vec_inc(def->methods);
	vec_last(def->methods) = fn;
	return vec_len(def->fields) - 1;
}


// Create a new field on the struct. Return the index of the field.
Index struct_field_new(StructDefinition *def, char *name, uint32_t length) {
	return struct_field_or_method_new(def, name, length, NOT_FOUND);
}


// Create a new method on the struct with the function defined at `fn`. Return
// the index of the field the method was created at.
Index struct_method_new(StructDefinition *def, char *name, uint32_t length,
		Index fn) {
	return struct_field_or_method_new(def, name, length, fn);
}


// Return the index of a field with the name `name`, or NOT_FOUND if one
// couldn't be found.
Index struct_field_find(StructDefinition *def, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(def->fields); i++) {
		Identifier *ident = &vec_at(def->fields, i);
		if (length == ident->length &&
				strncmp(name, ident->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}



//
//  Native Interface
//

// Add a native struct to a package. `size` specifies how much memory (in bytes)
// needed in each instance of the struct. `constructor` is called every time an
// instance of the struct is instantiated.
HyStruct hy_add_struct(HyState *state, HyPackage pkg, char *name, uint32_t size,
		HyNativeMethod constructor, uint32_t constructor_arity) {

}


// Set the destructor on a native struct, called every time an instance of the
// struct is garbage collected, to allow you to free any associated resources.
void hy_set_destructor(HyState *state, HyStruct def, HyDestructor destructor) {

}


// Add a method on a native struct.
void hy_add_method(HyState *state, HyStruct def, char *name, uint32_t arity,
		HyNativeMethod method) {

}
