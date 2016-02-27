
//
//  Error Printing
//

#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <defines.h>

#ifndef WINDOWS
#include <unistd.h>
#endif


// Color codes
#define COLOR_NONE    "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"
#define COLOR_BOLD    "\x1B[1m"

// Number of spaces per tab.
#define TABS_TO_SPACES 2


// Prints a color code to the standard output if we are able to display colors.
static inline void print_color(char *color) {
	// Only print colors on non-windows machines
#ifndef WINDOWS
	// If we're outputting to a terminal
	if (isatty(fileno(stdout))) {
		fputs(color, stderr);
	}
#endif
}


// Prints an error's file path, line number, and column number.
static int print_path(char *path, uint32_t line, uint32_t column) {
	// Path
	uint32_t length = 0;
	if (path == NULL) {
		length += fputs("<string>:", stderr);
	} else {
		length += fputs(path, stderr);
		length += fputc(':', stderr);
	}

	// Line number
	if (line > 0) {
		length += fprintf(stderr, "%d:", line);
	}

	// Column number
	if (column > 0) {
		length += fprintf(stderr, "%d:", column);
	}

	return length;
}


// Prints the error tag.
static void print_tag(void) {
	print_color(COLOR_RED COLOR_BOLD);
	fputs("[Error] ", stderr);
	print_color(COLOR_NONE);
}


// Prints the description part of an error.
static int print_description(HyError *err) {
	int align = 0;

	// Path
	align += print_path(err->file, err->line, err->column);
	align += fputc(' ', stderr);

	// Tag
	print_tag();

	// Description
	print_color(COLOR_WHITE COLOR_BOLD);
	fputs(err->description, stderr);
	fputc('\n', stderr);
	print_color(COLOR_NONE);
	return align;
}


// Removes tabs from a line of code, replacing them with 4 spaces.
static char * replace_tabs(char *line, uint32_t *padding) {
	// Count the number of tabs
	*padding = 0;
	for (uint32_t i = 0; line[i] != '\0'; i++) {
		if (line[i] == '\t') {
			// Since we're not actually removing the tab character (just
			// replacing it), we only need to add TABS_TO_SPACES - 1
			*padding += TABS_TO_SPACES - 1;
		}
	}

	// Allocate a new line
	char *tabless_line = malloc(strlen(line) + (*padding) + 1);
	uint32_t length = 0;

	// Copy across the old line of code
	for (uint32_t i = 0; line[i] != '\0'; i++) {
		if (line[i] == '\t') {
			for (uint32_t j = 0; j < TABS_TO_SPACES; j++) {
				tabless_line[length++] = ' ';
			}
		} else {
			tabless_line[length++] = line[i];
		}
	}

	tabless_line[length] = '\0';
	return tabless_line;
}


// Prints the line of source code and an underline underneath the part causing
// the error.
static void print_code(HyError *err, int align) {
	// File path and line number
	int length = print_path(err->file, err->line, 0);

	// Padding
	for (int i = 0; i < align - length; i++) {
		fputc(' ', stderr);
	}

	// Replaces spaces on the line of code
	uint32_t tab_padding;
	char *tabless_line = replace_tabs(err->line_contents, &tab_padding);

	// Code
	print_color(COLOR_WHITE);
	fprintf(stderr, "%s\n", tabless_line);
	free(tabless_line);

	// Underline padding
	for (uint32_t i = 0; i < align + err->column + tab_padding - 1; i++) {
		fputc(' ', stderr);
	}

	// Underline
	fputc('^', stderr);
	for (int32_t i = 0; i < err->length - 1; i++) {
		fputc('~', stderr);
	}

	print_color(COLOR_NONE);
	fputc('\n', stderr);
}


// Prints an error message to the standard error output.
void print_err(HyError *err) {
	// Description
	int align = print_description(err);

	// Line of source code
	if (err->line_contents != NULL) {
		print_code(err, align);
	}
}
