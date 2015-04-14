
//
//  Run a Benchmark
//


#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "../include/hydrogen.h"

// Color codes
#define NORMAL  "\x1B[0m"
#define BOLD    "\x1B[1m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"


int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: interpreter <path>\n");
		return 1;
	}

	// Open the source code file
	FILE *f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Failed to open file!\n");
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
