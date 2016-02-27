
//
//  Infinite Loop Tests
//

#include "test.h"


// Tests an infinite loop
TEST(Loop, InfiniteLoop) {
	COMPILER(
		"let a = 3\n"
		"loop {\n"
		"	a = a + 1\n"
		"}\n"
	);

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(ADD_LI, 0, 0, 1);
	INS(MOV_TL, 0, 0, 0);
	INS(LOOP, 3, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests breaking from within an infinite loop
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

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(ADD_LI, 0, 0, 1);
	INS(MOV_TL, 0, 0, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(NEQ_LI, 0, 10, 0);
	JMP(2);
	JMP(2);
	INS(LOOP, 7, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}
