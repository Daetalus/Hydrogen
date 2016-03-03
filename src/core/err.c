
//
//  Errors
//

#include <setjmp.h>
#include <string.h>
#include <stdio.h>

#include "err.h"
#include "vm.h"


// The maximum size of an error description
#define MAX_DESCRIPTION_SIZE 1023

// The maximum number of characters to print when printing an unrecognised
// token
#define MAX_UNRECOGNISED_CHARACTERS 25


// Creates a new error object without any associated details yet. The error can
// be constructed using the building functions below
HyError * err_new(void) {
	HyError *err = malloc(sizeof(HyError));

	// Allocate a description and place a NULL terminator at the start
	err->description = malloc(MAX_DESCRIPTION_SIZE + 1);
	err->description[0] = '\0';

	err->file = NULL;
	err->line_contents = NULL;
	err->line = 0;
	err->column = 0;
	err->length = 0;
	err->stack_trace = NULL;
	err->stack_trace_length = 0;
	return err;
}


// Creates a new failed to open file error
HyError * err_failed_to_open_file(char *path) {
	HyError *err = err_new();
	err->file = malloc(strlen(path) + 1);
	strcpy(err->file, path);
	err_print(err, "Failed to open file");
	return err;
}


// Release resources allocated by an error object
void hy_err_free(HyError *err) {
	free(err->description);
	free(err->file);
	free(err->line_contents);
	free(err->stack_trace);
}


// Prints a string to an error's description using a `va_list` as arguments to
// the format string
void err_print_varargs(HyError *err, char *fmt, va_list args) {
	// Calculate the length and remaining capacity of the description
	uint32_t length = strlen(err->description);
	uint32_t capacity = MAX_DESCRIPTION_SIZE - length;

	// Print the format string to the description
	vsnprintf(&err->description[length], capacity, fmt, args);
}


// Prints a string to an error's description
void err_print(HyError *err, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	err_print_varargs(err, fmt, args);
	va_end(args);
}


// Prints an unrecognised token to a description buffer
static void print_unrecognised(char *description, uint32_t capacity,
		Token *token) {
	// Print until the first whitespace character, or the 25th character
	char *cursor = token->start;
	while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' &&
			*cursor != '\n' && *cursor != '\r' &&
			cursor - token->start <= MAX_UNRECOGNISED_CHARACTERS) {
		cursor++;
	}
	uint32_t length = cursor - token->start;

	// Write to the description string
	if (length == 0) {
		// Couldn't print any characters
		snprintf(description, capacity, "unrecognised token");
	} else {
		snprintf(description, capacity, "unrecognised token `%.*s`", length,
			token->start);
	}
}


// Returns the length of the line starting at `cursor`
static uint32_t line_length(char *cursor) {
	uint32_t length = 0;
	while (*cursor != '\0' && *cursor != '\n' && *cursor != '\r') {
		length++;
		cursor++;
	}
	return length;
}


// Prints a token to an error's description, surrounded in grave accents
void err_print_token(HyError *err, Token *token) {
	// Calculate the length and remaining capacity of the description
	// Keep track of the capacity so we don't potentially cause a buffer
	// overflow
	uint32_t length = strlen(err->description);
	uint32_t capacity = MAX_DESCRIPTION_SIZE - length;

	// If the token has a length, then we can just print it directly from its
	// source
	char *description = &err->description[length];
	if (token->type == TOKEN_ELSE_IF) {
		// `else if` has the potential to be spread across multiple lines
		// because of the arbitrary amount of whitespace between the two words,
		// so print it separately from the rest of the tokens
		snprintf(description, capacity, "`else if`");
	} else if (token->type == TOKEN_STRING) {
		// Strings also have the potential to span multiple lines, so print
		// only the first line
		uint32_t line = line_length(token->start);
		uint32_t actual = line < token->length ? line : token->length;
		snprintf(description, capacity, "`%.*s`", actual, token->start);
	} else if (token->length > 0) {
		snprintf(description, capacity, "`%.*s`", token->length, token->start);
	} else if (token->type == TOKEN_EOF) {
		snprintf(description, capacity, "end of file");
	} else if (token->type == TOKEN_UNRECOGNISED) {
		print_unrecognised(description, capacity, token);
	} else {
		snprintf(description, capacity, "invalid token");
	}
}


// Returns a pointer to the start of the first line before the character at
// `cursor`
static char * line_start(char *cursor, char *source) {
	while (cursor >= source && *cursor != '\n' && *cursor != '\r') {
		cursor--;
	}
	return cursor + 1;
}


// Returns the line number the character at `cursor` is on
static uint32_t line_number(char *cursor, char *source) {
	uint32_t line = 1;
	while (cursor >= source) {
		if (*cursor == '\n' || *cursor == '\r') {
			// Treat \r\n as a single newline
			if (cursor > source && *cursor == '\n' && *(cursor - 1) == '\r') {
				cursor--;
			}
			line++;
		}
		cursor--;
	}
	return line;
}


// Returns the column number for character at `cursor`
static uint32_t column_number(char *cursor, char *source) {
	uint32_t column = 0;
	while (cursor >= source && *cursor != '\n' && *cursor != '\r') {
		cursor--;
		column++;
	}
	return column;
}


// Associate a token with the error
void err_attach_token(HyState *state, HyError *err, Token *token) {
	Source *src = &vec_at(state->sources, token->source);

	// Copy across the file path
	if (src->file != NULL) {
		err->file = malloc(strlen(src->file) + 1);
		strcpy(err->file, src->file);
	}

	// Copy the line contents
	char *start = line_start(token->start, src->contents);
	uint32_t length = line_length(start);
	err->line_contents = malloc(length + 1);
	strncpy(err->line_contents, start, length);
	err->line_contents[length] = '\0';

	// Ensure the length of the token doesn't extend past the end of the line
	uint32_t max_length = line_length(token->start);
	if (token->length > max_length) {
		err->length = max_length;
	} else {
		err->length = token->length;
	}

	// Copy across the line and column numbers
	err->line = line_number(token->start, src->contents);
	err->column = column_number(token->start, src->contents);
}


// Triggers a jump back to the error guard with the built error
void err_trigger(HyState *state, HyError *err) {
	if (state->error != NULL) {
		// This shouldn't happen, but free the previous error anyway
		hy_err_free(state->error);
	}

	state->error = err;
	longjmp(state->error_jmp, 1);
}
