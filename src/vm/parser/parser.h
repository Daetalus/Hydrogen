
//
//  Parser
//

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

#include "../vm.h"
#include "../error.h"

#include "lexer.h"


// Triggers a custom error.
#define ERROR(...) \
	err_new(parser->vm, &parser->lexer->token, __VA_ARGS__);


// Triggers an unexpected token error.
#define UNEXPECTED(...) \
	err_unexpected(parser->vm, &parser->lexer->token, __VA_ARGS__);


// Triggers an unexpected token error if the current token does not match the
// given one.
#define EXPECT(expected, ...)                      \
	if (parser->lexer->token.type != (expected)) { \
		UNEXPECTED(__VA_ARGS__);                   \
		return;                                    \
	}


// A local variable.
typedef struct local Local;

// Data about a loop required by the parser in order to implement break
// statements.
typedef struct loop {
	// The index of the last break statement's jump instruction in the bytecode.
	// Used to form a jump list which can be patched after the loop has finished
	// being compiled. -1 if no break statements are used.
	int jump;

	// The next loop in the linked list, used by the parser so we can keep track
	// of which loop to break out of when we hit a break statement.
	struct loop *outer;
} Loop;


// A parser, which converts lexed source code into bytecode.
typedef struct _parser {
	// The virtual machine we're parsing for.
	VirtualMachine *vm;

	// A pointer to the parent parser. NULL if this parser is top level.
	struct _parser *parent;

	// The lexer.
	Lexer *lexer;

	// The function we're compiling the source code into.
	Function *fn;

	// The innermost loop being parsed, or NULL if we're not inside a loop.
	// Stored as a linked list.
	Loop *loop;

	// The current scope depth.
	uint32_t scope_depth;

	// All defined locals.
	ARRAY(Local, locals);

	// All imported packages.
	ARRAY(Package *, imports);
} Parser;


// Creates a new function on `vm`, used as `package`'s main function, and
// populates the function's bytecode based on `package`'s source code.
void parse_package(VirtualMachine *vm, Package *package);

// Creates a new parser. Does not create a new function for the parser.
Parser parser_new(Parser *parent);

// Parses a block of statements, terminated by `terminator`.
void parse_block(Parser *parser, TokenType terminator);

#endif
