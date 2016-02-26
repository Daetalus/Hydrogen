
//
//  Configuration
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vec.h>

#include "config.h"


// Read from stdin until the terminating character is reached.
static char * read_stdin(void) {
	Vec(char) contents;
	vec_new(contents, char, 4096);

	int ch = getchar();
	while (ch != EOF) {
		vec_inc(contents);
		vec_last(contents) = (char) ch;
		ch = getchar();
	}

	// Add the NULL terminator
	vec_inc(contents);
	vec_last(contents) = '\0';
	return &vec_at(contents, 0);
}


// Parse an option. Returns true if we are to continue parsing options.
static bool config_opt(Config *config, char *opt) {
	if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0) {
		// Help
		config->type = EXEC_HELP;
	} else if (strcmp(opt, "--version") == 0 || strcmp(opt, "-v") == 0) {
		// Version
		config->type = EXEC_VERSION;
	} else if (strcmp(opt, "--") == 0) {
		// Stop parsing options
		return false;
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
		config->input_type = INPUT_STDIN;
	} else if (opt[0] == '-') {
		// Invalid option
		fprintf(stderr, "Invalid option `%s`\n", opt);
		config->type = EXEC_USAGE;
		return false;
	} else {
		// Path to input
		config->type = EXEC_RUN;
		config->input_type = INPUT_FILE;
		config->input = opt;
	}

	// Keep parsing options
	return true;
}


// Load configuration options from the given command line options.
Config config_new(int argc, char *argv[]) {
	Config config;
	config.enable_jit = true;
	config.show_jit_info = false;
	config.show_bytecode = false;
	config.type = EXEC_REPL;
	config.input_type = INPUT_NONE;
	config.input = NULL;

	// Parse options
	for (int i = 1; i < argc; i++) {
		if (!config_opt(&config, argv[i])) {
			break;
		}
	}

	// Read the source from the standard input
	if (config.type == EXEC_RUN && config.input_type == INPUT_STDIN) {
		config.input = read_stdin();
	}

	return config;
}


// Free a configuration object.
void config_free(Config *config) {
	// Free the source code we read from the standard input
	if (config->type == EXEC_RUN && config->input_type == INPUT_STDIN) {
		free(config->input);
	}
}
