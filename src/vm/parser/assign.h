
//
//  Assignment Parsing
//

#ifndef ASSIGN_H
#define ASSIGN_H

#include "parser.h"

// Parses an assignment to a new variable (using a `let` token).
void parse_initial_assignment(Parser *parser);

// Parses an assignment to an already initialised variable.
void parse_assignment(Parser *parser, Identifier *left, int count);

#endif
