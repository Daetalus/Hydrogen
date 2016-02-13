
//
//  Standard Library
//

#include <hystdlib.h>
#include <stdio.h>


// Register the entire standard library.
void hy_add_stdlib(HyState *state) {
	hy_add_io(state);
	hy_add_err(state);
}



//
//  IO
//

// Prints a line to the standard output without a trailing newline.
static HyValue io_print(HyState *state, HyArgs *args) {
	printf("YES!\n");
	return hy_nil();
}


// Prints a line to the standard output with a trailing newline.
static HyValue io_println(HyState *state, HyArgs *args) {
	return hy_nil();
}


// Register the IO library.
void hy_add_io(HyState *state) {
	HyPackage pkg = hy_add_pkg(state, "io");
	hy_add_native(state, pkg, "print", -1, io_print);
	hy_add_native(state, pkg, "println", -1, io_println);
}
