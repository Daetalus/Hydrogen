
//
//  IO Library
//

#include <stdio.h>

#include <hystdlib.h>


// Prints an arbitrary number of arguments to the standard output, without a
// trailing newline.
HyValue io_print(HyVM *vm, HyArgs *args) {
	uint32_t arity = hy_args_count(args);
	for (uint32_t i = 0; i < arity; i++) {
		HyValue arg = hy_arg(args, i);
		switch (hy_type(arg)) {
		case HY_NUMBER:
			printf("%f", hy_expect_number(arg));
			break;
		case HY_STRING:
			printf("%s", hy_expect_string(arg));
			break;
		case HY_STRUCT:
		case HY_FN:
			break;
		case HY_BOOLEAN:
			if (hy_to_bool(arg)) {
				printf("true");
			} else {
				printf("false");
			}
			break;
		case HY_NIL:
			printf("nil");
			break;
		}

		// Separate arguments with a single space
		if (i < arity - 1) {
			printf(" ");
		}
	}

	return hy_nil();
}


// Prints an arbitrary number of arguments to the standard output, appending a
// trailing newline after all of them.
HyValue io_println(HyVM *vm, HyArgs *args) {
	HyValue result = io_print(vm, args);
	printf("\n");
	return result;
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
	hy_fn_new(package, "println", -1, io_println);
	hy_fn_new(package, "print_err", -1, io_print_err);
}
