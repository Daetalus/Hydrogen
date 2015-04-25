
//
//  Function Calls
//


#include "test.h"


START(one) {
	COMPILER("print('hello')");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("\n\rprint\n\n\r(\n'hello'\n\r)\n");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_NATIVE_CALL(native_print);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(three) {
	COMPILER("print('hello', 'hai')");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_STRING_PUSH(1, "hai");
	ASSERT_NATIVE_CALL(native_print_2);
	ASSERT_INSTRUCTION(CODE_POP);

	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("\n\rprint\n\r(\n\r'hello'\n\r\n,\n \n'hai'\n\t)\n\r");

	ASSERT_STRING_PUSH(0, "hello");
	ASSERT_STRING_PUSH(1, "hai");
	ASSERT_NATIVE_CALL(native_print_2);
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