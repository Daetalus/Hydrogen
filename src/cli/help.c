
//
//  Help
//

#include <stdio.h>

#include "help.h"

// The current version number.
#define VERSION "0.1.0"


// Print usage information.
void print_usage(void) {
	printf(
		"Usage:\n"
		"  hydrogen [options] [path to file]\n"
		"Run hydrogen -h for a list of options\n"
	);
}


// Print version information.
void print_version(void) {
	printf(
		"Hydrogen " VERSION "\n"
		"A toy tracing JIT compiled programming language.\n"
		"By Ben Anderson, 2016.\n"
	);
}


// Print full help information.
void print_help(void) {
	print_version();
	printf(
		"\n"
		"Usage:\n"
		"  hydrogen [options] [path to file]\n"
		"\n"
		"A REPL is run if no file is given.\n"
		"\n"
		"Options:\n"
		"  -b             Print the bytecode for a program\n"
		"  --stdin        Read from the standard input rather than a file\n"
		"  --joff         Disable JIT compilation\n"
		"  --jinfo        Show information about JIT compiled loops\n"
		"  --version, -v  Show Hydrogen's version number\n"
		"  --help, -h     Show this help text\n"
	);
}
