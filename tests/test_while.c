
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
	ASSERT_RET0();
	FREE_COMPILER();
}


TEST(break_statements) {

}


TEST(nested) {

}


MAIN() {
	RUN(basic);
	RUN(break_statements);
	RUN(nested);
}
