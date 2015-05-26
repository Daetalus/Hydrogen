
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

	"functions/1.hy",
	"functions/2.hy",
	"functions/3.hy",
	"functions/4.hy",
	"functions/5.hy",
	"functions/6.hy",
	"functions/7.hy",
	"functions/8.hy",
	"functions/9.hy",
	"functions/10.hy",
	"functions/11.hy",
	"functions/12.hy",

	"closures/1.hy",
	"closures/2.hy",
	"closures/3.hy",
	"closures/4.hy",
	"closures/5.hy",
	"closures/6.hy",
	"closures/7.hy",
	"closures/8.hy",
	"closures/9.hy",
	"closures/10.hy",
	"closures/11.hy",
	"closures/12.hy",

	"function_expressions/1.hy",
	"function_expressions/2.hy",
	"function_expressions/3.hy",

	"structs/1.hy",
	"structs/2.hy",
	"structs/3.hy",

	"methods/1.hy",
	"methods/2.hy",
	"methods/3.hy",
	"methods/4.hy",
	"methods/5.hy",
	"methods/6.hy",
	"methods/7.hy",
	"methods/8.hy",
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
