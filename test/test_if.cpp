
//
//  If Tests
//

#include "test.h"


// Tests a single if statement.
TEST(If, If) {
	COMPILER(
		"let a = 3\n"
		"if a == 3 {\n"
		"	a = 4\n"
		"}\n"
	);

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(NEQ_LI, 0, 3, 0);
	JMP(2);
	INS(MOV_TI, 0, 4, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests an if followed by an else.
TEST(If, IfElse) {
	COMPILER(
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 4\n"
		"} else {\n"
		"	a = 5\n"
		"}\n"
	);

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(NEQ_LI, 0, 4, 0);
	JMP(3);
	INS(MOV_TI, 0, 4, 0);
	JMP(2);
	INS(MOV_TI, 0, 5, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests an if followed by a single else if.
TEST(If, IfElseIf) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(NEQ_LI, 0, 4, 0);
	JMP(3);
	INS(MOV_LI, 0, 5, 0);
	JMP(4);
	INS(NEQ_LI, 0, 5, 0);
	JMP(2);
	INS(MOV_LI, 0, 6, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests an if followed by multiple else ifs.
TEST(If, IfElseIfs) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else if a == 7 {\n"
		"	a = 8\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(NEQ_LI, 0, 4, 0);
	JMP(3);
	INS(MOV_LI, 0, 5, 0);
	JMP(8);
	INS(NEQ_LI, 0, 5, 0);
	JMP(3);
	INS(MOV_LI, 0, 6, 0);
	JMP(4);
	INS(NEQ_LI, 0, 7, 0);
	JMP(2);
	INS(MOV_LI, 0, 8, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests an if, followed by an else if, followed by an else.
TEST(If, ElseIfElse) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else {\n"
		"	a = 7\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(NEQ_LI, 0, 4, 0);
	JMP(3);
	INS(MOV_LI, 0, 5, 0);
	JMP(6);
	INS(NEQ_LI, 0, 5, 0);
	JMP(3);
	INS(MOV_LI, 0, 6, 0);
	JMP(2);
	INS(MOV_LI, 0, 7, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests an ifs, followed by multiple else ifs, followed by an else.
TEST(If, ElseIfsElse) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else if a == 6 {\n"
		"	a = 7\n"
		"} else {\n"
		"	a = 8\n"
		"}\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(NEQ_LI, 0, 4, 0);
	JMP(3);
	INS(MOV_LI, 0, 5, 0);
	JMP(10);
	INS(NEQ_LI, 0, 5, 0);
	JMP(3);
	INS(MOV_LI, 0, 6, 0);
	JMP(6);
	INS(NEQ_LI, 0, 6, 0);
	JMP(3);
	INS(MOV_LI, 0, 7, 0);
	JMP(2);
	INS(MOV_LI, 0, 8, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}
