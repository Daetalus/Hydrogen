
//
//  Configuration
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vec.h>

#include "config.h"
#include "help.h"


// Read from stdin until the terminating character is reached.
static char * read_stdin(void) {
	Vec(char) contents;
	vec_new(contents, char, 4096);

	int ch = getchar();
	while (ch != EOF) {
		vec_add(contents);
		vec_last(contents) = (char) ch;
		ch = getchar();
	}

	// Add the NULL terminator
	vec_add(contents);
	vec_last(contents) = '\0';
	return &vec_at(contents, 0);
}


// Parse an option.
static void config_opt(Config *config, char *opt) {
	if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0) {
		// Help
		print_help();
		config->type = EXEC_EXIT;
	} else if (strcmp(opt, "--version") == 0 || strcmp(opt, "-v") == 0) {
		// Version
		print_version();
		config->type = EXEC_EXIT;
	} else if (strcmp(opt, "--jinfo") == 0) {
		// Show JIT info
		config->show_jit_info = true;
	} else if (strcmp(opt, "--joff") == 0) {
		// Disable JIT
		config->enable_jit = false;
	} else if (strcmp(opt, "-b") == 0) {
		// Show bytecode
		config->show_bytecode = true;
	} else if (strcmp(opt, "--stdin") == 0) {
		// Read from stdin
		config->type = EXEC_RUN;
		config->input_type = INPUT_SOURCE;
	} else if (opt[0] != '-') {
		// Path to input
		config->type = EXEC_RUN;
		config->input_type = INPUT_FILE;
		config->input = opt;
	} else {
		// Invalid option
		printf("Unrecognised option `%s`\n", opt);
		print_usage();
		config->type = EXEC_EXIT;
	}
}


// Load configuration options from the given command line options.
Config config_new(int argc, char *argv[]) {
	Config config;
	config.enable_jit = true;
	config.show_jit_info = false;
	config.show_bytecode = false;
	config.type = EXEC_REPL;
	config.input_type = INPUT_FILE;
	config.input = NULL;

	// Parse options
	for (int i = 1; i < argc && config.type != EXEC_EXIT; i++) {
		config_opt(&config, argv[i]);
	}

	// Read the source from the standard input
	if (config.type != EXEC_EXIT && config.input_type == INPUT_SOURCE) {
		config.input = read_stdin();
	}

	return config;
}


// Free a configuration object.
void config_free(Config *config) {
	// Free the source code we read from the standard input
	if (config->input_type == INPUT_SOURCE) {
		free(config->input);
	}
}
