
//
//  Testing Framework
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lexer.h>

#define NORMAL  "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"
#define BOLD    "\x1B[1m"


// Creates a new test case.
#define TEST(name) void test_ ## name (void)

// Defines the main function.
#define MAIN() int main(int argc, char *argv[])

// Runs a test case.
#define RUN(name)                                            \
	printf(BOLD BLUE "Running " WHITE #name "...\n" NORMAL); \
	test_ ## name ();                                        \
	printf(BOLD GREEN "Passed!\n" NORMAL);

// Prints an error message stating the two given values
// are not equal, and crashes the program.
#define ERROR(a, b, message)                                 \
	printf(BOLD RED "Assertion failed: " WHITE "line %d:\n"  \
		"    " #a " " message " " #b NORMAL "\n", __LINE__); \
	exit(1);

// Errors if two values are not equal to each other.
#define EQ(a, b)            \
	if ((a) != (b)) {       \
		ERROR(a, b, "!=");  \
	}

// Errors if two values are equal to each other.
#define NEQ(a, b)          \
	if ((a) == (b)) {      \
		ERROR(a, b, "=="); \
	}

// Errors if two strings are not equal to each other.
#define EQ_STR(a, b)             \
	if (strcmp((a), (b)) != 0) { \
		ERROR(a, b, "!=");       \
	}

// Errors if two strings are not equal to each other, only
// checking up to the given length.
#define EQ_STRN(a, b, length)               \
	if (strncmp((a), (b), (length)) != 0) { \
		ERROR(a, b, "!=");                  \
	}
