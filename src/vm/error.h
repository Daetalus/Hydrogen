
//
//  Error
//

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <hydrogen.h>

#include "vm.h"
#include "parser/lexer.h"

// Exits back to where the error guard is placed.
void err_jump(VirtualMachine *vm);

// Sets the error on the VM.
void err_new(VirtualMachine *vm, char *fmt, ...);

// Triggers a fatal error on the Vm.
void err_fatal(VirtualMachine *vm, char *fmt, ...);

// Triggers a custom error on a token.
void err_token(VirtualMachine *vm, Token *token, char *fmt, ...);

// Triggers an unexpected token error.
void err_unexpected(VirtualMachine *vm, Token *token, char *fmt, ...);

#endif
