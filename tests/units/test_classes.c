
//
//  Classes
//


#include "test.h"


START(one) {
	COMPILER("class \nTest\n{a,\nb,\n}");
	ASSERT_RETURN_NIL();
}
END()


START(two) {
	COMPILER("class Test {a, b}\nlet a = new Test()");

	ASSERT_INSTANTIATE_CLASS(0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(three) {
	COMPILER("class Test {a, b}\nlet a = \nnew\n\n Test(\n\n)\n");

	ASSERT_INSTANTIATE_CLASS(0);
	ASSERT_STORE(0);
	ASSERT_RETURN_NIL();
}
END()


START(four) {
	COMPILER("class Test{a, b}\nlet a = new Test()\nprint(a.a)");

	ASSERT_INSTANTIATE_CLASS(0);
	ASSERT_STORE(0);
	ASSERT_PUSH_NATIVE(0);
	ASSERT_PUSH_LOCAL(0);
	ASSERT_PUSH_FIELD("a");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START(five) {
	COMPILER("class Test{a, b}\nlet a = new Test()\nprint(a\n.\na)");

	ASSERT_INSTANTIATE_CLASS(0);
	ASSERT_STORE(0);
	ASSERT_PUSH_NATIVE(0);
	ASSERT_PUSH_LOCAL(0);
	ASSERT_PUSH_FIELD("a");
	ASSERT_CALL(1);
	ASSERT_INSTRUCTION(CODE_POP);
	ASSERT_RETURN_NIL();
}
END()


START_MAIN(classes) {
	RUN(one)
	RUN(two)
	RUN(three)
	RUN(four)
	RUN(five)
}
END_MAIN()
