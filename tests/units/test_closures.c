
//
//  Closures
//


#include "test.h"


START(one) {
	VM("let a = 3\nfn test() {\nreturn a\n}\nprint(test())");

	// main
	USE_FUNCTION(0);
	ASSERT_NUMBER_PUSH(3.0);
	ASSERT_STORE(0);
	ASSERT_CALL(1);
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_UPVALUE_CLOSE(0);
	ASSERT_RETURN_NIL();

	// closure
	USE_FUNCTION(1);
	ASSERT_UPVALUE_PUSH(0);
	ASSERT_INSTRUCTION(CODE_RETURN);
}
END()


START_MAIN(closures) {
	RUN(one)
}
END_MAIN()
