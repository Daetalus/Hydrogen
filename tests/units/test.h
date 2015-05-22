
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
#include "../../src/lib/io.h"


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

#define LINE                                   \
	"----------------------------------------" \
	"----------------------------------------"


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


#define START_MAIN(name)                                               \
	int main(int argc, char *argv[]) {                                 \
		int failed = 0;                                                \
		printf(LINE "\n" BLUE BOLD "Testing %s...\n\n" NORMAL, #name); \
		if (1)


#define END_MAIN()                                                            \
		if (failed == 0) {                                                    \
			printf(GREEN BOLD "All tests passed.\n");                         \
			printf(GREEN BOLD "You are awesome!\n" NORMAL LINE "\n");         \
			return 0;                                                         \
		} else if (failed == 1) {                                             \
			printf(RED BOLD "1 test failed.\n" NORMAL LINE "\n");             \
			return 1;                                                         \
		} else {                                                              \
			printf(RED BOLD "%d tests failed.\n" NORMAL LINE "\n", failed);   \
			return 1;                                                         \
		}                                                                     \
	}


#define PRINT_ASSERTION_FAILED()                        \
	fprintf(stderr, BOLD RED "Assertion failed " NORMAL \
		"in test `%s` on line %d:\n",                   \
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

#define EXPRESSION(content)                                  \
	VirtualMachine vm = vm_new((content));                   \
	Function fn;                                             \
	fn.arity = 0;                                            \
	fn.captured_upvalue_count = 0;                           \
	fn.defined_upvalue_count = 0;                            \
	fn.bytecode = bytecode_new(64);                          \
	Bytecode *bytecode = &fn.bytecode;                       \
	Compiler compiler;                                       \
	compiler.vm = &vm;                                       \
	compiler.parent = NULL;                                  \
	compiler.fn = &fn;                                       \
	compiler.local_count = 0;                                \
	compiler.scope_depth = 0;                                \
	compiler.loop_count = 0;                                 \
	Expression expression = expression_new(&compiler, NULL); \
	expression_compile(&expression);                         \
	uint8_t *ip = &bytecode->instructions[0];


#define COMPILER(code) \
	VM(code);          \
	USE_FUNCTION(0);


#define VM(code)                        \
	VirtualMachine vm = vm_new((code)); \
	vm_attach_standard_library(&vm);    \
	vm_compile(&vm);                    \
	Bytecode *bytecode;                 \
	uint8_t *ip;


#define USE_FUNCTION(slot)                     \
	bytecode = &vm.functions[(slot)].bytecode; \
	ip = &bytecode->instructions[0];


#define ASSERT_INSTRUCTION(instruction) \
	ASSERT_EQ(READ_BYTE(), instruction);


#define ASSERT_PUSH_NUMBER(number)        \
	ASSERT_INSTRUCTION(CODE_PUSH_NUMBER); \
	ASSERT_EQ(value_to_number(READ_8_BYTES()), number);


#define ASSERT_PUSH_STRING(index, str)    \
	ASSERT_INSTRUCTION(CODE_PUSH_STRING); \
	ASSERT_EQ(READ_2_BYTES(), index);     \
	ASSERT_STR_EQ(vm.literals[index]->contents, str);


#define ASSERT_PUSH_LOCAL(slot)          \
	ASSERT_INSTRUCTION(CODE_PUSH_LOCAL); \
	ASSERT_EQ(READ_2_BYTES(), slot);


#define ASSERT_PUSH_NATIVE(index)         \
	ASSERT_INSTRUCTION(CODE_PUSH_NATIVE); \
	ASSERT_EQ(READ_2_BYTES(), index);


#define ASSERT_PUSH_FUNCTION(index)         \
	ASSERT_INSTRUCTION(CODE_PUSH_FUNCTION); \
	ASSERT_EQ(READ_2_BYTES(), index);


#define ASSERT_PUSH_UPVALUE(index)         \
	ASSERT_INSTRUCTION(CODE_PUSH_UPVALUE); \
	ASSERT_EQ(READ_2_BYTES(), index);


#define ASSERT_PUSH_FIELD(name)              \
	ASSERT_INSTRUCTION(CODE_PUSH_FIELD);     \
	ASSERT_EQ(READ_2_BYTES(), strlen(name)); \
	ASSERT_STRN_EQ(value_to_ptr(READ_8_BYTES()), name, (int) strlen(name));


#define ASSERT_UPVALUE_CLOSE(index)         \
	ASSERT_INSTRUCTION(CODE_CLOSE_UPVALUE); \
	ASSERT_EQ(READ_2_BYTES(), index);


#define ASSERT_STORE_LOCAL(slot)          \
	ASSERT_INSTRUCTION(CODE_STORE_LOCAL); \
	ASSERT_EQ(READ_2_BYTES(), slot);


#define ASSERT_STORE_FIELD(name)             \
	ASSERT_INSTRUCTION(CODE_STORE_FIELD);    \
	ASSERT_EQ(READ_2_BYTES(), strlen(name)); \
	ASSERT_STRN_EQ(value_to_ptr(READ_8_BYTES()), name, (int) strlen(name));


#define ASSERT_CALL(arity)         \
	ASSERT_INSTRUCTION(CODE_CALL); \
	ASSERT_EQ(READ_2_BYTES(), arity);


#define ASSERT_NATIVE_CALL(ptr)           \
	ASSERT_INSTRUCTION(CODE_CALL_NATIVE); \
	ASSERT_EQ(value_to_ptr(READ_8_BYTES()), ptr);


#define ASSERT_CONDITIONAL_JUMP(amount)   \
	ASSERT_INSTRUCTION(CODE_JUMP_IF_NOT); \
	ASSERT_EQ(READ_2_BYTES(), amount);


#define ASSERT_JUMP(amount)                \
	ASSERT_INSTRUCTION(CODE_JUMP_FORWARD); \
	ASSERT_EQ(READ_2_BYTES(), amount);


#define ASSERT_BACKWARDS_JUMP(amount)   \
	ASSERT_INSTRUCTION(CODE_JUMP_BACK); \
	ASSERT_EQ(READ_2_BYTES(), amount);


#define ASSERT_RETURN_NIL()            \
	ASSERT_INSTRUCTION(CODE_PUSH_NIL); \
	ASSERT_INSTRUCTION(CODE_RETURN);


#define ASSERT_INSTANTIATE_CLASS(index)         \
	ASSERT_INSTRUCTION(CODE_INSTANTIATE_CLASS); \
	ASSERT_EQ(READ_2_BYTES(), index);
