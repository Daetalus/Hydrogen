
//
//  Errors
//

#ifndef ERR_H
#define ERR_H

#include <hydrogen.h>
#include <stdarg.h>

// Triggers a fatal error, setting the error on the interpreter state and
// returning execution to the error guard.
void err_fatal(HyState *state, char *fmt, ...);

#endif
