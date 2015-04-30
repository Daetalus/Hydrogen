
//
//  Function Calls
//


#include "test.h"


START(one) {
	COMPILER("print('hello')");

	ASSERT_PUSH_NATIVE(0);
	ASSERT_PUSH_STRING(0, "hello");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("\n\rprint(\n'hello'\n\r)\n");

	ASSERT_PUSH_NATIVE(0);
	ASSERT_PUSH_STRING(0, "hello");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(three) {
	COMPILER("print2('hello', 'hai')");

	ASSERT_PUSH_NATIVE(1);
	ASSERT_PUSH_STRING(0, "hello");
	ASSERT_PUSH_STRING(1, "hai");
	ASSERT_CALL(2);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("\n\rprint2(\n\r'hello'\n\r\n,\n \n'hai'\n\t)\n\r");

	ASSERT_PUSH_NATIVE(1);
	ASSERT_PUSH_STRING(0, "hello");
	ASSERT_PUSH_STRING(1, "hai");
	ASSERT_CALL(2);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START_MAIN(function_calls) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
}
END_MAIN()
