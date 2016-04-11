
//
//  Array Tests
//

#include <mock_parser.h>
#include <test.h>


// Tests defining an array
void test_definition(void) {
	MockParser p = mock_parser(
		"let a = [1, 'hello', 58, fn(hello, there) {let c = hello},]"
	);

	switch_fn(&p, 0);
	ins(&p, ARRAY_NEW, 0, 0, 0);
	ins(&p, ARRAY_I_SET_I, 0, 1, 0);
	ins(&p, ARRAY_I_SET_S, 1, 0, 0);
	ins(&p, ARRAY_I_SET_I, 2, 58, 0);
	ins(&p, ARRAY_I_SET_F, 3, 1, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LL, 2, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests nested definitions of arrays
void test_nested_definitions(void) {
	MockParser p = mock_parser(
		"let a = [1, [1, 2, []], 3]"
	);

	ins(&p, ARRAY_NEW, 0, 0, 0);
	ins(&p, ARRAY_I_SET_I, 0, 1, 0);

	ins(&p, ARRAY_NEW, 1, 0, 0);
	ins(&p, ARRAY_I_SET_I, 0, 1, 1);
	ins(&p, ARRAY_I_SET_I, 1, 2, 1);
	ins(&p, ARRAY_NEW, 2, 0, 0);
	ins(&p, ARRAY_I_SET_L, 2, 2, 1);
	ins(&p, ARRAY_I_SET_L, 1, 1, 0);

	ins(&p, ARRAY_I_SET_I, 2, 3, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests accessing elements from an array
void test_access(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = [1, 2, 3, 4]\n"
		"let b = a[0]\n"
		"let c = a[3]\n"
		"let d = 2\n"
		"let e = a[d]\n"
		"let f = a[d / 2 + 53 - d * 2]\n"
		"}\n"
	);

	ins(&p, ARRAY_NEW, 0, 0, 0);
	ins(&p, ARRAY_I_SET_I, 0, 1, 0);
	ins(&p, ARRAY_I_SET_I, 1, 2, 0);
	ins(&p, ARRAY_I_SET_I, 2, 3, 0);
	ins(&p, ARRAY_I_SET_I, 3, 4, 0);

	ins(&p, ARRAY_GET_I, 1, 0, 0);
	ins(&p, ARRAY_GET_I, 2, 3, 0);
	ins(&p, MOV_LI, 3, 2, 0);
	ins(&p, ARRAY_GET_L, 4, 3, 0);
	ins(&p, DIV_LI, 6, 3, 2);
	ins(&p, ADD_LI, 6, 6, 53);
	ins(&p, MUL_LI, 7, 3, 2);
	ins(&p, SUB_LL, 6, 6, 7);
	ins(&p, ARRAY_GET_L, 5, 6, 0);

	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests nested array element access
void test_nested_access(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = []\n"
		"let b = []\n"
		"let c = a[b[0]]\n"
		"let d = a[0][1]\n"
		"let e = a[0][1][2]\n"
		"}\n"
	);

	ins(&p, ARRAY_NEW, 0, 0, 0);
	ins(&p, ARRAY_NEW, 1, 0, 0);

	ins(&p, ARRAY_GET_I, 3, 0, 1);
	ins(&p, ARRAY_GET_L, 2, 3, 0);

	ins(&p, ARRAY_GET_I, 3, 0, 0);
	ins(&p, ARRAY_GET_I, 3, 1, 3);

	ins(&p, ARRAY_GET_I, 4, 0, 0);
	ins(&p, ARRAY_GET_I, 4, 1, 4);
	ins(&p, ARRAY_GET_I, 4, 2, 4);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Definition", test_definition);
	test_pass("Nested definitions", test_nested_definitions);
	test_pass("Element access", test_access);
	test_pass("Nested element access", test_nested_access);
	return test_run(argc, argv);
}
