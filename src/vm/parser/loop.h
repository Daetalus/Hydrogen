
//
//  Loop Parsing
//

#ifndef LOOP_H
#define LOOP_H

#include <stdlib.h>

#include "parser.h"

// Parses an infinite loop.
void parse_loop(Parser *parser);

// Parses a while loop.
void parse_while(Parser *parser);

// Parses a break statement.
void parse_break(Parser *parser);

#endif
