
//
//  Expression Test
//


#include "test.h"


START(single_operand_one) {
	EXPRESSION("3");

	ASSERT_NUMBER_PUSH(3.0);
}
END()


START(single_operand_two) {
	EXPRESSION("\n3\r\n");

	ASSERT_NUMBER_PUSH(3.0);
}
END()


START(single_precedence_one) {
	EXPRESSION("3 + 4")

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(single_precedence_two) {
	EXPRESSION("3 * 4\n / 5")

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NATIVE_CALL(operator_division);
}
END()


START(single_precedence_three) {
	EXPRESSION("1\n \n-\n 2 - 3");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_subtraction);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_subtraction);
}
END()


START(multi_precedence_one) {
	EXPRESSION("3 * 4 +\n 5\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(multi_precedence_two) {
	EXPRESSION("5 +\n 3 * 4");

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(multi_precedence_three) {
	EXPRESSION("2 * 3 + 4 / 5");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NATIVE_CALL(operator_division);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(multi_precedence_four) {
	EXPRESSION("2 +\r 3 * 4 + 5");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(multi_precedence_five) {
	EXPRESSION("2 + 3 * 4 - 5 * 6");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(6.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_subtraction);
}
END()


START(boolean) {
	EXPRESSION("1 + 2 < 8 + 9 && 3 >= 90");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(8.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NATIVE_CALL(operator_less_than);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(90.0);
	ASSERT_NATIVE_CALL(operator_greater_than_equal_to);
	ASSERT_NATIVE_CALL(operator_boolean_and);
}
END()


START(single_parenthesis_one) {
	EXPRESSION("(1 + 2) * 3");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
}
END()


START(single_parenthesis_two) {
	EXPRESSION("1 * (3 - 2)");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_subtraction);
	ASSERT_NATIVE_CALL(operator_multiplication);
}
END()


START(single_parenthesis_three) {
	EXPRESSION("2 * (3 + 4) / (9 - 3)");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_subtraction);
	ASSERT_NATIVE_CALL(operator_division);
}
END()


START(nested_parenthesis_one) {
	EXPRESSION("2 * (3 + 4 * (2 + 6))");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(6.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NATIVE_CALL(operator_multiplication);
}
END()


START(nested_parenthesis_two) {
	EXPRESSION("2 / (9 - ((7 + 3) * 8))");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NUMBER_PUSH(7.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(8.0);
	ASSERT_NATIVE_CALL(operator_multiplication);
	ASSERT_NATIVE_CALL(operator_subtraction);
	ASSERT_NATIVE_CALL(operator_division);
}
END()


START(newlines_one) {
	EXPRESSION("3 + \n\t 4\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(newlines_two) {
	EXPRESSION("3\n\t + 4");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(newlines_three) {
	EXPRESSION("3\n\r\n\r +\n\r\n\r\n\n 4");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


START(strings) {
	EXPRESSION("'hello' + 'hai'");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_STRING_PUSH(1, "hai");
	ASSERT_NATIVE_CALL(operator_addition);
}
END()


MAIN(expression) {
	RUN(single_operand_one)
	RUN(single_operand_two)
	RUN(single_precedence_one)
	RUN(single_precedence_two)
	RUN(single_precedence_three)
	RUN(multi_precedence_one)
	RUN(multi_precedence_two)
	RUN(multi_precedence_three)
	RUN(multi_precedence_four)
	RUN(multi_precedence_five)
	RUN(boolean)
	RUN(single_parenthesis_one)
	RUN(single_parenthesis_two)
	RUN(single_parenthesis_three)
	RUN(nested_parenthesis_one)
	RUN(nested_parenthesis_two)
	RUN(newlines_one)
	RUN(newlines_two)
	RUN(newlines_three)
	RUN(strings)
}
MAIN_END()
