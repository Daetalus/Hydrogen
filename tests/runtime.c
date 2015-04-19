
//
//  Runtime Tests
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/hydrogen.h"
#include "../src/vm.h"
#include "../src/error.h"


char *files[] = {
	"assignment/1.hy",
	"assignment/2.hy",
	"assignment/3.hy",
	"assignment/4.hy",
	"assignment/5.hy",
	"assignment/6.hy",

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
	"while/10.hy",

	"loop/1.hy",
	"loop/2.hy",
	"loop/3.hy",

	"fn/1.hy",
	"fn/2.hy",
	"fn/3.hy",
	"fn/4.hy",
	"fn/5.hy",
	"fn/6.hy",
	"fn/7.hy",
	"fn/8.hy",
	"fn/9.hy",
	"fn/10.hy",
	"fn/11.hy",
	"fn/12.hy",

	"closure/1.hy",
	"closure/2.hy",
	"closure/3.hy",
	"closure/4.hy",
	"closure/5.hy",
	"closure/6.hy",
	"closure/7.hy",
	"closure/8.hy",
	"closure/9.hy",
	"closure/10.hy",
	"closure/11.hy",
	"closure/12.hy",
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

	printf(GREEN BOLD "\n\nAll tests passed!\n" NORMAL);
	return 0;
}
