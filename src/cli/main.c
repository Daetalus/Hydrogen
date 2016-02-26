
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
#include "repl.h"
#include "err.h"


// Print the bytecode of some input specified by the configuration.
static int bytecode(Config *config) {
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
	HyError *err;
	if (config->input_type == INPUT_STDIN) {
		err = hy_print_bytecode_string(state, pkg, config->input);
	} else {
		err = hy_print_bytecode_file(state, pkg, config->input);
	}

	// Print the error if needed
	if (err != NULL) {
		print_err(err);
		hy_err_free(err);
	}

	// Release resources
	hy_free(state);
	return (err == NULL) ? 0 : 1;
}


// Run some input specified by the configuration.
static int run(Config *config) {
	// Create the interpreter state
	HyState *state = hy_new();
	hy_add_libs(state);

	// Depending on the type of the input
	HyError *err;
	if (config->input_type == INPUT_STDIN) {
		err = hy_run_string(state, config->input);
	} else {
		err = hy_run_file(state, config->input);
	}

	// Print the error if needed
	if (err != NULL) {
		print_err(err);
		hy_err_free(err);
	}

	// Release resources
	hy_free(state);
	return (err == NULL) ? 0 : 1;
}


// Main entry point.
int main(int argc, char *argv[]) {
	// Parse options
	Config config = config_new(argc, argv);
	int exit_code = EXIT_SUCCESS;

	// Depending on what the config says to do
	switch (config.type) {
	case EXEC_RUN:
		// Run code
		exit_code = config.show_bytecode ? bytecode(&config) : run(&config);
		break;

	case EXEC_REPL:
		// Start REPL
		repl(&config);
		break;

	case EXEC_VERSION:
		// Print version information
		print_version();
		break;

	case EXEC_HELP:
		// Print help information
		print_help();
		break;

	case EXEC_USAGE:
		// Print usage information
		print_usage();
		break;
	}

	// Free resources
	config_free(&config);
    return exit_code;
}
