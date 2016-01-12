
//
//  Error
//

#include <hydrogen.h>
#include <stdio.h>
#include <string.h>

#include "error.h"


// The maximum length of an error description string.
#define MAX_DESCRIPTION_LENGTH 512

// The number of characters of an unrecognised token to print.
#define UNRECOGNISED_TOKEN_LENGTH 8


// Copies a string into a new, heap allocated one, as long as `source` isn't
// NULL.
static char * str_copy(char *source) {
	if (source != NULL) {
		char *result = malloc(strlen(source) + 1);
		strcpy(result, source);
		return result;
	}
	return NULL;
}


// Allocate the error on the VM, copying across token information if `token` is
// not NULL.
static void err_init(VirtualMachine *vm, Token *token) {
	if (vm->err != NULL) {
		// Once an error is triggered, we should've long jumped back to the
		// error guard, which shouldn't trigger this function again until after
		// the error has been reset. Something somewhere went wrong... oh well
		hy_err_free(vm->err);
	}

	HyError *err = vm->err;
	err = malloc(sizeof(HyError));
	err->description = malloc(MAX_DESCRIPTION_LENGTH);
	err->package = NULL;
	err->file = NULL;

	if (token != NULL) {
		// Copy across line information
		err->line = token->line;
		err->column = token->column;

		// Copy across the file and package name into a heap allocated string,
		// since the lifetime of the error can outlive that of the VM
		err->file = str_copy(token->file);
		err->package = str_copy(token->package);
	} else {
		err->line = -1;
		err->column = -1;
	}
}


// Creates a new error. Doesn't trigger the jump back to the error guard.
void err_new(VirtualMachine *vm, char *fmt, ...) {
	err_init(vm, NULL);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(vm->err->description, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);
}


// Frees an error.
void hy_err_free(HyError *err) {
	// Nothing happens if we give `free` a NULL pointer
	free(err->description);
	free(err->package);
	free(err->file);
	free(err);
}


// Exits back to where the error guard is placed.
void err_jump(VirtualMachine *vm) {
	longjmp(vm->error_jump, 1);
}


// Triggers a fatal error.
void err_fatal(VirtualMachine *vm, char *fmt, ...) {
	err_init(vm, NULL);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(vm->err->description, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);

	// Jump back to error guard (terminate compilation)
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


// Writes a value to a string safely (avoiding buffer overflow), returning the
// remaining space left in the string.
size_t str_write(char **string, size_t capacity, char *value, size_t length) {
	if (length > capacity) {
		// No more room in the string
		return 0;
	}

	strncpy(*string, value, length);
	*string = &((*string)[length]);
	return capacity - length;
}


// Prints a token into a string, returning the remaining capacity of the string.
size_t print_token(char **string, size_t capacity, Token *token) {
	if (token->length > 0) {
		// We can copy across the token's text straight from the source code
		return str_write(string, capacity, token->start, token->length);
	} else if (token->type == TOKEN_UNRECOGNISED) {
		// Attempt to print the first 8 characters of the token
		// Check we're not already at the end of the file
		if (*token->start == '\0') {
			// Can't print any of the token
			return str_write(string, capacity, "<unrecognised token>", 20);
		}

		// Print the first few characters
		size_t length = 0;
		while (token->start[length] != '\0' &&
				length <= UNRECOGNISED_TOKEN_LENGTH) {
			length++;
		}

		// Write the first few characters, then `...`
		capacity = str_write(string, capacity, token->start, length);
		return str_write(string, capacity, "...", 3);
	} else if (token->type == TOKEN_EOF) {
		return str_write(string, capacity, "end of file", 11);
	} else {
		return str_write(string, capacity, "<invalid token>", 23);
	}
}


// Triggers an unexpected token error.
void err_unexpected(VirtualMachine *vm, Token *token, char *fmt, ...) {
	err_init(vm, token);

	// Save a pointer to the most recent location in the description we can
	// write to
	char *desc = vm->err->description;
	size_t capacity = MAX_DESCRIPTION_LENGTH;

	// Given description text
	va_list args;
	va_start(args, fmt);
	size_t length = vsnprintf(desc, MAX_DESCRIPTION_LENGTH, fmt, args);
	va_end(args);

	desc = &desc[length];
	capacity -= length;

	// Token
	capacity = str_write(&desc, capacity, ", found `", 9);
	capacity = print_token(&desc, capacity, token);
	sprintf(desc, "`");

	// Long jump back to error handler
	err_jump(vm);
}
