
//
//  Compiler Test
//


#include "test.h"


START(variable_assignment_one) {
	COMPILER("let a = 3");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
}
END()


START(variable_assignment_two) {
	COMPILER("\n\rlet\n\r \n\ra\n\r \n=\n\n \n\r3\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
}
END()


START(variable_assignment_three) {
	COMPILER("\nlet testing = 3 + 4 *\n 9\n\r");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_STORE(0);
}
END()


MAIN(compiler) {
	RUN(variable_assignment_one)
	RUN(variable_assignment_two)
	RUN(variable_assignment_three)
}
MAIN_END()
