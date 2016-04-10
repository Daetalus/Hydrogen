
//
//  While Loop Tests
//

#include <mock_parser.h>
#include <test.h>


// Tests a single while loop
void test_single(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"while a < 100 {\n"
		"	a = a + 1\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, GE_LI, 0, 100, 0);
	jmp(&p, 5);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, LOOP, 6, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests a break statement from within a while loop
void test_break(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"while a < 1000 {\n"
		"	a = a + 1\n"
		"	if a == 100 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, GE_LI, 0, 1000, 0);
	jmp(&p, 6);

	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, NEQ_LI, 0, 100, 0);
	jmp(&p, 2);
	jmp(&p, 2);
	ins(&p, LOOP, 6, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests two nested while loops
void test_nested(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"	}\n"
		"	a = a + 1\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, GE_LI, 0, 100, 0);
	jmp(&p, 8);

	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, GE_LI, 1, 100, 0);
	jmp(&p, 3);
	ins(&p, ADD_LI, 1, 1, 1);
	ins(&p, LOOP, 3, 0, 0);

	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, LOOP, 8, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests a break statement from within a nested while loop
void test_nested_break(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"while a < 100 {\n"
		"	let b = 4\n"
		"	while b < 100 {\n"
		"		b = b + 1\n"
		"		if b == 10 {\n"
		"			break\n"
		"		}\n"
		"	}\n"
		"	a = a + 1\n"
		"	if a == 20 {\n"
		"		break\n"
		"	}\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, GE_LI, 0, 100, 0);
	jmp(&p, 14);

	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, GE_LI, 1, 100, 0);
	jmp(&p, 6);
	ins(&p, ADD_LI, 1, 1, 1);
	ins(&p, NEQ_LI, 1, 10, 0);
	jmp(&p, 2);
	jmp(&p, 2);
	ins(&p, LOOP, 6, 0, 0);

	ins(&p, ADD_LI, 0, 0, 1);
	ins(&p, NEQ_LI, 0, 20, 0);
	jmp(&p, 2);
	jmp(&p, 2);
	ins(&p, LOOP, 14, 0, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Single loop", test_single);
	test_pass("Break", test_break);
	test_pass("Nested loops", test_nested);
	test_pass("Break from nested loops", test_nested_break);
	return test_run(argc, argv);
}
