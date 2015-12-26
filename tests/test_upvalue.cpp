
//
//  Upvalue Tests
//

#include "test.h"


// Tests fetching an upvalue from a scope external to the function.
TEST(Upvalue, Get) {
	COMPILER(
		"let a = 3\n"
		"fn test() {\n"
		"	let b = a + 2\n"
		"}\n"
	);

	FN(0);
	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LF, 1, 1, 0);
	ASSERT_INSTR(UPVALUE_CLOSE, 0, 0, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 2);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests setting an upvalue from a scope external to the function.
TEST(Upvalue, Set) {
	COMPILER(
		"let a = 3\n"
		"fn test() {\n"
		"	a = a + 1\n"
		"}\n"
	);

	FN(0);
	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LF, 1, 1, 0);
	ASSERT_INSTR(UPVALUE_CLOSE, 0, 0, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(MOV_UL, 0, 0, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests closing upvalues when they go out of stack scope.
TEST(Upvalue, Close) {
	COMPILER(
		"fn adder() {\n"
		"	let i = 0\n"
		"	return fn() {\n"
		"		i = i + 1\n"
		"		return i\n"
		"	}\n"
		"}\n"
	);

	FN(0);
	ASSERT_INSTR(MOV_LF, 0, 1, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LI, 0, 0, 0);
	ASSERT_INSTR(UPVALUE_CLOSE, 0, 0, 0);
	ASSERT_INSTR(RET_F, 2, 0, 0);

	FN(2);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(MOV_UL, 0, 0, 0);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(RET_L, 0, 0, 0);

	COMPILER_FREE();
}


// Tests using multiple upvalues.
TEST(Upvalue, Multiple) {
	COMPILER(
		"let a = 0\n"
		"let b = 0\n"
		"fn adder() {\n"
		"	a = a + b\n"
		"}\n"
		"fn subtracter() {\n"
		"	a = a - b\n"
		"}\n"
	);

	FN(0);
	ASSERT_INSTR(MOV_LI, 0, 0, 0);
	ASSERT_INSTR(MOV_LI, 1, 0, 0);
	ASSERT_INSTR(MOV_LF, 2, 1, 0);
	ASSERT_INSTR(MOV_LF, 3, 2, 0);

	FN(1);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(MOV_LU, 1, 1, 0);
	ASSERT_INSTR(ADD_LL, 0, 0, 1);
	ASSERT_INSTR(MOV_UL, 0, 0, 0);
	ASSERT_RET();

	FN(2);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(MOV_LU, 1, 1, 0);
	ASSERT_INSTR(SUB_LL, 0, 0, 1);
	ASSERT_INSTR(MOV_UL, 0, 0, 0);
	ASSERT_RET();

	COMPILER_FREE();
}
