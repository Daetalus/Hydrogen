
//
//  Error Library
//

#include <hystdlib.h>
#include "io.h"


// Print a string to the standard error output without a trailing newline.
HyValue err_print(HyVM *vm, HyArgs *args) {
	return io_fprint(stderr, vm, args);
}


// Print a string to the standard error output with a trailing newline.
HyValue err_println(HyVM *vm, HyArgs *args) {
	return io_fprintln(stderr, vm, args);
}


// Register the error library.
void hy_add_err(HyVM *vm) {
	HyNativePackage *package = hy_package_new(vm, "err");
	hy_fn_new(package, "print", -1, err_print);
	hy_fn_new(package, "println", -1, err_println);
}
