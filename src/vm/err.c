
//
//  Errors
//

#include <setjmp.h>

#include "err.h"
#include "vm.h"


// Triggers a jump back to the error guard.
static void err_jmp(HyState *state) {
	longjmp(state->error_jmp, 1);
}


// Triggers a fatal error, setting the error on the interpreter state and
// returning execution to the error guard.
void err_fatal(HyState *state, char *fmt, ...) {

}


// Release resources allocated by an error object.
void hy_err_free(HyError *err) {
	free(err->description);
	free(err->file);
	free(err->line);
	free(err->stack_trace);
}
