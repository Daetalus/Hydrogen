
//
//  Error
//


#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

#include "vm.h"
#include "lexer.h"


// Triggers a custom fatal error on the VM.
void err_fatal(VirtualMachine *vm, Lexer *lexer, char *fmt, ...);

// Triggers an unexpected token error on the VM.
void err_unexpected(VirtualMachine *vm, Lexer *lexer, char *fmt, ...);

#endif
