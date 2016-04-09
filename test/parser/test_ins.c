
//
//  Test Instructions
//

extern "C" {
#include <ins.h>
}

#include <gtest/gtest.h>


// Tests creating a new instruction and fetching its arguments
TEST(Bytecode, Arguments) {
	Instruction arg = ins_new(MOV_LL, 1, 200, 0xfffe);
	ASSERT_EQ(ins_arg(arg, 0), MOV_LL);
	ASSERT_EQ(ins_arg(arg, 1), 1);
	ASSERT_EQ(ins_arg(arg, 2), 200);
	ASSERT_EQ(ins_arg(arg, 3), 0xfffe);
}


// Tests setting arguments on an instruction
TEST(Bytecode, Modification) {
	Instruction arg = ins_new(MOV_LI, 21, 42, 0xff);
	ASSERT_EQ(ins_arg(arg, 2), 42);
	arg = ins_set(arg, 2, 101);
	ASSERT_EQ(ins_arg(arg, 2), 101);
	arg = ins_set(arg, 1, 32);
	ASSERT_EQ(ins_arg(arg, 1), 32);
}
