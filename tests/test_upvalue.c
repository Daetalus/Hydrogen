
//
//  Upvalue Tests
//

#include "test.h"


TEST(getting) {
	COMPILER("let a = 3\nfn test() {\nlet b = a + 2\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LF, 1, 1, 0);
	ASSERT_INSTRUCTION(CLOSE_U, 0, 0, 0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 2);
	ASSERT_RET();
}


TEST(setting) {
	COMPILER("let a = 3\nfn test() {\na = a + 1\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LF, 1, 1, 0);
	ASSERT_INSTRUCTION(CLOSE_U, 0, 0, 0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(MOV_UL, 0, 0, 0);
	ASSERT_RET();
}


TEST(closing) {
	COMPILER("fn adder() {\nlet i = 0\nreturn fn() {\n"
		"i = i + 1\nreturn i\n}\n}\n");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 0, 0);
	ASSERT_INSTRUCTION(CLOSE_U, 0, 0, 0);
	ASSERT_INSTRUCTION(RET_F, 2, 0, 0);

	SELECT_FN(2);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(MOV_UL, 0, 0, 0);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(RET_L, 0, 0, 0);
}


TEST(multiple) {
	COMPILER("let a = 0\nlet b = 0\nfn adder() {\na = a + b\n}\n"
		"fn subtracter() {\na = a - b\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LI, 0, 0, 0);
	ASSERT_INSTRUCTION(MOV_LI, 1, 0, 0);
	ASSERT_INSTRUCTION(MOV_LF, 2, 1, 0);
	ASSERT_INSTRUCTION(MOV_LF, 3, 2, 0);

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(MOV_LU, 1, 1, 0);
	ASSERT_INSTRUCTION(ADD_LL, 0, 0, 1);
	ASSERT_INSTRUCTION(MOV_UL, 0, 0, 0);
	ASSERT_RET();

	SELECT_FN(2);
	ASSERT_INSTRUCTION(MOV_LU, 0, 0, 0);
	ASSERT_INSTRUCTION(MOV_LU, 1, 1, 0);
	ASSERT_INSTRUCTION(SUB_LL, 0, 0, 1);
	ASSERT_INSTRUCTION(MOV_UL, 0, 0, 0);
	ASSERT_RET();
}


MAIN() {
	RUN(getting);
	RUN(setting);
	RUN(closing);
	RUN(multiple);
}