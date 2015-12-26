
//
//  Infinite Loop Tests
//

#include "test.h"


// Tests an infinite loop.
TEST(Loop, Loop) {
	COMPILER(
		"let a = 3\n"
		"loop {\n"
		"	a = a + 1\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(LOOP, 1, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests breaking from within an infinite loop.
TEST(Loop, Break) {
	COMPILER(
		"let a = 3\n"
		"loop {\n"
		"	a = a + 1\n"
		"	if a == 10 {\n"
		"		break\n"
		"	}\n"
		"}"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 1);
	ASSERT_INSTR(NEQ_LI, 0, 10, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTR(LOOP, 4, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}
