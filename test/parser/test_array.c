
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


int main(int argc, char *argv[]) {
	test_pass("Definition", test_definition);
	return test_run(argc, argv);
}
