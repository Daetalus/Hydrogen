
//
//  Error
//

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

#include "vm.h"
#include "parser/lexer.h"

// Creates a new error. Doesn't trigger the jump back to the error guard.
void err_new(VirtualMachine *vm, char *fmt, ...);

// Exits back to where the error guard is placed.
void err_jump(VirtualMachine *vm);

// Triggers a fatal error.
void err_fatal(VirtualMachine *vm, char *fmt, ...);

// Triggers a fatal error on a particular token.
void err_token(VirtualMachine *vm, Token *token, char *fmt, ...);

// Triggers an unexpected token error.
void err_unexpected(VirtualMachine *vm, Token *token, char *fmt, ...);

#endif
