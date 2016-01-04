
//
//  Test Case Includes
//

extern "C" {
#include <hydrogen.h>
#include <bytecode.h>
#include <debug.h>
#include <error.h>
#include <parser/lexer.h>
#include <parser/parser.h>
#include <parser/jmp.h>
#include <util.h>
#include <value.h>
#include <vm.h>
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>


// Color codes.
#define NORMAL  "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"
#define BOLD    "\x1B[1m"


// Asserts two strings are equal up to the given length (since this function is
// annoyingly missing from the Google test framework).
#define ASSERT_STREQN(first, second, length) { \
	char first_str[length + 1];                \
	char second_str[length + 1];               \
	strncpy(first_str, (first), length);       \
	strncpy(second_str, (second), length);     \
	first_str[length] = '\0';                  \
	second_str[length] = '\0';                 \
	ASSERT_STREQ(first_str, second_str);       \
}


// Creates a function with the given bytecode.
#define FUNCTION(...)                                                      \
	uint16_t bytecode[] = {__VA_ARGS__};                                   \
	int count = sizeof(bytecode) / sizeof(uint16_t);                       \
	Function fn;                                                           \
	fn.name = NULL;                                                        \
	fn.length = 0;                                                         \
	ARRAY_INIT(fn.bytecode, uint64_t, count / 4);                          \
	fn.package = NULL;                                                     \
	for (int i = 0; i < count; i += 4) {                                   \
		fn.bytecode[fn.bytecode_count++] = instr_new((Opcode) bytecode[i], \
			bytecode[i + 1], bytecode[i + 2], bytecode[i + 3]);            \
	}


// Creates a mock compiler with the given source code.
#define COMPILER(code)                                   \
	VirtualMachine *vm = hy_new();                       \
	Package *package = package_new(vm, NULL);            \
	package->source = (char *) (code);                   \
	if (setjmp(vm->error_jump) == 0) {                   \
		parse_package(vm, package);                      \
	} else {                                             \
		if (vm->err != NULL) {                           \
			printf("Error: %s\n", vm->err->description); \
			exit(1);                                     \
		}                                                \
	}                                                    \
	Function *fn = &vm->functions[package->main_fn];     \
	size_t index = 0;


// Frees a compiler.
#define COMPILER_FREE() hy_free(vm);


// Begins asserting instructions at the start of another function's bytecode.
#define FN(fn_index)                 \
	fn = &vm->functions[(fn_index)]; \
	index = 0;


// Asserts the next instruction's opcode and arguments are equal to the given
// values.
#define ASSERT_INSTR(opcode, arg1, arg2, arg3) {     \
	ASSERT_NE(index, fn->bytecode_count);            \
	uint64_t instruction = fn->bytecode[index++];    \
	ASSERT_EQ(instr_opcode(instruction), opcode);    \
	ASSERT_EQ(instr_argument(instruction, 1), arg1); \
	ASSERT_EQ(instr_argument(instruction, 2), arg2); \
	ASSERT_EQ(instr_argument(instruction, 3), arg3); \
}


// Asserts the next instruction is an empty return.
#define ASSERT_RET() ASSERT_INSTR(RET, 0, 0, 0)


// Asserts the next instruction is a jump, and that it will jump forward by
// `amount`.
#define ASSERT_JMP(amount) {                           \
	ASSERT_NE(index, fn->bytecode_count);              \
	uint64_t instruction = fn->bytecode[index++];      \
	ASSERT_EQ(instr_opcode(instruction), JMP);         \
	ASSERT_EQ(instr_argument(instruction, 1), amount); \
}


// Asserts the next instruction is a function call.
#define ASSERT_CALL(opcode, fn_index, arg_start, arity, return_slot) { \
	ASSERT_NE(index, fn->bytecode_count);                              \
	uint64_t instruction = fn->bytecode[index++];                      \
	ASSERT_EQ(instr_opcode(instruction), opcode);                      \
	ASSERT_EQ(instr_argument(instruction, 0), arity);                  \
	ASSERT_EQ(instr_argument(instruction, 1), fn_index);               \
	ASSERT_EQ(instr_argument(instruction, 2), arg_start);              \
	ASSERT_EQ(instr_argument(instruction, 3), return_slot);            \
}


// Main entry point for a test case.
int main(int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
