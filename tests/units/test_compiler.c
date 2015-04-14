
//
//  Compiler Test
//


#include "test.h"

#include "../../src/vm.c"


START(variable_assignment_one) {
	COMPILER("let a = 3");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(variable_assignment_two) {
	COMPILER("\n\rlet\n\r \n\ra\n\r \n=\n\n \n\r3\n");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(variable_assignment_three) {
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


START(variable_assignment_four) {
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


START(modifier_assignment_operators) {
	COMPILER("let testing = 3\ntesting += 1");

	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);

	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_STORE(0);

	ASSERT_RETURN_NIL();
}
END()


START(if_statement_one) {
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


START(if_statement_two) {
	COMPILER("\nif \n\r5\n == \n9 \n{\n}\n");

	ASSERT_NUMBER_PUSH(5.0);
	ASSERT_NUMBER_PUSH(9.0);
	ASSERT_NATIVE_CALL(operator_equal);
	ASSERT_CONDITIONAL_JUMP(0);

	ASSERT_RETURN_NIL();
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
	ASSERT_RETURN_NIL();
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
	ASSERT_RETURN_NIL();
}
END()


START(while_loop_one) {
	COMPILER("while 1 {let test = 3\n}");

	// Conditional
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_CONDITIONAL_JUMP(16);

	// Block
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_BACKWARDS_JUMP(28);

	// After
	ASSERT_RETURN_NIL();
}
END()


START(while_loop_two) {
	COMPILER("\n\nwhile\n 1 + 2\n {\n\nlet test = 3\r}\n");

	// Conditional
	ASSERT_NUMBER_PUSH(1.0);
	ASSERT_NUMBER_PUSH(2.0);
	ASSERT_NATIVE_CALL(operator_addition);
	ASSERT_CONDITIONAL_JUMP(16);

	// Block
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_BACKWARDS_JUMP(46);

	// After
	ASSERT_RETURN_NIL();
}
END()


START(while_loop_three) {
	COMPILER("let i = 0\nwhile true {\nif i >= 3 {\nbreak\n}\n}");

	// let i = 0
	ASSERT_NUMBER_PUSH(0.0);
	ASSERT_STORE(0);

	// while true
	ASSERT_INSTRUCTION(CODE_PUSH_TRUE);
	ASSERT_CONDITIONAL_JUMP(30);

	// if i >= 3
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(operator_greater_than_equal_to);
	ASSERT_CONDITIONAL_JUMP(3);

	// break
	ASSERT_JUMP(3);

	// while loop
	ASSERT_BACKWARDS_JUMP(34);

	ASSERT_RETURN_NIL();
}
END()


START(while_loop_four) {
	COMPILER("while true {let i = 3\nif i == 3 {break}}");

	// while true
	ASSERT_INSTRUCTION(CODE_PUSH_TRUE);
	ASSERT_CONDITIONAL_JUMP(44);

	// let i = 3
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);

	// if i == 3
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NUMBER_PUSH(3.0);
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


START(function_call_one) {
	COMPILER("print('hello')");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(function_call_two) {
	COMPILER("\n\rprint\n\n\r(\n'hello'\n\r)\n");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(function_call_three) {
	COMPILER("print('hello', 'hai')");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_STRING_PUSH(1, "hai");
	ASSERT_NATIVE_CALL(native_print_2);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(function_call_four) {
	COMPILER("\n\rprint\n\r(\n\r'hello'\n\r\n,\n \n'hai'\n\t)\n\r");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_STRING_PUSH(1, "hai");
	ASSERT_NATIVE_CALL(native_print_2);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(function_definition_one) {
	VM("fn test() {let a = 3\nprint(a)\n}\n test()");

	// main
	USE_FUNCTION(0);
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(function_definition_two) {
	VM("\n\rfn\n test\n(\n)\n \n{\nlet a = 3\nprint(a)\n\n\n}\n \ntest\n(\n)\n");

	// main
	USE_FUNCTION(0);
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(function_definition_three) {
	VM("fn test1(arg)\n{\n\tlet a = 4\n\tprint(arg)\n\tprint(a)\n}\ntest1('hello')\n");

	// main
	USE_FUNCTION(0);
	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(4.0);
	ASSERT_STORE(1);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_VARIABLE_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(function_definition_four) {
	VM("\nfn\n test(\n\rarg\n\r)\n\r {\n\rprint(arg)\n} test('hello')");

	// main
	USE_FUNCTION(0);
	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(function_definition_five) {
	VM("fn test(arg1, arg2) {print(arg1)print(arg2)}test('h', 'a')");

	// main
	USE_FUNCTION(0);
	ASSERT_STRING_PUSH(0, "h");
	ASSERT_STRING_PUSH(1, "a");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_VARIABLE_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(function_definition_six) {
	VM("\nfn \ntest\n\r(\narg1\n\r,\n\r \narg2\n\r)\n {\n\r"
		"print(arg1)print(arg2)}test('h', 'a')");

	// main
	USE_FUNCTION(0);
	ASSERT_STRING_PUSH(0, "h");
	ASSERT_STRING_PUSH(1, "a");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_VARIABLE_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_VARIABLE_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


MAIN(compiler) {
	RUN(variable_assignment_one)
	RUN(variable_assignment_two)
	RUN(variable_assignment_three)
	RUN(variable_assignment_four)
	RUN(modifier_assignment_operators)
	RUN(if_statement_one)
	RUN(if_statement_two)
	RUN(if_else_statement_one)
	RUN(if_else_statement_two)
	RUN(while_loop_one)
	RUN(while_loop_two)
	RUN(while_loop_three)
	RUN(while_loop_four)
	RUN(function_call_one)
	RUN(function_call_two)
	RUN(function_call_three)
	RUN(function_call_four)
	RUN(function_definition_one)
	RUN(function_definition_two)
	RUN(function_definition_three)
	RUN(function_definition_four)
	RUN(function_definition_five)
	RUN(function_definition_six)
}
MAIN_END()
