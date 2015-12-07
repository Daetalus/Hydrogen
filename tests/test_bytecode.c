
//
//  Bytecode Tests
//

#include "test.h"


TEST(arguments) {
	uint64_t arg = instr_new(0, 1, 200, 0xfffe);
	EQ(instr_argument(arg, 0), 0);
	EQ(instr_argument(arg, 1), 1);
	EQ(instr_argument(arg, 2), 200);
	EQ(instr_argument(arg, 3), 0xfffe);
}


TEST(modification) {
	uint64_t arg = instr_new(10, 21, 42, 0xff);
	EQ(instr_argument(arg, 2), 42);
	arg = instr_modify_argument(arg, 2, 101);
	EQ(instr_argument(arg, 2), 101);
	arg = instr_modify_argument(arg, 1, 32);
	EQ(instr_argument(arg, 1), 32);
}


MAIN() {
	RUN(arguments);
	RUN(modification);
}
