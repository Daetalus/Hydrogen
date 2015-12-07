
//
//  Infinite Loop Tests
//

#include "test.h"


TEST(loop) {
	COMPILER("let a = 3\nloop {\na = a + 1\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(LOOP, 1, 0, 0);
	ASSERT_RET();
	FREE_COMPILER();
}


TEST(breaking) {
	COMPILER("let a = 3\nloop {\na = a + 1\nif a == 10 {\nbreak\n}\n}");

	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(ADD_LI, 0, 0, 1);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 10, 0);
	ASSERT_JMP(2);
	ASSERT_JMP(2);
	ASSERT_INSTRUCTION(LOOP, 4, 0, 0);
	ASSERT_RET();
	FREE_COMPILER();
}


MAIN() {
	RUN(loop);
	RUN(breaking);
}
