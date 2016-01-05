
//
//  Error
//

#include <stdio.h>
#include <string.h>

#include "error.h"

// The maximum length of an error description string.
#define MAX_DESCRIPTION_LENGTH 512


// Allocate the error on the VM.
void err_init(VirtualMachine *vm, Token *token) {
	if (vm->err != NULL) {
		// Somehow this happened, because the longjmp should've prevented it
		// Oh well, just free it and allocate it again
		hy_err_free(vm->err);
	}

	vm->err = malloc(sizeof(HyError));
	vm->err->description = malloc(sizeof(char) * MAX_DESCRIPTION_LENGTH);

	if (token != NULL) {
		vm->err->line = token->line;
		vm->err->column = token->column;

		// Copy across the file into a heap allocated string, since we don't
		// know the lifetime of the one provided by the package
		if (token->file != NULL) {
			vm->err->file = malloc(sizeof(char) * strlen(token->file));
			strcpy(vm->err->file, token->file);
		} else {
			vm->err->file = NULL;
		}

		// Copy across the package name into a heap allocated string, similar to
		// the file name
		if (token->package != NULL) {
			vm->err->package = malloc(sizeof(char) * strlen(token->package));
			strcpy(vm->err->package, token->package);
		} else {
			vm->err->package = NULL;
		}
	}
}


// Exits back to where the error guard is placed.
void err_jump(VirtualMachine *vm) {
	longjmp(vm->error_jump, 1);
}


// Sets the error on the VM.
void err_new(VirtualMachine *vm, char *fmt, ...) {
	err_init(vm, NULL);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(vm->err->description, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);
}


// Triggers a fatal error on the Vm.
void err_fatal(VirtualMachine *vm, char *fmt, ...) {
	err_init(vm, NULL);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(vm->err->description, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);

	// Terminate
	err_jump(vm);
}


// Triggers a custom error.
void err_token(VirtualMachine *vm, Token *token, char *fmt, ...) {
	err_init(vm, token);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(vm->err->description, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);

	// Jump to the error handler
	err_jump(vm);
}


// Writes a value to a string safely.
size_t str_write(char **string, size_t capacity, char *value, size_t length) {
	if (length > capacity) {
		// No more room in the string
		return 0;
	}

	strncpy(*string, value, length);
	*string = &((*string)[length]);
	return capacity - length;
}


// Prints a token into a string.
size_t print_token(char **string, size_t capacity, Token *token) {
	if (token->length > 0) {
		return str_write(string, capacity, token->start, token->length);
	}

	if (token->type == TOKEN_UNRECOGNISED) {
		if (*token->start == '\0') {
			// Can't print any of the token
			return str_write(string, capacity, "unrecognised token", 18);
		}

		// Print up to the first 8 characters
		char *current = token->start;
		while (*current != '\0' && current - token->start <= 8) {
			current++;
		}

		size_t length = current - token->start;
		capacity = str_write(string, capacity, token->start, length);
		return str_write(string, capacity, "...", 3);
	} else if (token->type == TOKEN_EOF) {
		return str_write(string, capacity, "end of file", 11);
	} else {
		return str_write(string, capacity, "<unable to print token>", 23);
	}
}


// Triggers an unexpected token error.
void err_unexpected(VirtualMachine *vm, Token *token, char *fmt, ...) {
	err_init(vm, token);

	char *desc = vm->err->description;
	size_t capacity = MAX_DESCRIPTION_LENGTH;

	// `fmt` and arguments
	va_list args;
	va_start(args, fmt);
	size_t length = vsnprintf(desc, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);
	desc = &desc[length];
	capacity -= length;

	// Print token
	capacity = str_write(&desc, capacity, ", found `", 9);
	capacity = print_token(&desc, capacity, token);
	sprintf(desc, "`");

	// Long jump back to error handler
	err_jump(vm);
}


// Frees an error.
void hy_err_free(HyError *err) {
	if (err == NULL) {
		return;
	}

	if (err->description != NULL) {
		free(err->description);
	}
	if (err->package != NULL) {
		free(err->package);
	}
	if (err->file != NULL) {
		free(err->file);
	}
	free(err);
}
