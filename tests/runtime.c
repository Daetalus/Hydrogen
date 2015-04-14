
//
//  Runtime Tests
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/hydrogen.h"
#include "../src/vm.h"
#include "../src/error.h"


// The maximum number of characters that can be outputted by
// the standard output stream on any of the tests.
#define MAX_STDOUT 1024


char *files[] = {
	"variables/1.hy",
	"variables/2.hy",
	"variables/3.hy",
	"variables/4.hy",
	"variables/5.hy",
	"variables/6.hy",

	"if/1.hy",
	"if/2.hy",
	"if/3.hy",
	"if/4.hy",
	"if/5.hy",
	"if/6.hy",
	"if/7.hy",
	"if/8.hy",
	"if/9.hy",
	"if/10.hy",
	"if/11.hy",
	"if/12.hy",
	"if/13.hy",

	"while/1.hy",
	"while/2.hy",
	"while/3.hy",
	"while/4.hy",
	"while/5.hy",
	"while/6.hy",
	"while/7.hy",
	"while/8.hy",
	"while/9.hy",
};


int main(int argc, char *argv[]) {
	for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
		printf(BLUE BOLD "Starting test %s...\n" NORMAL, files[i]);

		// Get the full path of the file
		size_t path_length = (strlen(files[i]) + 18) * sizeof(char);
		char path[path_length];
		strcpy(path, "../tests/runtime/");
		strcpy(&path[17], files[i]);

		// Open the source code file
		FILE *f = fopen(path, "r");
		if (f == NULL) {
			fprintf(stderr, "Failed to open file!\n");
			return 1;
		}

		// Get the size of the file
		fseek(f, 0, SEEK_END);
		size_t size = ftell(f);
		fseek(f, 0, SEEK_SET);

		// Read the contents
		char source[size];
		fread(source, size, 1, f);
		fclose(f);
		source[size] = '\0';

		// Execute the source code
		hydrogen_run(source);

		printf(GREEN BOLD "Test succeeded!\n" NORMAL);
	}

	return 0;
}
