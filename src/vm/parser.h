
//
//  Parser
//

#ifndef PARSER_H
#define PARSER_H

#include <hydrogen.h>
#include <stdbool.h>

#include "vec.h"
#include "lexer.h"
#include "bytecode.h"


// Data associated with a loop so we know where to point break statement's jump
// instructions once the whole loop has been compiled.
typedef struct loop {
	// The outer loop this one is contained inside of, or NULL if there are no
	// enclosing loops.
	struct loop *parent;

	// The head of the jump list for all break statements inside this loop.
	Index head;
} Loop;


// Each function is parsed in their own scope.
typedef struct function_scope {
	// The parent function scope in which this function is defined (or NULL if
	// this function scope is the top level of a package).
	struct function_scope *parent;

	// The index of the function being parsed in the interpreter's function
	// list. Bytecode instructions are emitted into this function.
	Index fn_index;

	// Set to true when this function scope is a method on a struct.
	bool is_method;

	// The base local in the parser's locals stack which begins the locals
	// defined inside this function. The instance of the struct (`self`) is
	// first (present only if this is a method), then the arguments to the
	// function, then the locals defined in the function.
	uint32_t first_local;

	// A linked list of loops so we know which loop to break out of when we
	// encounter a break statement. The inner most loop is stored at the head
	// of the linked list (this pointer).
	Loop *loop;
} FunctionScope;


// A named local variable on the parser's locals stack.
typedef struct {
	// The name of this variable.
	char *name;
	uint32_t length;
} Local;


// Parses source code into bytecode instructions.
typedef struct {
	// A pointer to the interpreter state that functions, packages, etc will be
	// created on.
	HyState *state;

	// The index of the package the source code we're compiling is associated
	// with.
	Index package;

	// The lexer, emitting tokens from source code that the parser transforms
	// into more cohesive language structures.
	Lexer lexer;

	// All permanent locals defined on the stack by the source code we're
	// parsing. The length of this vector is the number of active locals (ie.
	// locals that have been given a name). Expressions, function calls, etc
	// all use temporary locals on top of these.
	Vec(Local) locals;

	// The actual number of locals (including temporary ones).
	uint32_t locals_count;

	// Each function is parsed in its own scope. Functions defined inside other
	// functions have their scopes linked together by a linked list. The head
	// of the linked list (this pointer) is the inner most function (the one
	// currently being parsed).
	FunctionScope *scope;
} Parser;


// Creates a new parser, which will append all functions, packages, etc it needs
// to define to the interpreter `state`, associating them with the package
// `pkg`.
Parser parser_new(HyState *state, Index pkg);

// Releases resources allocated by a parser.
void parser_free(Parser *parser);

// Parses some source code, creating a function for the top level code in the
// source. Returns the index of this function.
Index parser_parse(Parser *parser, Index source);

#endif
