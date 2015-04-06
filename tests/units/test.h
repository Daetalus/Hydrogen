
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


#define START(name)                    \
	int test_##name(void) {            \
		const char *test_name = #name; \
		if (1)


#define END()                                                  \
		printf(GREEN BOLD "`%s` passed.\n" NORMAL, test_name); \
		return 0;                                              \
	}


#define RUN(name)                                              \
	printf(BLUE BOLD "Starting test " NORMAL "`%s`\n", #name); \
	if (test_##name()) {                                       \
		failed++;                                              \
	}


#define MAIN(name)                                                     \
	int main(int argc, char *argv[]) {                                 \
		int failed = 0;                                                \
		printf(LINE "\n" BLUE BOLD "Testing %s...\n\n" NORMAL, #name); \
		if (1)


#define MAIN_END()                                                            \
		if (failed == 0) {                                                    \
			printf(GREEN BOLD "\nAll tests passed.\n");                       \
			printf(GREEN BOLD "You are awesome!\n" NORMAL LINE "\n");         \
			return 0;                                                         \
		} else if (failed == 1) {                                             \
			printf(RED BOLD "\n1 test failed.\n" NORMAL LINE "\n");           \
			return 1;                                                         \
		} else {                                                              \
			printf(RED BOLD "\n%d tests failed.\n" NORMAL LINE "\n", failed); \
			return 1;                                                         \
		}                                                                     \
	}


#define PRINT_ASSERTION_FAILED()                                                      \
	fprintf(stderr, BOLD RED "Assertion failed " NORMAL "in test `%s` on line %d:\n", \
		test_name, __LINE__);


#define ASSERT_EQ(a, b)                            \
	if ((a) != (b)) {                              \
		PRINT_ASSERTION_FAILED()                   \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1;                                  \
	}


#define ASSERT_STR_EQ(a, b)                        \
	if (strcmp((a), (b)) != 0) {                   \
		PRINT_ASSERTION_FAILED()                   \
		fprintf(stderr, "    %s == %s\n", #a, #b); \
		return 1;                                  \
	}


#define ASSERT_STRN_EQ(a, b, length)                                     \
	if (strncmp((a), (b), (length)) != 0) {                              \
		PRINT_ASSERTION_FAILED()                                         \
		fprintf(stderr, "    %s == %s, length: %d\n", #a, #b, (length)); \
		return 1;                                                        \
	}



//
//  Bytecode Testing
//


#include "../../src/vm.h"
#include "../../src/compiler.h"
#include "../../src/expression.h"
#include "../../src/debug.h"
#include "../../src/bytecode.h"
#include "../../src/lexer.h"
#include "../../src/operators.h"


#define EXPRESSION(content)                   \
	VirtualMachine vm = vm_new((content));    \
	Function fn;                              \
	fn.name = "test";                         \
	fn.name_length = 4;                       \
	fn.argument_count = 0;                    \
	Bytecode *bytecode = &fn.bytecode;        \
	bytecode_new(bytecode, 10);               \
	Compiler compiler;                        \
	compiler.vm = &vm;                        \
	compiler.fn = &fn;                        \
	compiler.local_count = 0;                 \
	compiler.scope_depth = 0;                 \
	compiler.string_literal_count = 0;        \
	expression(&compiler, TOKEN_END_OF_FILE); \
	uint8_t *cursor = &bytecode->instructions[0];


#define COMPILER(code)                    \
	VirtualMachine vm = vm_new((code));   \
	Function fn;                          \
	fn.name = "test";                     \
	fn.name_length = 4;                   \
	fn.argument_count = 0;                \
	Bytecode *bytecode = &fn.bytecode;    \
	bytecode_new(bytecode, 10);           \
	compile(&vm, &fn, TOKEN_END_OF_FILE); \
	uint8_t *cursor = &bytecode->instructions[0];


#define READ_BYTE() \
		(cursor++, (uint8_t) (*(cursor - 0)))


#define READ_2_BYTES() \
	(cursor += 2, ((uint16_t) *(cursor - 0) << (1 << 3)) | *(cursor - 1))


#define READ_4_BYTES()                           \
	(cursor += 4,                                \
		((uint32_t) *(cursor - 0) << (3 << 3)) | \
		((uint32_t) *(cursor - 1) << (2 << 3)) | \
		((uint32_t) *(cursor - 2) << (1 << 3)) | \
		(uint32_t) (*(cursor - 3)))


#define READ_8_BYTES()                           \
	(cursor += 8,                                \
		((uint64_t) *(cursor - 0) << (7 << 3)) | \
		((uint64_t) *(cursor - 1) << (6 << 3)) | \
		((uint64_t) *(cursor - 2) << (5 << 3)) | \
		((uint64_t) *(cursor - 3) << (4 << 3)) | \
		((uint64_t) *(cursor - 4) << (3 << 3)) | \
		((uint64_t) *(cursor - 5) << (2 << 3)) | \
		((uint64_t) *(cursor - 6) << (1 << 3)) | \
		((uint64_t) *(cursor - 7)))


#define ASSERT_INSTRUCTION(instruction) \
	ASSERT_EQ(READ_BYTE(), instruction);


#define ASSERT_NUMBER_PUSH(number)            \
	ASSERT_EQ(READ_BYTE(), CODE_PUSH_NUMBER); \
	ASSERT_EQ(as_number(READ_8_BYTES()), number);


#define ASSERT_OPERATOR_CALL(fn)              \
	ASSERT_EQ(READ_BYTE(), CODE_CALL_NATIVE); \
	ASSERT_EQ(AS_FN(READ_8_BYTES()), &(fn));


#define ASSERT_STORE(slot)              \
	ASSERT_EQ(READ_BYTE(), CODE_STORE); \
	ASSERT_EQ(READ_2_BYTES(), slot);
