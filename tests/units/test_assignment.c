
//
//  Variable Assignment
//


#include "test.h"


START(one) {
	COMPILER("let a = 3");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("\n\rlet\n\r \n\ra\n\r \n=\n\n \n\r3\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(three) {
	COMPILER("\nlet testing = 3 + 4 *\n 9\n\r");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("\nlet testing = 3 + 4 *\n 9\ntesting = 5\r");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_STORE(0);

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_STORE(0);

	ASSERT_RETURN_NIL();
}
END()


START(modifier_one) {
	COMPILER("let testing = 3\ntesting += 1");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);

	ASSERT_LOCAL_PUSH(0);
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_STORE(0);

	ASSERT_RETURN_NIL();
}
END()


START(modifier_two) {
	COMPILER("let \n\rtesting = 3\n\n\rtesting \n \n\r+= \n\r1\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);

	ASSERT_LOCAL_PUSH(0);
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_STORE(0);

	ASSERT_RETURN_NIL();
}
END()


START_MAIN(assignment) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
	RUN(modifier_one)
	RUN(modifier_two)
}
END_MAIN()
