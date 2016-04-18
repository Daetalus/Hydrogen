
//
//  Standard Library
//

#include <hylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Register the entire standard library.
void hy_add_libs(HyState *state) {
	hy_add_io(state);
}



//
//  IO
//

// Print a value to the standard output, returning the number of characters
// printed.
static uint32_t io_print_value(HyValue value) {
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
	default:
		return printf("<unimplemented>");
	}
}


// Print a line to the standard output without a trailing newline.
static HyValue io_print(HyState *state, HyArgs *args) {
	uint32_t arity = hy_args_count(args);
	uint32_t length = 0;
	for (uint32_t i = 0; i < arity; i++) {
		length += io_print_value(hy_arg(args, i));

		// Add a space if this isn't the last argument
		if (i != arity - 1) {
			length += printf(" ");
		}
	}
	return hy_number((double) length);
}


// Print a line to the standard output with a trailing newline.
static HyValue io_println(HyState *state, HyArgs *args) {
	HyValue result = io_print(state, args);
	printf("\n");
	return result;
}


// Create a new IO stream.
static void * io_stream_new(HyState *state, HyArgs *args) {
	char *value = "hello!";
	char *mem = malloc(strlen(value) + 1);
	strcpy(mem, value);
	return mem;
}


// Free resources associated with an IO stream.
static void io_stream_free(HyState *state, void *data) {
	free(data);
}


// Print the contents of a stream.
static HyValue io_stream_print(HyState *state, void *data, HyArgs *args) {
	printf("Data: %s\n", (char *) data);
	return hy_nil();
}


// Register the IO library.
void hy_add_io(HyState *state) {
	HyPackage pkg = hy_add_pkg(state, "io");
	hy_add_fn(state, pkg, "print", -1, io_print);
	hy_add_fn(state, pkg, "println", -1, io_println);

	// Add a test native struct
	HyStruct stream = hy_add_struct(state, pkg, "Stream", io_stream_new, 0);
	hy_set_destructor(state, stream, io_stream_free);
	hy_add_method(state, stream, "print", 0, io_stream_print);
}
