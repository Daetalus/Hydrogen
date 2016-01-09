
//
//  Bytecode Tests
//

#include "test.h"


// Tests we can correctly retrieve arguments from a bytecode instruction.
TEST(Bytecode, Arguments) {
	uint64_t arg = instr_new(MOV_LL, 1, 200, 0xfffe);
	ASSERT_EQ(instr_argument(arg, 0), 0);
	ASSERT_EQ(instr_argument(arg, 1), 1);
	ASSERT_EQ(instr_argument(arg, 2), 200);
	ASSERT_EQ(instr_argument(arg, 3), 0xfffe);
}


// Tests we can correctly set arguments after an instruction has been created.
TEST(Bytecode, Modification) {
	uint64_t arg = instr_new(MOV_LI, 21, 42, 0xff);
	ASSERT_EQ(instr_argument(arg, 2), 42);
	arg = instr_modify_argument(arg, 2, 101);
	ASSERT_EQ(instr_argument(arg, 2), 101);
	arg = instr_modify_argument(arg, 1, 32);
	ASSERT_EQ(instr_argument(arg, 1), 32);
}
