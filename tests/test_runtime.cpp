
//
//  Runtime Tests
//

#include "test.h"


// The prefix added to all runtime test paths.
#define PATH_PREFIX "../tests/runtime"

// The maximum size of the output from a test case
#define MAX_OUTPUT 1024

// Defines a runtime test.
#define RUNTIME_TEST(group, name, path)                                         \
	TEST(Runtime ## group, name) {                                              \
		char *full_path = (char *) PATH_PREFIX "/" path;                        \
		char *line = read_first_line(full_path);                                \
		if (line == NULL) {                                                     \
			FAIL() << "Couldn't find runtime test `" << full_path << "`";       \
		}                                                                       \
		ASSERT_EXIT(death_test(full_path), ::testing::ExitedWithCode(0), line); \
		free(line);                                                             \
	}


// Reads the first line of a file, excluding the opening comment `// `.
char * read_first_line(char *path) {
	// Read first line of the file for the expected output of the test
	FILE *file = fopen(path, "r");
	if (file == NULL) {
		return NULL;
	}

	char *line = (char *) malloc(sizeof(char) * MAX_OUTPUT);
	size_t length = 0;

	// Skip the first 3 characters (the `// `)
	getc(file);
	getc(file);
	char last = getc(file);
	if (last == '\n' || last == '\r') {
		// No output expected, so return an empty string
		line[0] = '\0';
		return line;
	}

	// Read the rest of the line
	char ch = getc(file);
	while (length < MAX_OUTPUT - 1 && ch != '\n' && ch != '\r' &&
			ch != EOF) {
		line[length++] = ch;
		ch = getc(file);
	}
	line[length] = '\0';
	fclose(file);

	// Replace `|` with newlines
	for (size_t i = 0; i < length; i++) {
		if (line[i] == '|') {
			line[i] = '\n';
		}
	}

	return line;
}


// Runs a death test on the given file.
void death_test(char *path) {
	HyVM *vm = hy_new();
	hy_add_stdlib(vm);
	HyError *err = hy_run_file(vm, (char *) path);
	hy_free(vm);

	// Check for an error
	if (err != NULL) {
		fprintf(stderr, "Error: %s\n", err->description);
		hy_err_free(err);
		exit(1);
	} else {
		// Google Test will complain if we don't exit like this
		exit(0);
	}
}


// Assignment
RUNTIME_TEST(Assign, TopLevel, "assign/top_level.hy");
RUNTIME_TEST(Assign, Stack, "assign/stack.hy");

// Expressions
RUNTIME_TEST(Expr, Arithmetic, "expr/arithmetic.hy");
// RUNTIME_TEST(Expr, Concat, "expr/concat.hy");
RUNTIME_TEST(Expr, Comparison, "expr/comparison.hy");

// If statements
RUNTIME_TEST(If, If, "if/if.hy");
RUNTIME_TEST(If, IfElse, "if/if_else.hy");
RUNTIME_TEST(If, IfElseif, "if/if_elseif.hy");
RUNTIME_TEST(If, IfElseifElse, "if/if_elseif_else.hy");
RUNTIME_TEST(If, IfElseifs, "if/if_elseifs.hy");
RUNTIME_TEST(If, IfElseIfsElse, "if/if_elseifs_else.hy");
RUNTIME_TEST(If, Nested, "if/nested.hy");

// Functions
RUNTIME_TEST(Fn, Call, "fn/call.hy");
