
//
//  Locals
//

#include <string.h>

#include "local.h"
#include "../bytecode.h"


// Creates a new local at the top of the locals stack. Returns NULL if a local
// couldn't be allocated.
Local * local_new(Parser *parser, uint16_t *slot) {
	uint16_t index = parser->locals_count++;
	if (slot != NULL) {
		*slot = index;
	}

	ARRAY_REALLOC(parser->locals, Local);
	Local *local = &parser->locals[index];
	local->name = NULL;
	local->length = 0;
	local->scope_depth = parser->scope_depth;
	local->upvalue_index = -1;
	return local;
}


// Searches a parser's locals list for a local called `name`, returning its
// stack slot index, or -1 if the local doesn't exist.
int local_find(Parser *parser, char *name, size_t length) {
	// Iterate over the locals backwards, as locals are more likely to be used
	// right after they've been defined (maybe)
	for (int i = parser->locals_count - 1; i >= 0; i--) {
		Local *local = &parser->locals[i];

		// Check if this is the correct local
		if (local->length == length && strncmp(local->name, name, length) == 0) {
			// Return the slot the local is in
			return i;
		}
	}

	return -1;
}


// Searches parent compiler locals recursively for a local called `name`,
// returning the index of a new upvalue created from the local, or -1 if no such
// local could be found.
int local_find_all(Parser *parser, char *name, size_t length) {
	if (parser == NULL) {
		// No more parent compilers, local is undefined
		return -1;
	}

	int slot = local_find(parser, name, length);
	if (slot >= 0) {
		Function *fn = parser->fn;

		// Create an upvalue from the local
		int index;
		Upvalue *upvalue = upvalue_new(parser->vm, &index);
		upvalue->name = name;
		upvalue->length = length;
		upvalue->slot = slot;
		fn->defined_upvalues[fn->defined_upvalues_count++] = upvalue;

		// Set the local as an upvalue
		Local *local = &parser->locals[slot];
		local->upvalue_index = index;

		return index;
	} else {
		// Search parent compiler
		return local_find_all(parser->parent, name, length);
	}
}


// Returns a local or upvalue with the given name. First searches the parser's
// locals list, then the existing upvalues, then parent parsers' locals.
Variable local_capture(Parser *parser, char *name, size_t length) {
	Variable result;
	int slot;

	// Search parser locals
	slot = local_find(parser, name, length);
	if (slot >= 0) {
		result.type = VAR_LOCAL;
		result.slot = slot;
		return result;
	}

	// Search existing upvalues
	slot = upvalue_find(parser->vm, name, length);
	if (slot >= 0) {
		result.type = VAR_UPVALUE;
		result.slot = slot;
		return result;
	}

	// Search parent locals
	slot = local_find_all(parser->parent, name, length);
	if (slot >= 0) {
		result.type = VAR_UPVALUE;
		result.slot = slot;
		return result;
	}

	result.type = VAR_UNDEFINED;
	return result;
}


// Returns true if a local exists in a parent compiler.
bool local_exists_all(Parser *parser, char *name, size_t length) {
	if (parser == NULL) {
		// Reached top level compiler, variable must be unique
		return false;
	}

	return (local_find(parser, name, length) >= 0) ||
		local_exists_all(parser->parent, name, length);
}


// Returns true if a variable name already exists
bool local_exists(Parser *parser, char *name, size_t length) {
	// Parser locals
	if (local_find(parser, name, length) >= 0) {
		return true;
	}

	// Existing upvalues
	if (upvalue_find(parser->vm, name, length) >= 0) {
		return true;
	}

	// Parent locals
	// Start at the parent compiler since we've already searched this compiler's
	// locals
	if (local_exists_all(parser->parent, name, length)) {
		return true;
	}

	// Structs
	if (struct_find(parser->vm, name, length, NULL) != NULL) {
		return true;
	}

	return false;
}


// Emits close upvalue instructions for all locals still on the parser's local
// stack.
void local_close_upvalues(Parser *parser) {
	// Emit in reverse order
	for (int i = parser->locals_count; i >= 0; i--) {
		int upvalue = parser->locals[i].upvalue_index;
		if (upvalue >= 0) {
			// Emit close upvalue instruction
			emit(parser->fn, instr_new(UPVALUE_CLOSE, upvalue, 0, 0));
		}
	}
}


// Increments the parser's scope depth.
void scope_new(Parser *parser) {
	parser->scope_depth++;
}


// Decrements the parser's scope depth, removing all locals from the stack
// created in that scope and closing any upvalues.
void scope_free(Parser *parser) {
	parser->scope_depth--;

	// Since the locals are stored in order of stack depth, with the locals
	// allocated in the deepest scope stored at the end of the array,
	// continually decrease the size of the array until we hit a local in a
	// scope that is still active
	int i = parser->locals_count - 1;
	while (i >= 0 && parser->locals[i].scope_depth > parser->scope_depth) {
		// Check if the local was used as an upvalue
		int upvalue = parser->locals[i].upvalue_index;
		if (upvalue >= 0) {
			// Close the upvalue
			emit(parser->fn, instr_new(UPVALUE_CLOSE, upvalue, 0, 0));
		}

		parser->locals_count--;
		i--;
	}
}
