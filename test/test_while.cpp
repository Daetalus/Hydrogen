
//
//  While Loop Tests
//

#include "test.h"


// Tests a single while loop.
TEST(While, Single) {
	COMPILER(
		"let a = 3\n"
		"while a < 100 {\n"
		"	a = a + 1\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(GE_LI, 0, 100, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_INSTR(LOOP, 6, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests a break statement from within a while loop.
TEST(While, Break) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 1000 {\n"
		"	a = a + 1\n"
		"	if a == 100 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(GE_LI, 0, 1000, 0);
	ASSERT_JMP(6);

	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(NEQ_LI, 0, 100, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTR(LOOP, 6, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests two nested while loops.
TEST(While, Nested) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"	}\n"
		"	a = a + 1\n"
		"}\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(GE_LI, 0, 100, 0);
	ASSERT_JMP(8);

	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(GE_LI, 1, 100, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(ADD_LI, 1, 1, 1);
	ASSERT_INSTR(LOOP, 3, 0, 0);

	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(LOOP, 8, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests a break statement from within a nested while loop.
TEST(While, NestedBreak) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"		if b == 10 {\n"
		"			break\n"
		"		}\n"
		"	}\n"
		"	a = a + 1\n"
		"	if a == 20 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(GE_LI, 0, 100, 0);
	ASSERT_JMP(14);

	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(GE_LI, 1, 100, 0);
	ASSERT_JMP(6);
	ASSERT_INSTR(ADD_LI, 1, 1, 1);
	ASSERT_INSTR(NEQ_LI, 1, 10, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTR(LOOP, 6, 0, 0);

	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(NEQ_LI, 0, 20, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTR(LOOP, 14, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}
