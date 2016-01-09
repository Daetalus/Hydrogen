
//
//  IO Library
//

#include "io.h"


// Prints a value to the given file stream.
void io_print_value(FILE *stream, HyValue value) {
	switch (hy_type(value)) {
	case HY_NUMBER:
		fprintf(stream, "%.15g", hy_expect_number(value));
		break;
	case HY_STRING:
		fprintf(stream, "%s", hy_expect_string(value));
		break;
	case HY_STRUCT:
		// TODO
		fprintf(stream, "struct");
		break;
	case HY_FN:
		// TODO
		fprintf(stream, "fn");
		break;
	case HY_BOOLEAN:
		if (hy_to_bool(value)) {
			fprintf(stream, "true");
		} else {
			fprintf(stream, "false");
		}
		break;
	case HY_NIL:
		fprintf(stream, "nil");
		break;
	}
}


// Print a value to the given stream.
HyValue io_fprint(FILE *stream, HyVM *vm, HyArgs *args) {
	uint32_t arity = hy_args_count(args);
	for (uint32_t i = 0; i < arity; i++) {
		HyValue arg = hy_arg(args, i);
		io_print_value(stream, arg);

		// Separate arguments with a single space
		if (i < arity - 1) {
			fprintf(stream, " ");
		}
	}

	return hy_nil();
}


// Print a value to the given stream with a trailing newline.
HyValue io_fprintln(FILE *stream, HyVM *vm, HyArgs *args) {
	HyValue result = io_fprint(stream, vm, args);
	fprintf(stream, "\n");
	return result;
}


// Prints an arbitrary number of arguments to the standard output, without a
// trailing newline.
HyValue io_print(HyVM *vm, HyArgs *args) {
	return io_fprint(stdout, vm, args);
}


// Prints an arbitrary number of arguments to the standard output, appending a
// trailing newline after all of them.
HyValue io_println(HyVM *vm, HyArgs *args) {
	return io_fprintln(stdout, vm, args);
}


// Register the IO library.
void hy_add_io(HyVM *vm) {
	HyNativePackage *package = hy_package_new(vm, "io");
	hy_fn_new(package, "print", -1, io_print);
	hy_fn_new(package, "println", -1, io_println);
}
