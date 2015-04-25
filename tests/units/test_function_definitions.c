
//
//  Function Definitions
//


#include "test.h"


START(one) {
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
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(two) {
	VM("\n\rfn\n test\n(\n)\n \n{\nlet a = 3\nprint(a)\n\n\n}\n "
		"\ntest\n(\n)\n");

	// main
	USE_FUNCTION(0);
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(three) {
	VM("fn test1(arg)\n{\n\tlet a = 4\n\tprint(arg)\n\tprint(a)\n}\n"
		"test1('hello')\n");

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
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_LOCAL_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(four) {
	VM("\nfn\n test(\n\rarg\n\r)\n\r {\n\rprint(arg)\n} test('hello')");

	// main
	USE_FUNCTION(0);
	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(five) {
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
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_LOCAL_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(six) {
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
	ASSERT_LOCAL_PUSH(0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_LOCAL_PUSH(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START_MAIN(function_definitions) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
	RUN(five)
	RUN(six)
}
END_MAIN()
