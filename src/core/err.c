
//
//  Errors
//

#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "err.h"
#include "vm.h"
#include "debug.h"


// The maximum length that can be printed by a single print operation
#define MIN_CAPACITY 128

// The maximum number of characters to print for an unrecognised token
#define MAX_UNRECOGNISED_CHARACTERS 25

// The number of spaces to have in a tab character
#define TABS_TO_SPACES 2


// Creates a new error object without any associated details yet. The error can
// be constructed using the building functions below
Error err_new(HyState *state) {
	// Create the underlying error object
	HyError *err = malloc(sizeof(HyError));
	err->description = NULL;
	err->file = NULL;
	err->code = NULL;
	err->line = 0;
	err->column = 0;
	err->length = 0;

	Error parent;
	parent.native = err;
	parent.state = state;
	vec_new(parent.description, char, 128);
	return parent;
}


// Release resources allocated by an error object
void hy_err_free(HyError *err) {
	if (err == NULL) {
		return;
	}

	free(err->description);
	free(err->file);
	free(err->code);
	free(err);
}


// Prints a string to an error's description using a `va_list` as arguments to
// the format string
void err_print_va(Error *err, char *fmt, va_list args) {
	// Ensure the description has at least 128 characters of extra capacity
	uint32_t limit = vec_len(err->description) + MIN_CAPACITY;
	vec_resize(err->description, limit, vec_capacity(err->description) * 2);

	// Print the message
	char *description = &vec_at(err->description, vec_len(err->description));
	int size = vsnprintf(description, MIN_CAPACITY, fmt, args);
	ASSERT(size < MIN_CAPACITY);
	vec_len(err->description) += size;
}


// Prints a string to an error's description
void err_print(Error *err, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	err_print_va(err, fmt, args);
	va_end(args);
}


// Prints an unrecognised token to a description buffer
static void err_print_unrecognised(Error *err, Token *token) {
	// Print until the first whitespace character, or the 25th character
	char *cursor = token->start;
	while (*cursor != '\0' && !isspace(*cursor) &&
			cursor - token->start <= MAX_UNRECOGNISED_CHARACTERS) {
		cursor++;
	}

	// Write to the description string
	uint32_t length = cursor - token->start;
	if (length == 0) {
		// Couldn't print any characters
		err_print(err, "<unrecognised>");
	} else {
		// Print part of the unrecognised token
		err_print(err, "<unrecognised `%.*s`>", length, token->start);
	}
}


// Returns the length of the line starting at `cursor`
static uint32_t line_length(char *cursor) {
	char *start = cursor;
	while (*cursor != '\0' && *cursor != '\n' && *cursor != '\r') {
		cursor++;
	}
	return cursor - start;
}


// Prints a token to an error's description, surrounded in grave accents
void err_print_token(Error *err, Token *token) {
	switch (token->type) {
	case TOKEN_ELSE_IF:
		// `else if` has the potential to be spread across multiple lines
		// because of the arbitrary amount of whitespace between the two words,
		// so print it separately from the rest of the tokens
		err_print(err, "`else if`");
		break;

	case TOKEN_STRING: {
		// Strings also have the potential to span multiple lines, so print
		// only the first line
		uint32_t line_len = line_length(token->start);
		uint32_t length = line_len < token->length ? line_len : token->length;
		err_print(err, "`%.*s`", length, token->start);
		break;
	}

	case TOKEN_EOF:
		// Print `end of file`
		err_print(err, "end of file");
		break;

	case TOKEN_UNRECOGNISED:
		// Unrecognised token
		err_print_unrecognised(err, token);
		break;

	default:
		if (token->length > 0) {
			// Print the token straight from the source code
			err_print(err, "`%.*s`", token->length, token->start);
			break;
		}

		// This shouldn't happen
		err_print(err, "<invalid>");
		break;
	}
}


// Returns a pointer to the start of the first line before the character at
// `cursor`
static char * line_start(char *cursor, char *start) {
	while (cursor >= start && *cursor != '\n' && *cursor != '\r') {
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

			// Found a new line
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
		if (*cursor == '\t') {
			// Since we're treating tabs as multiple spaces, add some extra to
			// the column number to account for it
			column += TABS_TO_SPACES;
		} else {
			column++;
		}

		cursor--;
	}
	return column;
}


// Associates a line of source code with the error
void err_code(Error *err, Token *token) {
	HyError *native = err->native;
	Source *src = &vec_at(err->state->sources, token->source);

	// File path
	if (src->file != NULL) {
		native->file = malloc(strlen(src->file) + 1);
		strcpy(native->file, src->file);
	}

	// Line of source code
	char *start = line_start(token->start, src->contents);
	uint32_t length = line_length(start);
	native->code = malloc(length + 1);
	strncpy(native->code, start, length);
	native->code[length] = '\0';

	// TODO: Handle length of unrecognised tokens properly
	if (token->type == TOKEN_EOF) {
		// Give the end of file token a length of 1 so we actually point to it
		native->length = 1;
	} else {
		// Ensure the length of the token doesn't extend past the end of the
		// line
		uint32_t max_length = line_length(token->start);
		if (token->length > max_length) {
			native->length = max_length;
		} else {
			native->length = token->length;
		}
	}

	// Line and column numbers
	native->line = line_number(token->start, src->contents);
	native->column = column_number(token->start, src->contents);
}


// Associates a file with an error object
void err_file(Error *err, char *file) {
	err->native->file = malloc(strlen(file) + 1);
	strcpy(err->native->file, file);
}


// Constructs the native error object from the parent error, freeing resources
// allocated by the parent at the same time
HyError * err_make(Error *err) {
	// Copy across the description into the native error
	uint32_t length = vec_len(err->description);
	err->native->description = malloc(length + 1);
	strncpy(err->native->description, &vec_at(err->description, 0), length);
	err->native->description[length] = '\0';
	vec_free(err->description);
	return err->native;
}


// Stops execution of the current code and jumps back to the error guard. Frees
// any resources allocated by the error construction
void err_trigger(Error *err) {
	err->state->error = err_make(err);
	longjmp(err->state->error_jmp, 1);
}
