
//
//  Struct Parsing
//

#ifndef STRUCT_H
#define STRUCT_H

#include "parser.h"

// Parses a struct definition.
void parse_struct_definition(Parser *parser);

// Parses a struct instantiation, storing the resulting struct into `slot`.
void parse_struct_instantiation(Parser *parser, uint16_t slot);

#endif
