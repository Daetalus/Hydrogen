
//
//  Parser
//

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

#include "vm.h"
#include "lexer.h"

// Creates a new function on `vm`, used as `package`'s main function, and
// populates the function's bytecode based on `package`'s source code.
void parse_package(VirtualMachine *vm, Package *package);

#endif
