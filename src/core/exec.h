
//
//  Execution
//

#ifndef EXEC_H
#define EXEC_H

#include "state.h"

// Execute a function on the interpreter state.
HyError * exec_fn(HyState *state, Index fn);

#endif
