
//
//  REPL
//

#include "repl.h"

#include <hydrogen.h>
#include <hylib.h>

#include <stdio.h>
#include <limits.h>

#include <vec.h>

#include "help.h"
#include "err.h"


// The type of the history array.
typedef Vec(char *) History;


// Read from the standard input.
static char * repl_input(History *history) {
	// Print initial prompt
	printf("> ");

	// Create an input string
	Vec(char) input;
	vec_new(input, char, 512);

	// Position in history
	uint32_t history_position = NOT_FOUND;

	// Read characters
	int ch = getchar();
	while (ch != '\n' && ch != '\r') {
		// Exit program if we reach end of file
		if (ch == EOF) {
			return NULL;
		} else if (ch == '\033') {
			// Skip the next value (it's useless)
			getchar();

			// Escape code, might be an arrow key
			ch = getchar();
			if (ch == 'A' && history_position > 0) {
				// Up arrow

			} else if (ch == 'B') {

			}
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
void repl(Config *config) {
	// Print version information
	print_version();

	// Create a new interpreter state and package
	// TODO: Add exit() native function to package for clean exit
	HyState *state = hy_new();
	hy_add_libs(state);
	HyPackage pkg = hy_add_pkg(state, NULL);

	// Save our input history in an array
	History history;
	vec_new(history, char *, 64);

	while (true) {
		// Read input
		char *input = repl_input(&history);
		if (input == NULL) {
			// Exit the program
			break;
		}

		// Execute input
		HyError *err;
		if (config->show_bytecode) {
			err = hy_print_bytecode_string(state, pkg, input);
		} else {
			err = hy_pkg_run_string(state, pkg, input);
		}
		free(input);

		// Print the error if necessary
		if (err != NULL) {
			print_err(err);
			hy_err_free(err);
		}
	}

	// Release resources
	hy_free(state);
}
