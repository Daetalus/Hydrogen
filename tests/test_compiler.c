
//
//  Compiler Test
//


#include "test.h"


START(variable_assignment_one) {
	COMPILER("let a = 3");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(variable_assignment_two) {
	COMPILER("\n\rlet\n\r \n\ra\n\r \n=\n\n \n\r3\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_INSTRUCTION(CODE_RETURN);
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
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(variable_assignment_four) {
	COMPILER("\nlet testing = 3 + 4 *\n 9\ntesting = 5\r");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_STORE(0);

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_STORE(0);

	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(if_statement_one) {
	COMPILER("if 1 + 2 > 3 {let testing = 3}");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_greater_than);
	ASSERT_CONDITIONAL_JUMP(13);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(if_statement_two) {
	COMPILER("\nif \n\r5\n == \n9 \n{\n}\n");

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_OPERATOR_CALL(operator_equal);
	ASSERT_CONDITIONAL_JUMP(0);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(if_else_statement_one) {
	COMPILER("if 1 {\nlet test = 3\n} else {\nlet meh = 4\n}\n");

	// If conditional
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_CONDITIONAL_JUMP(16);

	// If block
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_JUMP(13);

	// Else block
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);

	// Outside
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START(if_else_statement_two) {
	COMPILER("if \n1\n\r \n{\nlet test = 3\n}\n\r \nelse\n\r \n{\nlet meh = 4\n}\n");

	// If conditional
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_CONDITIONAL_JUMP(16);

	// If block
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_JUMP(13);

	// Else block
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);

	// Outside
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


MAIN(compiler) {
	RUN(variable_assignment_one)
	RUN(variable_assignment_two)
	RUN(variable_assignment_three)
	RUN(variable_assignment_four)
	RUN(if_statement_one)
	RUN(if_statement_two)
	RUN(if_else_statement_one)
	RUN(if_else_statement_two)
}
MAIN_END()
