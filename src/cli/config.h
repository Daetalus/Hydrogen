
//
//  Configuration
//

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>


// Which stage of execution to reach before exiting.
typedef enum {
	STAGE_NORMAL,
	STAGE_BYTECODE,
	STAGE_REPL,
	STAGE_EXIT,
} Stage;


// The type of input we're giving to Hydrogen.
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

	// Which stage of execution to reach before stopping.
	Stage stage;

	// Input data (either source code or a path to a file).
	InputType input_type;
	char *input;
} Config;


// Load configuration options from the given command line options.
Config config_new(int argc, char *argv[]);

// Free a configuration object.
void config_free(Config *config);

#endif
