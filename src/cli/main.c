
//
//  Command Line Interface
//

#include <hydrogen.h>
#include <hystdlib.h>

#include <stdlib.h>
#include <stdio.h>

#include "config.h"


// Pretty prints an error.
void print_err(HyError *err) {
	printf("Error: %s\n", err->description);
	printf("Line: %d\n", err->line);
}


// Main entry point.
int main(int argc, char *argv[]) {
	// Parse options
	Config config = config_new(argc, argv);
	if (config.stage == STAGE_EXIT) {
		// Help or version information was displayed and we don't want to do
		// anything else
		return EXIT_SUCCESS;
	}

	// Create a VM
	HyVM *vm = hy_new();
	hy_add_stdlib(vm);

	int result = EXIT_SUCCESS;
	if (config.stage == STAGE_NORMAL) {
		// Run the input
		HyError *err;
		if (config.input_type == INPUT_FILE) {
			err = hy_run_file(vm, config.input);
		} else {
			err = hy_run(vm, config.input);
		}

		// Check for an error
		if (err != NULL) {
			print_err(err);
			hy_err_free(err);
			result = EXIT_FAILURE;
		}
	} else if (config.stage == STAGE_BYTECODE) {
		// Print bytecode
		printf("Bytecodebytecodebytecodesomemorebytecode\n");
	} else if (config.stage == STAGE_REPL) {
		// Start a REPL
		printf("Sorry, REPL isn't implemented yet :(\n");
	}

	// Free resources
	hy_free(vm);
	config_free(&config);

    return result;
}
