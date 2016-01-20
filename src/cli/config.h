
//
//  Configuration
//

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>


// The possible types of execution.
typedef enum {
	EXEC_RUN,
	EXEC_REPL,
	EXEC_EXIT,
} ExecutionType;


// The type of input we're given.
typedef enum {
	INPUT_SOURCE,
	INPUT_FILE,
} InputType;


// Configuration options specified on the command line.
typedef struct {
	// Whether to enable JIT compilation or not.
	bool enable_jit;

	// Whether to display information about JIT compiled loops during
	// execution or not.
	bool show_jit_info;

	// Whether to output bytecode or execute code.
	bool show_bytecode;

	// What type of execution is requested.
	ExecutionType type;

	// Input data (either source code or a path to a file).
	InputType input_type;
	char *input;
} Config;


// Load configuration from the given command line options.
Config config_new(int argc, char *argv[]);

// Free a configuration object.
void config_free(Config *config);

#endif
