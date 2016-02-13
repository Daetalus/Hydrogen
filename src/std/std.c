
//
//  Standard Library
//

#include <hystdlib.h>
#include <stdio.h>


// Register the entire standard library.
void hy_add_stdlib(HyState *state) {
	hy_add_io(state);
}



//
//  IO
//

// Prints a value to the standard output, returning the number of characters
// printed.
static uint32_t io_print_value(HyState *state, HyValue value) {
	switch (hy_type(value)) {
	case HY_NIL:
		return printf("nil");
	case HY_BOOL:
		return hy_to_bool(value) ? printf("true") : printf("false");
	case HY_NUMBER:
		return printf("%.15g", hy_expect_number(value));
	case HY_STRING:
		return printf("%s", hy_expect_string(value));
	case HY_STRUCT:
		return printf("struct");
	case HY_FUNCTION:
		return printf("fn");
	}
}


// Prints a line to the standard output without a trailing newline.
static HyValue io_print(HyState *state, HyArgs *args) {
	uint32_t length = 0;
	for (uint32_t i = 0; i < hy_args_count(args); i++) {
		length += io_print_value(state, hy_arg(args, i));
	}
	return hy_number((double) length);
}


// Prints a line to the standard output with a trailing newline.
static HyValue io_println(HyState *state, HyArgs *args) {
	HyValue result = io_print(state, args);
	printf("\n");
	return result;
}


// Register the IO library.
void hy_add_io(HyState *state) {
	HyPackage pkg = hy_add_pkg(state, "io");
	hy_add_native(state, pkg, "print", -1, io_print);
	hy_add_native(state, pkg, "println", -1, io_println);
}
