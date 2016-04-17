
//
//  Errors
//

#include <setjmp.h>
#include <string.h>
#include <stdio.h>

#include "err.h"
#include "vm.h"
#include "lexer.h"


// The maximum length of a single added part to an error message's description.
#define MIN_DESCRIPTION_CAPACITY 256

// The number of spaces to have in a tab character.
#define TABS_TO_SPACES 2

// Will evaluate to the minimum of two values.
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


// Create a new error object without any associated information.
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
	vec_new(parent.description, char, MIN_DESCRIPTION_CAPACITY);
	return parent;
}


// Release resources allocated by an error object.
void hy_err_free(HyError *err) {
	if (err == NULL) {
		return;
	}

	free(err->description);
	free(err->file);
	free(err->code);
	free(err);
}


// Print a string to an error's description using a `va_list` as arguments to
// the format string.
void err_print_va(Error *err, char *fmt, va_list args) {
	// Ensure the description has at least 128 characters of extra capacity
	uint32_t limit = vec_len(err->description) + MIN_DESCRIPTION_CAPACITY;
	vec_resize(err->description, limit, vec_capacity(err->description) * 2);

	// Print the message
	char *description = &vec_at(err->description, vec_len(err->description));
	int size = vsnprintf(description, MIN_DESCRIPTION_CAPACITY, fmt, args);
	vec_len(err->description) += size;
}


// Print a string to an error's description.
void err_print(Error *err, char *fmt, ...) {
	// Just use the vararg print function
	va_list args;
	va_start(args, fmt);
	err_print_va(err, fmt, args);
	va_end(args);
}


// Return the length of the line starting at `cursor`.
static uint32_t err_line_length(char *cursor) {
	char *start = cursor;
	while (*cursor != '\0' && *cursor != '\n' && *cursor != '\r') {
		cursor++;
	}
	return cursor - start;
}


// Print a token to an error's description, surrounded in grave accents.
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
		uint32_t length = MIN(err_line_length(token->start), token->length);
		err_print(err, "`%.*s`", length, token->start);
		break;
	}

		// End of file
	case TOKEN_EOF:
		err_print(err, "end of file");
		break;

		// Unrecognised token
	case TOKEN_UNRECOGNISED:
		err_print(err, "<unrecognised>");
		break;

		// Everything else
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


// Return the line number the character at `cursor` is on.
static uint32_t err_line_number(char *cursor, char *start) {
	uint32_t line = 1;
	while (cursor >= start) {
		// Treat \r\n as a single newline
		if (cursor > start && *cursor == '\r' && *(cursor - 1) == '\n') {
			cursor--;
		}

		// Check for a new line
		if (*cursor == '\n' || *cursor == '\r') {
			line++;
		}

		cursor--;
	}
	return line;
}


// Return the column number for the character at `cursor`.
static uint32_t err_column_number(char *cursor, char *start) {
	uint32_t column = 0;
	while (cursor >= start && *cursor != '\n' && *cursor != '\r') {
		// Treat a tab as multiple spaces
		if (*cursor == '\t') {
			column += TABS_TO_SPACES;
		} else {
			column++;
		}

		cursor--;
	}
	return column;
}


// Return a pointer to the start of the line before the character at `cursor`.
// Since this function searches backwards, we need a pointer to the start of the
// source code so we know when to stop if `cursor` is on the first line.
static char * err_line_start(char *cursor, char *start) {
	while (cursor >= start && *cursor != '\n' && *cursor != '\r') {
		cursor--;
	}
	return cursor + 1;
}


// Return a line of source code from a pointer to the start of the line.
static char * err_line_of_code(char *start) {
	// Create a new string to hold the source code
	uint32_t length = err_line_length(start);
	char *line = malloc(length + 1);

	// Copy it from the string
	strncpy(line, start, length);
	line[length] = '\0';
	return line;
}


// Associate a token with the error.
void err_token(Error *err, Token *token) {
	HyError *native = err->native;
	Source *src = &vec_at(err->state->sources, token->source);

	// File path
	if (src->file != NULL) {
		// Copy the file name across from the token
		native->file = malloc(strlen(src->file) + 1);
		strcpy(native->file, src->file);
	}

	// Line of source code
	char *line_start = err_line_start(token->start, src->contents);
	native->code = err_line_of_code(line_start);

	// Length of the token
	if (token->type == TOKEN_EOF || token->type == TOKEN_UNRECOGNISED) {
		// Give end of file and unrecognised tokens a length of 1
		native->length = 1;
	} else {
		// Ensure the token doesn't extend past the end of the line
		native->length = MIN(token->length, err_line_length(token->start));
	}

	// Line and column numbers
	native->line = err_line_number(token->start, src->contents);
	native->column = err_column_number(token->start, src->contents);
}


// Associate a file with an error object.
void err_file(Error *err, char *file) {
	// Copy the provided string into our own memory
	err->native->file = malloc(strlen(file) + 1);
	strcpy(err->native->file, file);
}


// Construct the native error object from the parent error, freeing resources
// allocated by the parent at the same time.
HyError * err_make(Error *err) {
	HyError *native = err->native;

	// Allocate memory for the native error's description string
	uint32_t length = vec_len(err->description);
	native->description = malloc(length + 1);

	// Copy across the description
	strncpy(native->description, &vec_at(err->description, 0), length);
	native->description[length] = '\0';

	// Free resources allocated by the parent
	vec_free(err->description);
	return native;
}


// Stop execution of the current code and jump back to the error guard. Free
// any resources allocated during error construction.
void err_trigger(Error *err) {
	HyState *state = err->state;

	// Set the error on the interpreter state
	if (err->native->description != NULL) {
		state->error = err->native;
	} else {
		state->error = err_make(err);
	}

	// Jump back to the error guard
	longjmp(state->error_jmp, 1);
}
