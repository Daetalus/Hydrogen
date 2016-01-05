
//
//  IO Library
//

#include <stdio.h>

#include <hystdlib.h>


// Prints an arbitrary number of arguments to the standard output, without a
// trailing newline.
HyValue io_print(HyVM *vm, HyArgs *args) {
	printf("Called print\n");
	return hy_nil();
}


// Prints an arbitrary number of arguments to the standard error output, without
// a trailing newline.
HyValue io_print_err(HyVM *vm, HyArgs *args) {
	printf("Called print err");
	return hy_nil();
}


// Register the IO library.
void hy_add_io(HyVM *vm) {
	HyNativePackage *package = hy_package_new(vm, "io");
	hy_fn_new(package, "print", -1, io_print);
	hy_fn_new(package, "print_err", -1, io_print_err);
}
