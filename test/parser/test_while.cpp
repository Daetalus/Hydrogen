
//
//  While Loop Tests
//

#include "test.h"


// Tests a single while loop.
TEST(While, Single) {
	COMPILER(
		"let a = 3\n"
		"while a < 100 {\n"
		"	a = a + 1\n"
		"}\n"
	);

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(GE_LI, 0, 100, 0);
	JMP(5);
	INS(MOV_LT, 0, 0, 0);
	INS(ADD_LI, 0, 0, 1);
	INS(MOV_TL, 0, 0, 0);
	INS(LOOP, 6, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests a break statement from within a while loop.
TEST(While, Break) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 1000 {\n"
		"	a = a + 1\n"
		"	if a == 100 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(GE_LI, 0, 1000, 0);
	JMP(6);

	INS(ADD_LI, 0, 0, 1);
	INS(NEQ_LI, 0, 100, 0);
	JMP(2);
	JMP(2);
	INS(LOOP, 6, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests two nested while loops.
TEST(While, Nested) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"	}\n"
		"	a = a + 1\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(GE_LI, 0, 100, 0);
	JMP(8);

	INS(MOV_LI, 1, 4, 0);
	INS(GE_LI, 1, 100, 0);
	JMP(3);
	INS(ADD_LI, 1, 1, 1);
	INS(LOOP, 3, 0, 0);

	INS(ADD_LI, 0, 0, 1);
	INS(LOOP, 8, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests a break statement from within a nested while loop.
TEST(While, NestedBreak) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"		if b == 10 {\n"
		"			break\n"
		"		}\n"
		"	}\n"
		"	a = a + 1\n"
		"	if a == 20 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(GE_LI, 0, 100, 0);
	JMP(14);

	INS(MOV_LI, 1, 4, 0);
	INS(GE_LI, 1, 100, 0);
	JMP(6);
	INS(ADD_LI, 1, 1, 1);
	INS(NEQ_LI, 1, 10, 0);
	JMP(2);
	JMP(2);
	INS(LOOP, 6, 0, 0);

	INS(ADD_LI, 0, 0, 1);
	INS(NEQ_LI, 0, 20, 0);
	JMP(2);
	JMP(2);
	INS(LOOP, 14, 0, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}
