
//
//  While Loops
//


#include "test.h"


START(one) {
	COMPILER("while 1 {let test = 3\n}");

	// Conditional
	ASSERT_PUSH_NUMBER(1.0);
	ASSERT_CONDITIONAL_JUMP(16);

	// Block
	ASSERT_PUSH_NUMBER(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_BACKWARDS_JUMP(28);

	// After
	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("\n\nwhile\n 1 + 2\n {\n\nlet test = 3\r}\n");

	// Conditional
	ASSERT_PUSH_NUMBER(1.0);
	ASSERT_PUSH_NUMBER(2.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_CONDITIONAL_JUMP(16);

	// Block
	ASSERT_PUSH_NUMBER(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_BACKWARDS_JUMP(46);

	// After
	ASSERT_RETURN_NIL();
}
END()


START(three) {
	COMPILER("let i = 0\nwhile true {\nif i >= 3 {\nbreak\n}\n}");

	// let i = 0
	ASSERT_PUSH_NUMBER(0.0);
	ASSERT_STORE(0);

	// while true
	ASSERT_INSTRUCTION(CODE_PUSH_TRUE);
	ASSERT_CONDITIONAL_JUMP(30);

	// if i >= 3
	ASSERT_PUSH_LOCAL(0);
	ASSERT_PUSH_NUMBER(3.0);
	ASSERT_NATIVE_CALL(operator_greater_than_equal_to);
	ASSERT_CONDITIONAL_JUMP(3);

	// break
	ASSERT_JUMP(3);

	// while loop
	ASSERT_BACKWARDS_JUMP(34);

	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("while true {let i = 3\nif i == 3 {break}}");

	// while true
	ASSERT_INSTRUCTION(CODE_PUSH_TRUE);
	ASSERT_CONDITIONAL_JUMP(44);

	// let i = 3
	ASSERT_PUSH_NUMBER(3.0);
	ASSERT_STORE(0);

	// if i == 3
	ASSERT_PUSH_LOCAL(0);
	ASSERT_PUSH_NUMBER(3.0);
	ASSERT_NATIVE_CALL(operator_equal);
	ASSERT_CONDITIONAL_JUMP(4);

	// break
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_JUMP(4);

	// while loop
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_BACKWARDS_JUMP(48);

	ASSERT_RETURN_NIL();
}
END()


START_MAIN(while) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
}
END_MAIN()
