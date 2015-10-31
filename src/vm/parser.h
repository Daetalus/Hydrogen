
//
//  Parser
//


#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

#include "vm.h"
#include "lexer.h"


// Parses a package into bytecode. Sets the main function
// index property on the package.
void parse_package(VirtualMachine *vm, Package *package);

#endif
