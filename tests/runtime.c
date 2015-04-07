
//
//  Runtime Tests
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/hydrogen.h"


// The maximum number of characters that can be outputted by
// the standard output stream on any of the tests.
#define MAX_STDOUT 1024


char *files[] = {
	"basic.hy"
};


int main(int argc, char *argv[]) {
	for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
		// Get the full path of the file
		size_t path_length = (strlen(files[i]) + 17) * sizeof(char);
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

		// Redirect stdout to a string
		// char contents[MAX_STDOUT];
		// memset(&contents[0], 0, MAX_STDOUT * sizeof(char));
		// freopen("/dev/null", "a", stdout);
		// setbuf(stdout, contents);

		// Execute the source code
		hydrogen_run(source);
	}

	return 0;
}
