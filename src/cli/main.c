
//
//  Command Line Interface
//

#include <hydrogen.h>
#include <hylib.h>

#include <vec.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "config.h"
#include "help.h"

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS
#endif

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



//
//  Error Printing
//

// Prints a color code to the standard output if we are able to display colors.
static inline void print_color(char *color) {
	// Only print colors on non-windows machines
#ifndef WINDOWS
	// If we're outputting to a terminal
	if (isatty(fileno(stdout))) {
		printf("%s", color);
	}
#endif
}


// Returns the number of digits in a number.
static int digits(int number) {
	int count = 0;
	while (number > 0) {
		count++;
		number /= 10;
	}
	return count;
}


// Prints the path to an error.
static int print_path(char *path, uint32_t line, uint32_t column) {
	// Path
	uint32_t prefix = 0;
	if (path == NULL) {
		prefix += printf("<string>:");
	} else {
		prefix += printf("%s:", path);
	}

	// Line number
	if (line > 0) {
		prefix += printf("%d:", line);
	}

	// Column number
	if (column > 0) {
		prefix += printf("%d:", column);
	}

	return prefix;
}


// Prints the description part of an error.
static int print_description(HyError *err) {
	// Header
	int prefix = print_path(err->file, err->line, err->column);
	prefix += printf(" ");

	// Tag
	print_color(COLOR_RED COLOR_BOLD);
	printf("[Error] ");
	print_color(COLOR_NONE);

	// Description
	print_color(COLOR_WHITE COLOR_BOLD);
	printf("%s\n", err->description);
	print_color(COLOR_NONE);
	return prefix;
}


