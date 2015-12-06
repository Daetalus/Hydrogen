
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


TEST(return_value) {

}


TEST(arguments_and_return) {

}


TEST(call) {

}


TEST(multiple_definitions) {

}


TEST(inner_call) {

}


TEST(recursion) {

}


MAIN() {
	RUN(basic);
	RUN(single_argument);
	RUN(multiple_arguments);
	RUN(return_value);
	RUN(arguments_and_return);
	RUN(call);
	RUN(multiple_definitions);
	RUN(inner_call);
	RUN(recursion);
}
