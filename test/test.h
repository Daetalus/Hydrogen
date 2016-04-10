
//
//  Test Library
//

#ifndef TEST_H
#define TEST_H

#include <vec.h>
#include <stdbool.h>


// The type of a unit test case function.
typedef void (* UnitTestFn)(void);


// Information for a test case.
typedef struct {
	// The function containing the test case code.
	UnitTestFn fn;

	// True if this test case should pass.
	bool should_pass;

	// The name of the test case.
	char *name;
} UnitTest;


// Add a passing unit test case.
void test_pass(char *name, UnitTestFn test);

// Add a failing unit test case.
void test_fail(char *name, UnitTestFn test);

// Run all tests. Returns the program exit code.
int test_run(int argc, char *argv[]);


// Information passed to an assertion.
typedef struct {
	// The line the assert occurred on.
	uint32_t line;

	// The path to the file the assert occurred in.
	char *file;

	// A stringified version of the condition.
	char *condition;
} AssertInfo;


// Triggers an error.
void test_trigger_error(AssertInfo info, char *message, ...);


// Ensures a condition is true.
#define _check(cond, ...)                      \
	if (!(cond)) {                             \
		AssertInfo info;                       \
		info.line = __LINE__;                  \
		info.file = __FILE__;                  \
		info.condition = #cond;                \
		test_trigger_error(info, __VA_ARGS__); \
	}                                          \

// Ensure two values are equal.
#define check(condition)     _check (condition, "Condition is false")
#define eq_int(left, right)  _check (left == right, "%d != %d", left, right)
#define eq_uint(left, right) _check (left == right, "%u != %u", left, right)
#define eq_ptr(left, right)  _check (left == right, "%p != %p", left, right)
#define eq_ch(left, right)   _check (left == right, "%c != %c", left, right)
#define eq_num(left, right)  _check (left == right, "%g != %g", left, right)

// Ensure two values are not equal.
#define neq_int(left, right)  _check (left != right, "%d == %d", left, right)
#define neq_uint(left, right) _check (left != right, "%u == %u", left, right)
#define neq_ptr(left, right)  _check (left != right, "%p == %p", left, right)
#define neq_ch(left, right)   _check (left != right, "%c == %c", left, right)
#define neq_num(left, right)  _check (left != right, "%g == %g", left, right)

// Ensure one value is less than another.
#define lt_int(left, right)  _check (left < right, "%d >= %d", left, right)
#define lt_uint(left, right) _check (left < right, "%u >= %u", left, right)
#define lt_num(left, right)  _check (left < right, "%g >= %g", left, right)

// Ensures two NULL terminated strings are equal.
#define eq_str(left, right) \
	_check(strcmp(left, right) == 0, "\"%s\" != \"%s\"", left, right)

// Ensures two strings are equal up to a certain length.
#define eq_strn(left, right, length) \
	_check(strncmp(left, right, length) == 0, "\"%.*s\" != \"%.*s\"", \
		length, left, length, right)

#endif
