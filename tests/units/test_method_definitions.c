
//
//  Method Definitions
//


#include "test.h"


START(one) {
	VM("class Test {a, b}\nfn (Test) test() {print(self.a)}\n");

	// main
	USE_FUNCTION(0);
	ASSERT_RETURN_NIL();

	// test
	USE_FUNCTION(1);
	ASSERT_PUSH_NATIVE(1);
	ASSERT_PUSH_LOCAL(0);
	ASSERT_PUSH_FIELD("a");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START_MAIN(method_definitions) {
	RUN(one)
}
END_MAIN()
