
//
//  Command Line Interface
//

#include <hydrogen.h>
#include <hystdlib.h>

#include <stdio.h>
#include <stdlib.h>


// Returns the contents of a file as a heap allocated string.
char * file_contents(char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		return NULL;
	}

	// Get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);

	// Read its contents
	char *contents = malloc(sizeof(char) * (length + 1));
	fread(contents, sizeof(char), length, f);
	fclose(f);
	contents[length] = '\0';
	return contents;
}


int main(int argc, char *argv[]) {
	// Ensure we have an argument
	if (argc <= 1) {
		printf("Usage:\n    hydrogen <path to file>\n");
		return 1;
	}

	// Read the contents of the file
	char *path = argv[1];
	char *contents = file_contents(path);
	if (contents == NULL) {
		printf("Failed to open file `%s`\n", path);
		return 1;
	}

	// Run the source string
	HyVM *vm = hy_new();
	hy_add_stdlib(vm);
	HyError *err = hy_run(vm, contents);
	if (err != NULL) {
		printf("Error: %s\n", err->description);
		printf("Line: %d\n", err->line);
		printf("Column: %d\n", err->column);
		hy_err_free(err);
	}

	hy_free(vm);
	free(contents);
    return 0;
}
