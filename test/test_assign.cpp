
//
//  Assignment Tests
//

#include "test.h"


// Tests we can assign single values to variables.
TEST(Assign, Assignment) {
	COMPILER(
		"let a = 3\n"
		"let b = 'hello'\n"
		"let c = true\n"
		"let d = c\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);

	ASSERT_INSTR(MOV_LS, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 1, 0, 0);

	ASSERT_INSTR(MOV_LP, 0, TRUE_TAG, 0);
	ASSERT_INSTR(MOV_TL, 2, 0, 0);

	ASSERT_INSTR(MOV_LT, 0, 0, 2);
	ASSERT_INSTR(MOV_TL, 3, 0, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests we can assign to variables between scopes.
TEST(Assign, Scopes) {
	COMPILER(
		"let a = 3\n"
		"if a == 3 {\n"
		"	let b = 5\n"
		"	a = 4\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);

	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(4);

	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 1);

	ASSERT_RET();
	COMPILER_FREE();
}
