
//
//  Command Line Interface
//

#include <hydrogen.h>

#include <stdio.h>
#include <stdlib.h>


// Returns the contents of a file as a heap allocated string.
char * read_file(char *path) {
	FILE *f = fopen(path, "r");

	// Get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	rewind(f);

	// Read its contents
	char *contents = malloc(sizeof(char) * (length + 1));
	fread(contents, sizeof(char), length, f);
	fclose(f);
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
	char *contents = read_file(path);

	// Run the source string
	HyVM *vm = hy_new();
	HyError *err = hy_run(vm, contents);
	if (err != NULL) {
		printf("Error: %s\n", err->description);
		hy_err_free(err);
	}

	hy_free(vm);
	free(contents);
    return 0;
}
