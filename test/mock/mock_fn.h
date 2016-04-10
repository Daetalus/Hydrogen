
//
//  Mock Function
//

#ifndef MOCK_FN_H
#define MOCK_FN_H

#include <hydrogen.h>
#include <fn.h>

// Shorthand for creating a new mock function using vararg expansion of macros.
#define MOCK_FN(name, ...)                                \
	uint16_t bytecode[] = {__VA_ARGS__};                  \
	uint32_t count = sizeof(bytecode) / sizeof(uint16_t); \
	Function name = mock_fn(count, bytecode);

// Create a new mock function.
Function mock_fn(uint32_t count, uint16_t *bytecode);

// Free a mock function.
void mock_fn_free(Function *fn);

#endif
