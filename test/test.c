
//
//  Test Library
//

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdarg.h>

#include "test.h"


// Terminal colors.
#define COLOR_NONE    "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"
#define COLOR_BOLD    "\x1B[1m"


// A list of test cases to run.
static Vec(UnitTest) test_cases = vec_null();

// The error guard to use for test cases.
jmp_buf guard;


// Triggers an error.
void test_trigger_error(AssertInfo info, char *message, ...) {
	va_list args;
	va_start(args, message);
	printf(COLOR_RED "Failed\n");
	printf("    Assertion failed!\n" COLOR_NONE);
	printf("    ");
	vprintf(message, args);
	printf("\n");
	va_end(args);

	// Print file and line
	printf("      in file %s\n", info.file);
	printf("      on line %d\n", info.line);
	printf("      where %s\n", info.condition);

	// Trigger the error by calling longjmp
	longjmp(guard, 1);
}


// Ensures the test cases function is initialised.
static void test_init(void) {
	if (vec_is_null(test_cases)) {
		vec_new(test_cases, UnitTest, 4);
	}
}


// Add a test case that should either pass or fail.
void test_add(char *name, UnitTestFn fn, bool should_pass) {
	test_init();
	vec_inc(test_cases);
	UnitTest *test_case = &vec_last(test_cases);
	test_case->fn = fn;
	test_case->name = malloc(strlen(name) + 1);
	test_case->should_pass = should_pass;
	strcpy(test_case->name, name);
}


// Add a passing unit test case.
void test_pass(char *name, UnitTestFn test) {
	test_add(name, test, true);
}


// Add a failing unit test case.
void test_fail(char *name, UnitTestFn test) {
	test_add(name, test, false);
}


// Runs a single test case. Returns true if the case passed.
bool test_run_case(UnitTest *test_case, uint32_t longest) {
	// Print name of test case
	printf("%*s: ", longest, test_case->name);

	// Surround the test case in an error guard
	int result = setjmp(guard);
	if (result == 0) {
		// Run the test case
		test_case->fn();
	} else if (result == 1) {
		// If we reach here, then longjmp was called and we reach this else
		// statement
		return false;
	}

	// If we reached here, then longjmp wasn't called, hence the test case
	// passed
	printf(COLOR_GREEN "Passed\n" COLOR_NONE);
	return true;
}


// Run all tests. Returns the program exit code.
int test_run(int argc, char *argv[]) {
	// Unused at the moment
	(void) argc;
	(void) argv;

	// Check we actually have some test cases to run
	if (vec_is_null(test_cases) || vec_len(test_cases) == 0) {
		printf("No tests to run!\n");
		return EXIT_SUCCESS;
	}

	// Keep track of how many test cases passed
	uint32_t passed = 0;

	// Find the length of the longest test case name
	uint32_t longest = 0;
	for (uint32_t i = 0; i < vec_len(test_cases); i++) {
		uint32_t length = strlen(vec_at(test_cases, i).name);
		if (length > longest) {
			longest = length;
		}
	}

	// Iterate over each test case
	for (uint32_t i = 0; i < vec_len(test_cases); i++) {
		if (test_run_case(&vec_at(test_cases, i), longest)) {
			// The test case passed
			passed++;
		}
	}

	// Print out how many passed
	if (passed == vec_len(test_cases)) {
		printf(COLOR_GREEN "All tests passed" COLOR_NONE " (%d of %d)\n",
			passed, vec_len(test_cases));
		return EXIT_SUCCESS;
	} else {
		printf(COLOR_RED "%d of %d passed\n" COLOR_NONE,
			passed, vec_len(test_cases));
		return EXIT_FAILURE;
	}
}
