
//
//  Test Instructions
//

#include <test.h>
#include <ins.h>


// Tests creating a new instruction and fetching its arguments
void test_arguments(void) {
	Instruction arg = ins_new(MOV_LL, 1, 200, 0xfffe);
	eq_int(ins_arg(arg, 0), MOV_LL);
	eq_int(ins_arg(arg, 1), 1);
	eq_int(ins_arg(arg, 2), 200);
	eq_int(ins_arg(arg, 3), 0xfffe);
}


// Tests setting arguments on an instruction
void test_modification(void) {
	Instruction arg = ins_new(MOV_LI, 21, 42, 0xff);
	eq_int(ins_arg(arg, 2), 42);
	arg = ins_set(arg, 2, 101);
	eq_int(ins_arg(arg, 2), 101);
	arg = ins_set(arg, 1, 32);
	eq_int(ins_arg(arg, 1), 32);
}


int main(int argc, char *argv[]) {
	test_pass("Arguments", test_arguments);
	test_pass("Modification", test_modification);
	return test_run(argc, argv);
}