// Removes tabs from a line of code, replacing them with 4 spaces.
static char * code_remove_tabs(char *line, uint32_t *padding) {
	// Count the number of tabs
	*padding = 0;
	for (uint32_t i = 0; line[i] != '\0'; i++) {
		if (line[i] == '\t') {
			*padding += 3;
		}
	}

	// Allocate a new line
	char *tabless_line = malloc(strlen(line) + *padding + 1);
	uint32_t length = 0;

	// Copy across to the new line
	for (uint32_t i = 0; line[i] != '\0'; i++) {
		if (line[i] == '\t') {
			for (uint32_t j = 0; j < 4; j++) {
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
static void print_code(HyError *err, int prefix) {
	if (err->line == 0 || err->column == 0) {
		// No code associated with the error
		return;
	}

	// Header
	int code_prefix = print_path(err->file, err->line, 0);

	// Padding
	for (int i = 0; i < prefix - code_prefix; i++) {
		printf(" ");
	}

	// Convert the line into one without tabs
	uint32_t tab_padding;
	char *tabless_line = code_remove_tabs(err->line_contents, &tab_padding);

	// Code
	print_color(COLOR_WHITE);
	printf("%s\n", tabless_line);
	free(tabless_line);

	// Underline padding
	for (uint32_t i = 0; i < prefix + err->column + tab_padding - 1; i++) {
		printf(" ");
	}

	// Underline
	printf("^");
	for (int32_t i = 0; i < err->length - 1; i++) {
		printf("~");
	}

	print_color(COLOR_NONE);
	printf("\n");
}


// Prints the type of a stack trace element.
static void print_stack_trace_type(HyStackTraceType type) {
	switch (type) {
	case HY_TRACE_FUNCTION:
		printf("function");
		break;
	case HY_TRACE_METHOD:
		printf("method");
		break;
	case HY_TRACE_PACKAGE:
		printf("package");
		break;
	case HY_TRACE_ANONYMOUS_PACKAGE:
		printf("anonymous package");
		break;
	}
}


// Prints an element in a stack trace.
static void print_stack_trace_element(HyStackTrace trace, int longest_path,
		int longest_line_number) {
	// Header
	printf("%*s:%-*d ", longest_path, trace.file, longest_line_number,
		trace.line);

	// Type
	print_color(COLOR_WHITE);
	printf("in ");
	print_stack_trace_type(trace.type);

	// Name
	if (trace.name != NULL) {
		printf(" `%s`", trace.name);
	}

	// Newline
	printf("\n");
	print_color(COLOR_NONE);
}


// Prints a stack trace.
static void print_stack_trace(HyStackTrace *trace, uint32_t count) {
	// Find the longest file path and line number
	int longest_path = 0;
	int longest_line_number = 0;
	for (uint32_t i = 0; i < count; i++) {
		// File path
		int length = strlen(trace[i].file);
		if (length > longest_path) {
			longest_path = length;
		}

		// Line number
		length = digits(trace[i].line);
		if (length > longest_line_number) {
			longest_line_number = length;
		}
	}

	// Tag padding
	for (int i = 0; i < longest_path + longest_line_number + 1; i++) {
		printf(" ");
	}

	// Tag
	print_color(COLOR_CYAN COLOR_BOLD);
	printf("[Stack Trace]\n");
	print_color(COLOR_NONE);

	// Each element in the stack trace
	for (uint32_t i = 0; i < count; i++) {
		print_stack_trace_element(trace[i], longest_path, longest_line_number);
	}
}


// Prints an error to the standard output. Returns the exit code to use.
static int print_err(HyError *err) {
	if (err == NULL) {
		return EXIT_SUCCESS;
	}

	int prefix = print_description(err);
	print_code(err, prefix);

	// Stack trace
	if (err->stack_trace != NULL) {
		printf("\n");
		print_stack_trace(err->stack_trace, err->stack_trace_length);
	}

	hy_err_free(err);
	return EXIT_FAILURE;
}



//
//  REPL
//

// Read from the standard input.
static char * repl_input(void) {
	// Print initial prompt
	printf("> ");

	// Create an input string
	Vec(char) input;
	vec_new(input, char, 512);

	// Read characters
	int ch = getchar();
	while (ch != '\n' && ch != '\r') {
		// Exit program if we reach end of file
		if (ch == EOF) {
			return NULL;
		} else if (ch <= CHAR_MAX) {
			// Otherwise add the character to the input
			vec_add(input);
			vec_last(input) = ch;
		}

		// Get the next character
		ch = getchar();
	}

	// Add a NULL terminator
	vec_add(input);
	vec_last(input) = '\0';
	return &vec_at(input, 0);
}


// Run the REPL.
static void repl(Config *config) {
	// Print version information
	print_version();

	// Create a new interpreter state and package
	// TODO: Add exit() native function to package for clean exit
	HyState *state = hy_new();
	hy_add_libs(state);
	HyPackage pkg = hy_add_pkg(state, NULL);

	while (true) {
		// Read input
		char *input = repl_input();
		if (input == NULL) {
			// Exit the program
			break;
		}

		// Execute input
		if (config->show_bytecode) {
			print_err(hy_print_bytecode_string(state, pkg, input));
		} else {
			print_err(hy_pkg_run_string(state, pkg, input));
		}
		free(input);
	}

	// Release resources
	hy_free(state);
}



//
//  Execution
//

// Print the bytecode of some input specified by the configuration.
static int print_bytecode(Config *config) {
	// Create a new interpreter state
	HyState *state = hy_new();

	// Add the standard library
	hy_add_libs(state);

	// Find the package name
	char *name = NULL;
	if (config->input_type == INPUT_FILE) {
		name = hy_pkg_name(config->input);
	}

	// Create a new package
	HyPackage pkg = hy_add_pkg(state, name);

	// Depending on the type of the input
	char *input = config->input;
	int exit_code = EXIT_SUCCESS;
	switch (config->input_type) {
	case INPUT_SOURCE:
		exit_code = print_err(hy_print_bytecode_string(state, pkg, input));
		break;
	case INPUT_FILE:
		exit_code = print_err(hy_print_bytecode_file(state, pkg, input));
		break;
	}

	// Release resources
	hy_free(state);
	return exit_code;
}


// Run some input specified by the configuration.
static int run(Config *config) {
	// Create the interpreter state
	HyState *state = hy_new();
	hy_add_libs(state);

	// Execute source code depending on the input type
	int exit_code = EXIT_SUCCESS;
	switch (config->input_type) {
	case INPUT_SOURCE:
		exit_code = print_err(hy_run_string(state, config->input));
		break;
	case INPUT_FILE:
		exit_code = print_err(hy_run_file(state, config->input));
		break;
	}

	hy_free(state);
	return exit_code;
}



//
//  Main
//

// Main entry point.
int main(int argc, char *argv[]) {
	// Parse options
	Config config = config_new(argc, argv);
	if (config.type == EXEC_EXIT) {
		// Help information was displayed, so exit
		return EXIT_SUCCESS;
	}

	// Start a REPL or run some code
	int exit_code = EXIT_SUCCESS;
	if (config.type == EXEC_REPL) {
		// Start REPL
		repl(&config);
	} else if (config.type == EXEC_RUN) {
		// Run code
		if (config.show_bytecode) {
			// Print bytecode
			exit_code = print_bytecode(&config);
		} else {
			// Actually execute the code
			exit_code = run(&config);
		}
	}

	// Free resources
	config_free(&config);
    return exit_code;
}
