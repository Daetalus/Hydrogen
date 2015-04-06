
//
//  Expression Test
//


#include "test.h"

#include "../../src/vm.h"
#include "../../src/compiler.h"
#include "../../src/expression.h"
#include "../../src/debug.h"
#include "../../src/bytecode.h"
#include "../../src/lexer.h"
#include "../../src/operators.h"


#define EXPRESSION(content)                   \
	VirtualMachine vm = vm_new((content));    \
	Function fn;                              \
	fn.name = "test";                         \
	fn.name_length = 4;                       \
	fn.argument_count = 0;                    \
	Bytecode *bytecode = &fn.bytecode;        \
	bytecode_new(bytecode, 10);               \
	Compiler compiler;                        \
	compiler.vm = &vm;                        \
	compiler.fn = &fn;                        \
	compiler.local_count = 0;                 \
	compiler.scope_depth = 0;                 \
	compiler.has_error = false;               \
	compiler.string_literal_count = 0;        \
	expression(&compiler, TOKEN_END_OF_FILE); \
	uint8_t *cursor = &bytecode->instructions[0];


#define READ_BYTE() \
		(cursor++, (uint8_t) (*(cursor - 0)))

#define READ_2_BYTES() \
	(cursor += 2, ((uint16_t) *(cursor - 0) << (1 << 3)) | *(cursor - 1))

#define READ_4_BYTES() \
	(cursor += 4, \
		((uint32_t) *(cursor - 0) << (3 << 3)) | \
		((uint32_t) *(cursor - 1) << (2 << 3)) | \
		((uint32_t) *(cursor - 2) << (1 << 3)) | \
		(uint32_t) (*(cursor - 3)))

#define READ_8_BYTES() \
	(cursor += 8, \
		((uint64_t) *(cursor - 0) << (7 << 3)) | \
		((uint64_t) *(cursor - 1) << (6 << 3)) | \
		((uint64_t) *(cursor - 2) << (5 << 3)) | \
		((uint64_t) *(cursor - 3) << (4 << 3)) | \
		((uint64_t) *(cursor - 4) << (3 << 3)) | \
		((uint64_t) *(cursor - 5) << (2 << 3)) | \
		((uint64_t) *(cursor - 6) << (1 << 3)) | \
		((uint64_t) *(cursor - 7)))


#define ASSERT_NUMBER_PUSH(number)            \
	ASSERT_EQ(READ_BYTE(), CODE_PUSH_NUMBER); \
	ASSERT_EQ(as_number(READ_8_BYTES()), number);

#define ASSERT_OPERATOR_CALL(fn)              \
	ASSERT_EQ(READ_BYTE(), CODE_CALL_NATIVE); \
	ASSERT_EQ(AS_FN(READ_8_BYTES()), &(fn));


START(single_operand) {
	EXPRESSION("3");

	ASSERT_EQ(READ_BYTE(), CODE_PUSH_NUMBER);
	ASSERT_EQ(as_number(READ_8_BYTES()), 3.0);
}
END()


START(single_precedence_one) {
	EXPRESSION("3 + 4")

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(single_precedence_two) {
	EXPRESSION("3 * 4 / 5")

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_OPERATOR_CALL(operator_division);
}
END()


START(single_precedence_three) {
	EXPRESSION("1 - 2 - 3");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_OPERATOR_CALL(operator_subtraction);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_subtraction);
}
END()


START(multi_precedence_one) {
	EXPRESSION("3 * 4 + 5");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(multi_precedence_two) {
	EXPRESSION("5 + 3 * 4");

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(multi_precedence_three) {
	EXPRESSION("2 * 3 + 4 / 5");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_OPERATOR_CALL(operator_division);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(multi_precedence_four) {
	EXPRESSION("2 + 3 * 4 + 5");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(multi_precedence_five) {
	EXPRESSION("2 + 3 * 4 - 5 * 6");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(6.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_subtraction);
}
END()


START(boolean) {
	EXPRESSION("1 + 2 < 8 + 9 && 3 >= 90");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(8.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_OPERATOR_CALL(operator_less_than);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(90.0);
	ASSERT_OPERATOR_CALL(operator_greater_than_equal_to);
	ASSERT_OPERATOR_CALL(operator_boolean_and);
}
END()


START(single_parenthesis_one) {
	EXPRESSION("(1 + 2) * 3");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
}
END()


START(single_parenthesis_two) {
	EXPRESSION("1 * (3 - 2)");

	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_OPERATOR_CALL(operator_subtraction);
	ASSERT_OPERATOR_CALL(operator_multiplication);
}
END()


START(single_parenthesis_three) {
	EXPRESSION("2 * (3 + 4) / (9 - 3)");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_subtraction);
	ASSERT_OPERATOR_CALL(operator_division);
}
END()


START(nested_parenthesis_one) {
	EXPRESSION("2 * (3 + 4 * (2 + 6))");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(6.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_OPERATOR_CALL(operator_multiplication);
}
END()


START(nested_parenthesis_two) {
	EXPRESSION("2 / (9 - ((7 + 3) * 8))");

	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NUMBER_PUSH(7.0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_OPERATOR_CALL(operator_addition);
	ASSERT_NUMBER_PUSH(8.0);
	ASSERT_OPERATOR_CALL(operator_multiplication);
	ASSERT_OPERATOR_CALL(operator_subtraction);
	ASSERT_OPERATOR_CALL(operator_division);
}
END()


START(newlines_one) {
	EXPRESSION("3 + \n\t 4\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(newlines_two) {
	EXPRESSION("3\n\t + 4");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


START(newlines_three) {
	EXPRESSION("3\n\r\n\r +\n\r\n\r\n\n 4");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_OPERATOR_CALL(operator_addition);
}
END()


MAIN(expression) {
	RUN(single_operand)
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
}
MAIN_END()
