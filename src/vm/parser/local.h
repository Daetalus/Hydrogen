
//
//  Locals
//

#ifndef LOCAL_H
#define LOCAL_H

#include <stdlib.h>

#include "parser.h"


// A local variable.
struct local {
	// The name of the local.
	char *name;
	size_t length;

	// The scope depth the local was defined at.
	uint32_t scope_depth;

	// The index of the upvalue in the VM's upvalue list, or -1 if this local
	// wasn't used in a closure.
	int upvalue_index;
};


// The type of a variable.
typedef enum {
	VAR_UNDEFINED,
	VAR_LOCAL,
	VAR_UPVALUE,
	VAR_PACKAGE,
	VAR_TOP_LEVEL,
} VariableType;


// A variable (upvalue or local).
typedef struct {
	// The type of the variable.
	VariableType type;

	// Depending on the variable type:
	// * Local: position on stack
	// * Upvalue: index in VM's upvalue list
	// * Package: index of package in VM's package list
	// * Top level: index of top level variable in the current function's
	//   package
	uint16_t slot;
} Variable;


// Creates a new local at the top of the locals stack. Returns NULL if a local
// couldn't be allocated.
Local * local_new(Parser *parser, uint16_t *slot);

// Searches a parser's locals list for a local called `name`, returning its
// stack slot index, or -1 if the local doesn't exist.
int local_find(Parser *parser, char *name, size_t length);

// Searches parent compiler locals recursively for a local called `name`,
// returning the index of a new upvalue created from the local, or -1 if no such
// local could be found.
int local_find_all(Parser *parser, char *name, size_t length);

// Returns a local or upvalue with the given name. First searches the parser's
// locals list, then the existing upvalues, then parent parsers' locals.
Variable local_capture(Parser *parser, char *name, size_t length);

// Returns true if a variable name already exists
bool local_exists(Parser *parser, char *name, size_t length);

// Emits close upvalue instructions for all locals still on the parser's local
// stack.
void local_close_upvalues(Parser *parser);

// Increments the parser's scope depth.
void scope_new(Parser *parser);

// Decrements the parser's scope depth, removing all locals from the stack
// created in that scope and closing any upvalues.
void scope_free(Parser *parser);

#endif
