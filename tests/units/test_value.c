
//
//  Value Test
//


#include "test.h"

#include "../../src/value.h"


START(conversion) {
	uint64_t value = number_to_value(3.0);
	double converted = value_to_number(value);
	ASSERT_EQ(converted, 3.0);

	char *str = "hello";
	uint64_t value2 = ptr_to_value(str);
	ASSERT_EQ(str, value_to_ptr(value2))
}
END()


START(type) {
	uint64_t value = number_to_value(3.0);
	ASSERT_EQ(IS_PTR(value), false);
	ASSERT_EQ(IS_NUMBER(value), true);
	ASSERT_EQ(IS_TRUE(value), false);
	ASSERT_EQ(IS_FALSE(value), false);
	ASSERT_EQ(IS_NIL(value), false);

	char *str = "hello";
	uint64_t value2 = ptr_to_value(str);
	ASSERT_EQ(IS_PTR(value2), true);
	ASSERT_EQ(IS_NUMBER(value2), false);
	ASSERT_EQ(IS_TRUE(value2), false);
	ASSERT_EQ(IS_FALSE(value2), false);
	ASSERT_EQ(IS_NIL(value2), false);
}
END()


START_MAIN(value) {
	RUN(conversion)
	RUN(type)
}
END_MAIN()
