
//
//  If Statement Tests
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 4, 0);
	ASSERT_RET();

	COMPILER_FREE();
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 4, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests an if followed by a single else if.
TEST(If, IfElseIf) {
	COMPILER(
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_JMP(4);
	ASSERT_INSTR(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 6, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests an if followed by multiple else ifs.
TEST(If, IfElseIfs) {
	COMPILER(
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else if a == 7 {\n"
		"	a = 8\n"
		"}"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_JMP(8);
	ASSERT_INSTR(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 6, 0);
	ASSERT_JMP(4);
	ASSERT_INSTR(NEQ_LI, 0, 7, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 8, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests an if, followed by an else if, followed by an else.
TEST(If, ElseIfElse) {
	COMPILER(
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else {\n"
		"	a = 7\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_JMP(6);
	ASSERT_INSTR(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 6, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 7, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests an ifs, followed by multiple else ifs, followed by an else.
TEST(If, ElseIfsElse) {
	COMPILER(
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
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(NEQ_LI, 0, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 5, 0);
	ASSERT_JMP(10);
	ASSERT_INSTR(NEQ_LI, 0, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 6, 0);
	ASSERT_JMP(6);
	ASSERT_INSTR(NEQ_LI, 0, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LI, 0, 7, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LI, 0, 8, 0);
	ASSERT_RET();

	COMPILER_FREE();
}