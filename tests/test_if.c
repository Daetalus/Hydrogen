
//
//  If Statement Tests
//

#include "test.h"


TEST(if_only) {
	COMPILER("let a = 3\nif a == 3 {\na = 4\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 4, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(if_else) {
	COMPILER("let a = 3\nif a == 4 {\na = 4\n} else {\na = 5\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 4, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 5, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(single_else_if) {
	COMPILER("let a = 3\nif a == 4 {\na = 5\n} else if a == 5 {\na = 6\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 5, 0);
	ASSERT_JMP(4);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 6, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(multiple_else_ifs) {
	COMPILER("let a = 3\nif a == 4 {\na = 5\n} else if a == 5 {\na = 6\n"
		"} else if a == 7 {\n a = 8\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 5, 0);
	ASSERT_JMP(8);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 6, 0);
	ASSERT_JMP(4);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 7, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 8, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(else_if_else) {
	COMPILER("let a = 3\nif a == 4 {\na = 5\n} else if a == 5 {\na = 6\n"
		"} else {\na = 7\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 5, 0);
	ASSERT_JMP(6);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 6, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 7, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(else_ifs_else) {
	COMPILER("let a = 3\nif a == 4 {\na = 5\n} else if a == 5 {\n a = 6\n"
		"} else if a == 6 {\n a = 7\n} else {\na = 8\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 5, 0);
	ASSERT_JMP(10);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 6, 0);
	ASSERT_JMP(6);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(MOV_LI, 0, 7, 0);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(MOV_LI, 0, 8, 0);
	ASSERT_RET0();
	FREE_COMPILER();
}


MAIN() {
	RUN(if_only);
	RUN(if_else);
	RUN(single_else_if);
	RUN(multiple_else_ifs);
	RUN(else_if_else);
	RUN(else_ifs_else);
}
