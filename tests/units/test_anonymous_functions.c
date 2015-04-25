
//
//  Anonymous Functions
//


#include "test.h"


START(one) {
	VM("let a = fn() { print(3) }");

	// main
	USE_FUNCTION(0);
	ASSERT_CLOSURE_PUSH(1);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();

	// anonymous function
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(two) {
	VM("let a = fn() {print(3)}\na()");

	// main
	USE_FUNCTION(0);
	ASSERT_CLOSURE_PUSH(1);
	ASSERT_STORE(0);
	ASSERT_LOCAL_PUSH(0);
	ASSERT_INSTRUCTION(CODE_CALL_STACK);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// anonymous function
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(three) {
	VM("let a\n = \nfn\n(\n)\n\r {\nprint(3)\n}\n\r\na\n(\n)\n");

	// main
	USE_FUNCTION(0);
	ASSERT_CLOSURE_PUSH(1);
	ASSERT_STORE(0);
	ASSERT_LOCAL_PUSH(0);
	ASSERT_INSTRUCTION(CODE_CALL_STACK);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();

	// anonymous function
	USE_FUNCTION(1);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START_MAIN(anonymous_functions) {
	RUN(one)
	RUN(two)
	RUN(three)
}
END_MAIN()