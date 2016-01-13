
//
//  Bytecode Tests
//

#include "test.h"


// Tests we can correctly retrieve arguments from a bytecode instruction.
TEST(Bytecode, Arguments) {
	uint64_t arg = ins_new(MOV_LL, 1, 200, 0xfffe);
	ASSERT_EQ(ins_arg(arg, 0), 0);
	ASSERT_EQ(ins_arg(arg, 1), 1);
	ASSERT_EQ(ins_arg(arg, 2), 200);
	ASSERT_EQ(ins_arg(arg, 3), 0xfffe);
}


// Tests we can correctly set arguments after an instruction has been created.
TEST(Bytecode, Modification) {
	uint64_t arg = ins_new(MOV_LI, 21, 42, 0xff);
	ASSERT_EQ(ins_arg(arg, 2), 42);
	arg = ins_set_arg(arg, 2, 101);
	ASSERT_EQ(ins_arg(arg, 2), 101);
	arg = ins_set_arg(arg, 1, 32);
	ASSERT_EQ(ins_arg(arg, 1), 32);
}
