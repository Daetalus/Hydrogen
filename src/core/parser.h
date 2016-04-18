
//
//  Parser
//

#ifndef PARSER_H
#define PARSER_H

#include <hydrogen.h>
#include <stdbool.h>
#include <vec.h>

#include "lexer.h"
#include "bytecode.h"


// Data associated with a loop, so we know where to target any jump instructions
// generated from break statements within the loop.
typedef struct loop {
	// The enclosing loop this loop is contained within, or NULL if there are no
	// enclosing loops.
	struct loop *parent;

	// The head of the jump list for all break statements inside this loop.
	Index head;
} Loop;


// Each function is parsed in their own scope.
typedef struct function_scope {
	// The parent function scope in which this function is defined (or NULL if
	// this function is defined in the top level of a package).
	struct function_scope *parent;

	// The index of the function being parsed in the interpreter's function
	// list. Bytecode instructions are emitted into this function.
	Index fn_index;

	// Set to true when this function scope is a method on a struct.
	bool is_method;

	// The start and size of all locals used by this function, including
	// temporary ones.
	uint32_t locals_start;
	uint32_t locals_count;

	// The start and size of all persistent locals (that end up in the parser's
	// `locals` array).
	uint32_t actives_start;
	uint32_t actives_count;

	// A linked list of loops so we know which loop to break out of when we
	// encounter a break statement. The inner most loop is stored at the head
	// of the linked list.
	Loop *loop;

	// The block scope depth inside the function, used to keep track of which
	// locals were defined in the current scope so we can free them when we
	// release a block.
	uint32_t block_depth;
} FunctionScope;


// A named local variable on the parser's locals stack.
typedef struct {
	// The name of this variable.
	char *name;
	uint32_t length;

	// The block scope in which the local was defined.
	uint32_t block;
} Local;


// Parses source code into bytecode instructions.
typedef struct {
	// A pointer to the interpreter state that functions, packages, etc will be
	// defined on.
	HyState *state;

	// The index of the package and source code that we're parsing.
	Index package;
	Index source;

	// The lexer, emitting tokens from source code.
	Lexer lexer;

	// All permanent locals defined on the stack. The length of this vector is
	// the number of active locals (ie. locals that have been given a name).
	// Expressions, function calls, etc all use temporary locals on top of
	// these.
	Vec(Local) locals;

	// A list of packages imported by this file.
	Vec(Index) imports;

	// Each function is parsed in its own scope. Functions defined inside other
	// functions have their scopes linked together by a linked list. The head
	// of the linked list (this pointer) is the inner most function (the one
	// currently being parsed).
	FunctionScope *scope;
} Parser;


// Create a new parser. Will append all newly defined functions, packages, etc.
// to the interpreter `state`, associating them with the package `pkg`.
Parser parser_new(HyState *state, Index pkg);

// Release resources allocated by a parser.
void parser_free(Parser *parser);

// Parse some source code, creating a function for all top level code. Return
// the index of this function.
Index parser_parse(Parser *parser, Index source);

#endif
