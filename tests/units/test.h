
//
//  Testing Macros
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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

#define LINE \
	"--------------------------------------------------------------------------------"


#define START(name) \
	int test_##name(void) { \
		const char *test_name = #name; \
		if (1)

#define END() \
		printf(GREEN BOLD "`%s` passed.\n" NORMAL, test_name); \
		return 0; \
	}

#define RUN(name) \
	result = test_##name(); \
	if (result != 0 && has_failed == 0) { \
		has_failed = result; \
	}

#define MAIN(name) \
	int main(int argc, char *argv[]) { \
		int result; \
		int has_failed = 0; \
		printf(LINE "\n" BLUE BOLD "Testing %s...\n\n" NORMAL, #name); \
		if (1)

#define MAIN_END() \
		if (has_failed == 0) { \
			printf(GREEN BOLD "\nAll tests passed.\n" NORMAL LINE "\n"); \
		} else { \
			printf(RED BOLD "\nSome tests failed.\n" NORMAL LINE "\n"); \
		} \
		return has_failed; \
	}

#define ASSERT_EQ(a, b) \
	if ((a) != (b)) { \
		fprintf(stderr, BOLD RED "Assertion failed " NORMAL "in test `%s`:\n", test_name); \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1; \
	}

#define ASSERT_STR_EQ(a, b) \
	if (strcmp((a), (b)) != 0) { \
		fprintf(stderr, BOLD RED "Assertion failed" NORMAL "in test `%s`:", test_name); \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1; \
	}

#define ASSERT_STRN_EQ(a, b, length) \
	if (strncmp((a), (b), (length)) != 0) { \
		fprintf(stderr, BOLD RED "Assertion failed" NORMAL "in test `%s`:", test_name); \
		fprintf(stderr, "    %s == %s, length: %s\n", #a, #b, #length); \
		return 1; \
	}

// Handles imprecision in floating point math
#define ASSERT_DOUBLE_EQ(a, b) \
	if ((a) - (b) > -0.00000001 && (a) - (b) < 0.00000001) { \
		fprintf(stderr, BOLD RED "Assertion failed " NORMAL "in test `%s`:\n", test_name); \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1; \
	}
