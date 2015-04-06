
//
//  Value Test
//


#include "test.h"

#include "../src/value.h"


START(conversion) {
	uint64_t value = as_value(3.0);
	double converted = as_number(value);
	ASSERT_EQ(converted, 3.0);
}
END()


MAIN(value) {
	RUN(conversion)
}
MAIN_END()
