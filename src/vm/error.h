
//
//  Error
//

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <hydrogen.h>

#include "vm.h"
#include "lexer.h"

// Triggers a custom error.
void err_new(VirtualMachine *vm, Token *token, char *fmt, ...);

// Triggers an unexpected token error.
void err_unexpected(VirtualMachine *vm, Token *token, char *fmt, ...);

#endif
