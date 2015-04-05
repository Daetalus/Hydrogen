
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
	printf(BLUE BOLD "Starting test " NORMAL "`%s`\n", #name); \
	if (test_##name()) { \
		failed++; \
	}

#define MAIN(name) \
	int main(int argc, char *argv[]) { \
		int result; \
		int failed = 0; \
		printf(LINE "\n" BLUE BOLD "Testing %s...\n\n" NORMAL, #name); \
		if (1)

#define MAIN_END() \
		if (failed == 0) { \
			printf(GREEN BOLD "\nAll tests passed.\n"); \
			printf(GREEN BOLD "You are awesome!\n" NORMAL LINE "\n"); \
		} else if (failed == 1) { \
			printf(RED BOLD "\n1 test failed.\n" NORMAL LINE "\n"); \
		} else { \
			printf(RED BOLD "\n%d tests failed.\n" NORMAL LINE "\n", failed); \
		} \
		return failed; \
	}

#define PRINT_ASSERTION_FAILED() \
	fprintf(stderr, BOLD RED "Assertion failed " NORMAL "in test `%s` on line %d:\n", \
		test_name, __LINE__);

#define ASSERT_EQ(a, b) \
	if ((a) != (b)) { \
		PRINT_ASSERTION_FAILED() \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1; \
	}

#define ASSERT_STR_EQ(a, b) \
	if (strcmp((a), (b)) != 0) { \
		PRINT_ASSERTION_FAILED() \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1; \
	}

#define ASSERT_STRN_EQ(a, b, length) \
	if (strncmp((a), (b), (length)) != 0) { \
		PRINT_ASSERTION_FAILED() \
		fprintf(stderr, "    %s == %s, length: %d\n", #a, #b, (length)); \
		return 1; \
	}
