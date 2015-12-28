
//
//  Error
//

#include <stdio.h>
#include <string.h>

#include "error.h"


// The maximum number of characters the description of an error can be.
#define MAX_ERROR_LENGTH 1024


// Returns a new, custom error.
HyError err_new(Token *token, char *fmt, ...) {
	HyError err;
	err.line = token->line;
	err.column = token->column;
	err.package = NULL;
	err.file = NULL;
	err.description = malloc(sizeof(char) * MAX_ERROR_LENGTH);

	// Write to the description
	va_list args;
	va_start(args, fmt);
	vsnprintf(err.description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);

	return err;
}


// Returns an unexpected token error.
// TODO: Ensure we don't cause a buffer overflow from `fmt` being too long
HyError err_unexpected(Token *token, char *fmt, ...) {
	HyError err;
	err.line = token->line;
	err.column = token->column;
	err.package = NULL;
	err.file = NULL;

	// Create the description
	err.description = malloc(MAX_ERROR_LENGTH * sizeof(char));
	char *description = err.description;

	// `fmt` and arguments
	va_list args;
	va_start(args, fmt);
	int length = vsnprintf(description, MAX_ERROR_LENGTH, fmt, args);
	va_end(args);
	description = &description[length];

	// `, found ...`
	length = sprintf(description, ", found `");
	description = &description[length];

	// Token
	if (token->length == 0) {
		// If the token's unrecognised and we can print at least 1 character
		// of the unrecognised token
		if (token->type == TOKEN_UNRECOGNISED && *token->start != '\0') {
			// Extract up to the first 8 characters (so we don't cause a
			// buffer overflow)
			char *current = token->start;
			while (*current != '\0' && current - token->start <= 8) {
				current++;
			}

			size_t length = current - token->start;
			strncpy(description, token->start, length);
			description = &description[length];
		} else if (token->type == TOKEN_EOF) {
			strcpy(description, "end of file");
			description = &description[11];
		} else {
			// TODO: handle this better
		}
	} else {
		strncpy(description, token->start, token->length);
		description = &description[token->length];
	}

	// Final backtick
	sprintf(description, "`");
	return err;
}


// Frees an error.
void err_free(HyError *err) {
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
