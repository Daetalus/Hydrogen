
//
//  Testing Macros
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../src/vm.h"
#include "../../src/compiler.h"
#include "../../src/expression.h"
#include "../../src/debug.h"
#include "../../src/bytecode.h"
#include "../../src/lexer.h"
#include "../../src/lib/operator.h"


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


#define EXPRESSION(content)                \
	VirtualMachine vm = vm_new((content)); \
	Function fn;                           \
	fn.name = "test";                      \
	fn.length = 4;                         \
	fn.arity = 0;                          \
	fn.bytecode = bytecode_new(64);        \
	Bytecode *bytecode = &fn.bytecode;     \
	Compiler compiler;                     \
	compiler.vm = &vm;                     \
	compiler.fn = &fn;                     \
	compiler.local_count = 0;              \
	compiler.scope_depth = 0;              \
	expression(&compiler, NULL);           \
	uint8_t *ip = &bytecode->instructions[0];


#define COMPILER(code)                    \
	VirtualMachine vm = vm_new((code));   \
	Function fn;                          \
	fn.name = "test";                     \
	fn.length = 4;                        \
	fn.arity = 0;                         \
	fn.bytecode = bytecode_new(64);       \
	Bytecode *bytecode = &fn.bytecode;    \
	compile(&vm, &fn, TOKEN_END_OF_FILE); \
	uint8_t *ip = &bytecode->instructions[0];


#define VM(code)                        \
	VirtualMachine vm = vm_new((code)); \
	vm_compile(&vm);                    \
	Bytecode *bytecode;                 \
	uint8_t *ip;


#define USE_FUNCTION(slot)                     \
	bytecode = &vm.functions[(slot)].bytecode; \
	ip = &bytecode->instructions[0];


#define ASSERT_INSTRUCTION(instruction) \
	ASSERT_EQ(READ_BYTE(), instruction);


#define ASSERT_NUMBER_PUSH(number)            \
	ASSERT_EQ(READ_BYTE(), CODE_PUSH_NUMBER); \
	ASSERT_EQ(value_to_number(READ_8_BYTES()), number);


#define ASSERT_STRING_PUSH(index, str)        \
	ASSERT_EQ(READ_BYTE(), CODE_PUSH_STRING); \
	ASSERT_EQ(READ_2_BYTES(), index);         \
	ASSERT_STR_EQ(vm.literals[index]->contents, str);


#define ASSERT_VARIABLE_PUSH(slot)           \
	ASSERT_EQ(READ_BYTE(), CODE_PUSH_LOCAL); \
	ASSERT_EQ(READ_2_BYTES(), slot);


#define ASSERT_NATIVE_CALL(fn)                \
	ASSERT_EQ(READ_BYTE(), CODE_CALL_NATIVE); \
	ASSERT_EQ(value_to_ptr(READ_8_BYTES()), &(fn));


#define ASSERT_CALL(slot)              \
	ASSERT_EQ(READ_BYTE(), CODE_CALL); \
	ASSERT_EQ(READ_2_BYTES(), slot);


#define ASSERT_STORE(slot)              \
	ASSERT_EQ(READ_BYTE(), CODE_STORE); \
	ASSERT_EQ(READ_2_BYTES(), slot);


#define ASSERT_CONDITIONAL_JUMP(amount)       \
	ASSERT_EQ(READ_BYTE(), CODE_JUMP_IF_NOT); \
	ASSERT_EQ(READ_2_BYTES(), amount);


#define ASSERT_JUMP(amount)                    \
	ASSERT_EQ(READ_BYTE(), CODE_JUMP_FORWARD); \
	ASSERT_EQ(READ_2_BYTES(), amount);


#define ASSERT_BACKWARDS_JUMP(amount)       \
	ASSERT_EQ(READ_BYTE(), CODE_JUMP_BACK); \
	ASSERT_EQ(READ_2_BYTES(), amount);

#define ASSERT_RETURN_NIL()            \
	ASSERT_INSTRUCTION(CODE_PUSH_NIL); \
	ASSERT_INSTRUCTION(CODE_RETURN);
