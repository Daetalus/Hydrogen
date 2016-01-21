
//
//  Struct Definitions
//

#include <string.h>

#include "struct.h"
#include "vm.h"


// Creates a new struct definition on the package `pkg`.
Index struct_new(HyState *state, Index pkg) {
	vec_add(state->structs);
	StructDefinition *def = &vec_last(state->structs);
	def->name = NULL;
	def->length = 0;
	def->package = pkg;
	def->constructor = NOT_FOUND;
	vec_new(def->fields, Identifier, 8);
	vec_new(def->values, HyValue, 8);
	return vec_len(state->structs) - 1;
}


// Frees resources allocated by a struct definition.
void struct_free(StructDefinition *def) {
	vec_free(def->fields);
	vec_free(def->values);
}


// Returns the index of the struct with the name `name` that is in the package
// `pkg`.
Index struct_find(HyState *state, Index pkg, char *name, uint32_t length) {
	for (uint32_t i = 0; i < vec_len(state->structs); i++) {
		StructDefinition *def = &vec_at(state->structs, i);
		if (length == def->length && strncmp(name, def->name, length) == 0) {
			return i;
		}
	}
	return NOT_FOUND;
}


// Returns the index of a field with the name `name`.
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
