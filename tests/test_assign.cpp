
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
	ASSERT_INSTR(MOV_LS, 1, 0, 0);
	ASSERT_INSTR(MOV_LP, 2, TRUE_TAG, 0);
	ASSERT_INSTR(MOV_LL, 3, 2, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests we can use modified assignment operators like `+=`.
TEST(Assign, ModifiedAssignment) {
	COMPILER(
		"let a = 3\n"
		"a += 5\n"
		"a -= a * 3\n"
		"a *= 2\n"
		"a /= 5\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(ADD_LI, 0, 0, 5);
	ASSERT_INSTR(MUL_LI, 1, 0, 3);
	ASSERT_INSTR(SUB_LL, 0, 0, 1);
	ASSERT_INSTR(MUL_LI, 0, 0, 2);
	ASSERT_INSTR(DIV_LI, 0, 0, 5);

	ASSERT_RET();
	COMPILER_FREE();
}
