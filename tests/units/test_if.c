
//
//  If Statements
//


#include "test.h"


START(one) {
	COMPILER("if 1 + 2 > 3 {let testing = 3\n}");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_greater_than);
	ASSERT_CONDITIONAL_JUMP(13);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("\nif \n\r5\n == \n9 \n{\n}\n");

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NATIVE_CALL(operator_equal);
	ASSERT_CONDITIONAL_JUMP(0);

	ASSERT_RETURN_NIL();
}
END()


START(three) {
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
	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("if \n1\n\r \n{\nlet test = 3\n}\n\r \nelse\n\r \n{\n"
		"let meh = 4\n}\n");

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
	ASSERT_RETURN_NIL();
}
END()


START_MAIN(if) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
}
END_MAIN()