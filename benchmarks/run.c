
//
//  Run a Benchmark
//


#include <string.h>
#include <stdio.h>

#include "../include/hydrogen.h"


int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: run <benchmark file path>\n");
		return 0;
	}

	// Get the full path of the file
	size_t path_length = (strlen(argv[1]) + 15) * sizeof(char);
	char path[path_length];
	strcpy(path, "../benchmarks/");
	strcpy(&path[14], argv[1]);

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

	// Run the file
	hydrogen_run(source);

	return 0;
}
