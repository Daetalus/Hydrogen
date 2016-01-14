
//
//  Loop Parsing
//

#ifndef LOOP_H
#define LOOP_H

#include <stdlib.h>

#include "parser.h"


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


// Parses an infinite loop.
void parse_loop(Parser *parser);

// Parses a while loop.
void parse_while(Parser *parser);

// Parses a break statement.
void parse_break(Parser *parser);

#endif
