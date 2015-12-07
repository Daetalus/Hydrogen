
//
//  While Loop Tests
//

#include "test.h"


TEST(basic) {
	COMPILER("let a = 3\nwhile a < 100 {\na = a + 1\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(GE_LI, 0, 100, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(LOOP, 3, 0, 0);
	ASSERT_RET();
	FREE_COMPILER();
}


TEST(break_statements) {
	COMPILER("let a = 3\nwhile a < 1000 {\na = a + 1\n"
		"if a == 100 {\nbreak\n}\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(GE_LI, 0, 1000, 0);
	ASSERT_JMP(6);

	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 100, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(LOOP, 6, 0, 0);

	ASSERT_RET();
	FREE_COMPILER();
}


TEST(nested) {
	COMPILER("let a = 3\nwhile a < 100 {\n"
		"let b = 4\nwhile b < 100 {\nb = b + 1\n}\na = a + 1\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(GE_LI, 0, 100, 0);
	ASSERT_JMP(8);

	ASSERT_INSTRUCTION(MOV_LI, 1, 4, 0);
	ASSERT_INSTRUCTION(GE_LI, 1, 100, 0);
	ASSERT_JMP(3);
	ASSERT_INSTRUCTION(ADD_LI, 1, 1, 1);
	ASSERT_INSTRUCTION(LOOP, 3, 0, 0);

	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(LOOP, 8, 0, 0);

	ASSERT_RET();
	FREE_COMPILER();
}


TEST(nested_break) {
	COMPILER("let a = 3\nwhile a < 100 {\n"
		"let b = 4\n while b < 100 {\nb = b + 1\nif b == 10 {\nbreak\n}\n}\n"
		"a = a + 1\nif a == 20 {\nbreak\n}\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(GE_LI, 0, 100, 0);
	ASSERT_JMP(14);

	ASSERT_INSTRUCTION(MOV_LI, 1, 4, 0);
	ASSERT_INSTRUCTION(GE_LI, 1, 100, 0);
	ASSERT_JMP(6);
	ASSERT_INSTRUCTION(ADD_LI, 1, 1, 1);
	ASSERT_INSTRUCTION(NEQ_LI, 1, 10, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(LOOP, 6, 0, 0);

	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 20, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(LOOP, 14, 0, 0);

	ASSERT_RET();
	FREE_COMPILER();
}


MAIN() {
	RUN(basic);
	RUN(break_statements);
	RUN(nested);
	RUN(nested_break);
}
