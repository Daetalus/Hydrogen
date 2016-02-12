
//
//  Command Line Interface
//

#include <hydrogen.h>
#include <hystdlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

// The maximum length of an input line of code for the REPL.
#define MAX_REPL_INPUT_SIZE 2047
#define MAX_REPL_INPUT_SIZE_STR "2047"



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


// Prints the description part of an error.
static int print_description(HyError *err) {
	// Header
	int prefix = 0;
	if (err->line != -1 && err->column != -1) {
		// Check if the error was triggered in a string or file
		if (err->file == NULL) {
			prefix = printf("<string>:%d:%d ", err->line, err->column);
		} else {
			prefix = printf("%s:%d:%d ", err->file, err->line, err->column);
		}
	}

	// Tag
	print_color(COLOR_RED);
	print_color(COLOR_BOLD);
	printf("[Error] ");
	print_color(COLOR_NONE);

	// Description
	print_color(COLOR_WHITE);
	print_color(COLOR_BOLD);
	printf("%s\n", err->description);
	print_color(COLOR_NONE);
	return prefix;
}


// Prints the line of source code and an underline underneath the part causing
// the error.
static void print_code(HyError *err, int description_prefix) {
	if (err->line == -1 || err->column == -1) {
		// No code associated with the error
		return;
	}

	// Header
	int code_prefix = printf("%s:%d", err->file, err->line);

	// Padding
	for (int i = 0; i < description_prefix - code_prefix + 1; i++) {
		printf(" ");
	}

	// Code
	print_color(COLOR_WHITE);
	printf("%s\n", err->line_contents);

	// Underline padding
	for (int32_t i = 0; i < description_prefix + err->column; i++) {
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


// Returns the number of digits in a number.
static int digits(int number) {
	int count = 0;
	while (number > 0) {
		count++;
		number /= 10;
	}
	return count;
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
	print_color(COLOR_CYAN);
	print_color(COLOR_BOLD);
	printf("[Stack Trace]\n");
	print_color(COLOR_NONE);

	// Each element in the stack trace
	for (uint32_t i = 0; i < count; i++) {
		print_stack_trace_element(trace[i], longest_path, longest_line_number);
	}
}


// Prints an error to the standard output.
static void print_err(HyError *err) {
	if (err == NULL) {
		return;
	}

	int prefix = print_description(err);
	print_code(err, prefix);

	// Stack trace
	if (err->stack_trace != NULL) {
		printf("\n");
		print_stack_trace(err->stack_trace, err->stack_trace_length);
	}

	hy_err_free(err);
}



//
//  REPL
//

// Read a line of input from the standard input in a REPL loop.
static char * repl_read_input(void) {
	printf("> ");
	char *input = malloc(MAX_REPL_INPUT_SIZE + 1);
	scanf("%" MAX_REPL_INPUT_SIZE_STR "s", input);
	return input;
}


// Run the REPL.
static void repl(Config *config) {
	// Print version information
	print_version();

	// Create a new interpreter state and package
	// TODO: Add exit() native function to package for clean exit
	HyState *state = hy_new();
	hy_add_stdlib(state);
	HyPackage pkg = hy_add_pkg(state, NULL);

	// REPL loop
	while (true) {
		char *input = repl_read_input();
		if (config->show_bytecode) {
			print_err(hy_print_bytecode_string(state, pkg, input));
		} else {
			print_err(hy_pkg_run_string(state, pkg, input));
		}
	}

	// Release resources
	hy_free(state);
}



//
//  Execution
//

// Print the bytecode of some input specified by the configuration.
static void print_bytecode(Config *config) {
	// Create a new interpreter state
	HyState *state = hy_new();

	// Add the standard library
	hy_add_stdlib(state);

	// Find the package name
	char *name = NULL;
	if (config->input_type == INPUT_FILE) {
		name = hy_pkg_name(config->input);
	}

	// Create a new package
	HyPackage pkg = hy_add_pkg(state, name);

	// Depending on the type of the input
	switch (config->input_type) {
	case INPUT_SOURCE:
		print_err(hy_print_bytecode_string(state, pkg, config->input));
		break;
	case INPUT_FILE:
		print_err(hy_print_bytecode_file(state, pkg, config->input));
		break;
	}

	// Release resources
	hy_free(state);
}


// Run some input specified by the configuration.
static void run(Config *config) {
	// Create the interpreter state
	HyState *state = hy_new();
	hy_add_stdlib(state);

	// Execute source code depending on the input type
	switch (config->input_type) {
	case INPUT_SOURCE:
		print_err(hy_run_string(state, config->input));
		break;
	case INPUT_FILE:
		print_err(hy_run_file(state, config->input));
		break;
	}

	hy_free(state);
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

	if (config.type == EXEC_REPL) {
		// Start a REPL
		repl(&config);
	} else if (config.type == EXEC_RUN) {
		if (config.show_bytecode) {
			print_bytecode(&config);
		} else {
			run(&config);
		}
	}

	// Free resources
	config_free(&config);
    return EXIT_SUCCESS;
}
