
//
//  Function Tests
//

#include "test.h"


TEST(basic) {
	COMPILER("fn test() {\nlet a = 1\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 1, 0);
	ASSERT_RET0();

	FREE_COMPILER();
}


TEST(single_argument) {
	COMPILER("fn test(arg1) {\nlet a = arg1 + 1\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(ADD_LI, 1, 0, 1);
	ASSERT_RET0();

	FREE_COMPILER();
}


TEST(multiple_arguments) {
	COMPILER("fn test(arg1, arg2) {\nlet a = arg1 + arg2\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(ADD_LL, 2, 0, 1);
	ASSERT_RET0();

	FREE_COMPILER();
}


TEST(return_nothing) {
	COMPILER("fn test() {\nlet a = 3\nif a == 3 {\nreturn\n}\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(2);
	ASSERT_RET0();
	ASSERT_RET0();

	FREE_COMPILER();
}


TEST(return_value) {
	COMPILER("fn test() {\nlet a = 3\nreturn a + 3\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(ADD_LI, 1, 0, 3);
	ASSERT_INSTRUCTION(RET1, 1, 0, 0);
}


TEST(arguments_and_return) {
	COMPILER("fn test(arg1, arg2) {\nreturn arg1 * arg2 * 2\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MUL_LL, 2, 0, 1);
	ASSERT_INSTRUCTION(MUL_LI, 2, 2, 2);
	ASSERT_INSTRUCTION(RET1, 2, 0, 0);
}


TEST(call) {
	COMPILER("fn test() {\nlet a = 1\n}\ntest()");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_CALL(0, 0, 0, 1);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 1, 0);
	ASSERT_RET0();
}


TEST(call_arg) {
	COMPILER("fn test(arg1) {\nlet a = arg1\n}\ntest(2)");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_INSTRUCTION(MOV_LI, 2, 2, 0);
	ASSERT_CALL(1, 0, 2, 1);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LL, 1, 0, 0);
	ASSERT_RET0();
}


TEST(call_multiple_args) {
	COMPILER("fn test(arg1, arg2, arg3) {\nlet a = arg1 + arg2 + arg3\n}\n"
		"test(1, 2, 3)\n");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_INSTRUCTION(MOV_LI, 2, 1, 0);
	ASSERT_INSTRUCTION(MOV_LI, 3, 2, 0);
	ASSERT_INSTRUCTION(MOV_LI, 4, 3, 0);
	ASSERT_CALL(3, 0, 2, 1);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(ADD_LL, 3, 0, 1);
	ASSERT_INSTRUCTION(ADD_LL, 3, 3, 2);
	ASSERT_RET0();
}


TEST(call_return_value) {
	COMPILER("fn test() {\nreturn 3\n}\nlet a = test() * 2\n");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_CALL(0, 0, 0, 1);
	ASSERT_INSTRUCTION(MUL_LI, 1, 1, 2);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 0, 3, 0);
	ASSERT_INSTRUCTION(RET1, 0, 0, 0);
}


TEST(multiple_definitions) {
	COMPILER("fn square(num) {\nreturn num * num\n}\n"
		"fn mul(num, other) {\nreturn num * other\n}");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_INSTRUCTION(MOV_LF, 1, 2, 0);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MUL_LL, 1, 0, 0);
	ASSERT_INSTRUCTION(RET1, 1, 0, 0);

	SELECT_FN(2);
	ASSERT_INSTRUCTION(MUL_LL, 2, 0, 1);
	ASSERT_INSTRUCTION(RET1, 2, 0, 0);
}


TEST(inner_call) {
	COMPILER("fn test(arg) {\nreturn arg + 1\n}\nlet a = test(test(1))");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(MOV_LF, 0, 1, 0);
	ASSERT_INSTRUCTION(MOV_LI, 3, 1, 0);
	ASSERT_CALL(1, 0, 3, 2);
	ASSERT_CALL(1, 0, 2, 1);
	ASSERT_RET0();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(ADD_LI, 1, 0, 1);
	ASSERT_INSTRUCTION(RET1, 1, 0, 0);
}


MAIN() {
	RUN(basic);
	RUN(single_argument);
	RUN(multiple_arguments);
	RUN(return_value);
	RUN(return_nothing);
	RUN(arguments_and_return);
	RUN(call);
	RUN(call_arg);
	RUN(call_multiple_args);
	RUN(call_return_value);
	RUN(multiple_definitions);
	RUN(inner_call);
}
