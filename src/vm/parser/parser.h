
//
//  Parser
//

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdbool.h>

#include "../vm.h"
#include "../error.h"

#include "lexer.h"


// * The parser converts lexed source code into bytecode
// * A `Parser` struct is used for each function
// * The top level source of a file (not inside a function) is treated as the
//   package's main function
// * A function has a main block and arguments
// * A block consists of a series of statements (eg. if, while, loop, for, etc.)
// * A statement itself may have another block (eg. while loops), which is
//   parsed recursively
//
// * Variables (locals) are stored in a stack in the order they were defined
// * Each local stores the scope depth at which it was defined
// * A new scope is defined at the start of each block and freed at the end of
//   the block
// * When a scope is freed, all variables defined in that scope are freed


// Triggers a custom error.
#define ERROR(...) {                      \
	VirtualMachine *vm = parser->vm;      \
	Token *token = &parser->lexer->token; \
	parser_free(parser);                  \
	err_new(vm, token, __VA_ARGS__);      \
}


// Triggers an unexpected token error.
#define UNEXPECTED(...) {                   \
	VirtualMachine *vm = parser->vm;        \
	Token *token = &parser->lexer->token;   \
	parser_free(parser);                    \
	err_unexpected(vm, token, __VA_ARGS__); \
}


// Triggers an unexpected token error if the current token does not match the
// given one.
#define EXPECT(expected, ...)                      \
	if (parser->lexer->token.type != (expected)) { \
		UNEXPECTED(__VA_ARGS__);                   \
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

// Frees resources allocated by a parser.
void parser_free(Parser *parser);

// Returns true if a parser is currently parsing the top level of a file.
bool parser_is_top_level(Parser *parser);

// Parses a block of statements, terminated by `terminator`.
void parse_block(Parser *parser, TokenType terminator);

#endif
