
//
//  Infinite Loop Tests
//

#include <mock_parser.h>
#include <test.h>


// Tests an infinite loop
void test_loop(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"loop {\n"
		"	a = a + 1\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, LOOP, 3, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests breaking from within an infinite loop
void test_break(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"loop {\n"
		"	a = a + 1\n"
		"	if a == 10 {\n"
		"		break\n"
		"	}\n"
		"}"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, NEQ_LI, 0, 10, 0);
	jmp(&p, 2);
	jmp(&p, 2);
	ins(&p, LOOP, 7, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Infinite loop", test_loop);
	test_pass("Break", test_break);
	return test_run(argc, argv);
}
