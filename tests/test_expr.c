
//
//  Expression Tests
//

#include "test.h"


TEST(assignment) {
	COMPILER("let a = 3\nlet b = 'hello'\nlet c = true\nlet d = c\n");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LS, 1, 0, 0);
	ASSERT_INSTRUCTION(MOV_LP, 2, TRUE_TAG, 0);
	ASSERT_INSTRUCTION(MOV_LL, 3, 2, 0);

	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(addition) {
	COMPILER("let a = 3\nlet b = 4\nlet c = a + b\nlet d = a * c\n"
		"let e = a - 3\nlet f = 5 / b");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LI, 1, 4, 0);
	ASSERT_INSTRUCTION(ADD_LL, 2, 0, 1);
	ASSERT_INSTRUCTION(MUL_LL, 3, 0, 2);
	ASSERT_INSTRUCTION(SUB_LI, 4, 0, 3);
	ASSERT_INSTRUCTION(DIV_IL, 5, 5, 1);

	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(precedence) {
	COMPILER("let a = 3\nlet b = 4\nlet c = 5\n"
		"let d = a * b + c\nlet e = a + b * c\nlet f = a * b + c * d\n"
		"let g = a * b * c\n");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LI, 1, 4, 0);
	ASSERT_INSTRUCTION(MOV_LI, 2, 5, 0);

	// a * b + c
	ASSERT_INSTRUCTION(MUL_LL, 3, 0, 1);
	ASSERT_INSTRUCTION(ADD_LL, 3, 3, 2);

	// a + b * c
	ASSERT_INSTRUCTION(MUL_LL, 5, 1, 2);
	ASSERT_INSTRUCTION(ADD_LL, 4, 0, 5);

	// a * b + c * d
	ASSERT_INSTRUCTION(MUL_LL, 5, 0, 1);
	ASSERT_INSTRUCTION(MUL_LL, 6, 2, 3);
	ASSERT_INSTRUCTION(ADD_LL, 5, 5, 6);

	// a * b * c
	ASSERT_INSTRUCTION(MUL_LL, 6, 0, 1);
	ASSERT_INSTRUCTION(MUL_LL, 6, 6, 2);

	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(parentheses) {
	COMPILER("let a = 3\nlet b = 4\nlet c = (a + b) * a\n"
		"let d = (a + b) * (c + a)\nlet e = (a + b) * (c + a) * (b + a)");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(MOV_LI, 1, 4, 0);

	// (a + b) * a
	ASSERT_INSTRUCTION(ADD_LL, 3, 0, 1);
	ASSERT_INSTRUCTION(MUL_LL, 2, 3, 0);

	// (a + b) * (c + a)
	ASSERT_INSTRUCTION(ADD_LL, 4, 0, 1);
	ASSERT_INSTRUCTION(ADD_LL, 5, 2, 0);
	ASSERT_INSTRUCTION(MUL_LL, 3, 4, 5);

	// (a + b) * (c + a) * (b + a)
	ASSERT_INSTRUCTION(ADD_LL, 5, 0, 1);
	ASSERT_INSTRUCTION(ADD_LL, 6, 2, 0);
	ASSERT_INSTRUCTION(MUL_LL, 4, 5, 6);
	ASSERT_INSTRUCTION(ADD_LL, 6, 1, 0);
	ASSERT_INSTRUCTION(MUL_LL, 4, 4, 6);

	ASSERT_RET0();
	FREE_COMPILER();
}


MAIN() {
	RUN(assignment);
	RUN(addition);
	RUN(precedence);
	RUN(parentheses);
}