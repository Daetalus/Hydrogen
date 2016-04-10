
//
//  Function Tests
//

#include <mock_parser.h>
#include <test.h>


// Tests we can define a function with no arguments or return values
void test_definition(void) {
	MockParser p = mock_parser(
		"fn test() {\n"
		"	let a = 1\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can define a function with one argument, and use that argument in
// an expression
void test_single_argument(void) {
	MockParser p = mock_parser(
		"fn test(arg1) {\n"
		"	let a = arg1 + 1\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LI, 1, 0, 1);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can define a function with more than one argument
void test_multiple_arguments(void) {
	MockParser p = mock_parser(
		"fn test(arg1, arg2) {\n"
		"	let a = arg1 + arg2\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LL, 2, 0, 1);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests a function can contain a return statement that has no return value
void test_return_nothing(void) {
	MockParser p = mock_parser(
		"fn test() {\n"
		"	let a = 3\n"
		"	if a == 3 {\n"
		"		return\n"
		"	}\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 2);
	ins(&p, RET0, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can call a function with a return statement that has an expression
// following it (indicating a return value)
void test_return_something(void) {
	MockParser p = mock_parser(
		"fn test() {\n"
		"	let a = 3\n"
		"	return a + 3\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, ADD_LI, 1, 0, 3);
	ins(&p, RET_L, 0, 1, 0);

	mock_parser_free(&p);
}


// Tests we can call a function that has both multiple arguments and a return
// value
void test_arguments_in_return(void) {
	MockParser p = mock_parser(
		"fn test(arg1, arg2) {\n"
		"	return arg1 * arg2 * 2\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MUL_LL, 2, 0, 1);
	ins(&p, MUL_LI, 2, 2, 2);
	ins(&p, RET_L, 0, 2, 0);

	mock_parser_free(&p);
}


// Tests we can call a function
void test_call(void) {
	MockParser p = mock_parser(
		"fn test() {\n"
		"	let a = 1\n"
		"}\n"
		"test()\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, CALL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can call a function with an argument
void test_call_single_argument(void) {
	MockParser p = mock_parser(
		"fn test(arg1) {\n"
		"	let a = arg1\n"
		"}\n"
		"test(2)"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_LI, 1, 2, 0);
	ins(&p, CALL, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can call a function with multiple arguments
void test_call_multiple_arguments(void) {
	MockParser p = mock_parser(
		"fn test(arg1, arg2, arg3) {\n"
		"	let a = arg1 + arg2 + arg3\n"
		"}\n"
		"test(1, 2, 3)\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_LI, 1, 1, 0);
	ins(&p, MOV_LI, 2, 2, 0);
	ins(&p, MOV_LI, 3, 3, 0);
	ins(&p, CALL, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LL, 3, 0, 1);
	ins(&p, ADD_LL, 3, 3, 2);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests we can call a function with a return value and save it into a local
void test_call_return(void) {
	MockParser p = mock_parser(
		"fn test() {\n"
		"	return 3\n"
		"}\n"
		"let a = test() * 2\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, CALL, 0, 0, 0);
	ins(&p, MUL_LI, 0, 0, 2);
	ins(&p, MOV_TL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, RET_I, 0, 3, 0);

	mock_parser_free(&p);
}


// Tests we can define two functions
void test_multiple_definitions(void) {
	MockParser p = mock_parser(
		"fn square(num) {\n"
		"	return num * num\n"
		"}\n"
		"fn mul(num, other) {\n"
		"	return num * other\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_TF, 1, 2, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MUL_LL, 1, 0, 0);
	ins(&p, RET_L, 0, 1, 0);

	switch_fn(&p, 2);
	ins(&p, MUL_LL, 2, 0, 1);
	ins(&p, RET_L, 0, 2, 0);

	mock_parser_free(&p);
}


// Tests we can define functions on the stack
void test_stack(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"fn test(arg) {\n"
		"	let a = 11\n"
		"	let b = arg\n"
		"	return a + b\n"
		"}\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LF, 2, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 1, 11, 0);
	ins(&p, MOV_LL, 2, 0, 0);
	ins(&p, ADD_LL, 3, 1, 2);
	ins(&p, RET_L, 0, 3, 0);

	mock_parser_free(&p);
}


// Tests we can have a function call's return value as an argument to another
// function call
void test_nested_calls(void) {
	MockParser p = mock_parser(
		"fn test(arg) {\n"
		"	return arg + 1\n"
		"}\n"
		"let a = test(test(1))\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_LT, 1, 0, 0);
	ins(&p, MOV_LI, 2, 1, 0);
	ins(&p, CALL, 1, 1, 1);
	ins(&p, CALL, 0, 1, 0);
	ins(&p, MOV_TL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LI, 1, 0, 1);
	ins(&p, RET_L, 0, 1, 0);

	mock_parser_free(&p);
}


// Tests defining and calling an anonymous function
void test_anonymous_function(void) {
	MockParser p = mock_parser(
		"let test = fn(arg1, arg2) {\n"
		"	return arg1 + arg2\n"
		"}\n"
		"test(1, 2)\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TF, 0, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_LI, 1, 1, 0);
	ins(&p, MOV_LI, 2, 2, 0);
	ins(&p, CALL, 0, 2, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LL, 2, 0, 1);
	ins(&p, RET_L, 0, 2, 0);

	mock_parser_free(&p);
}


// Tests we can call an anonymous function without saving the function into a
// local first
void test_call_anonymous_function(void) {
	MockParser p = mock_parser(
		"let tree = (fn(arg1, arg2) {\n"
		"	return arg1 + arg2\n"
		"})(1, 2)\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_LF, 1, 1, 0);
	ins(&p, MOV_LI, 2, 1, 0);
	ins(&p, MOV_LI, 3, 2, 0);
	ins(&p, CALL, 1, 2, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, ADD_LL, 2, 0, 1);
	ins(&p, RET_L, 0, 2, 0);

	mock_parser_free(&p);
}


// Tests overriding top level variables in function arguments and locals
void test_override_top_level(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"let b = 4\n"
		"fn test(a) {\n"
		"	let b = a\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_TI, 1, 4, 0);
	ins(&p, MOV_TF, 2, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Defining", test_definition);
	test_pass("Single argument", test_single_argument);
	test_pass("Multiple arguments", test_multiple_arguments);
	test_pass("Return nothing", test_return_nothing);
	test_pass("Return something", test_return_something);
	test_pass("Arguments in return", test_arguments_in_return);
	test_pass("Function call", test_call);
	test_pass("Call with one argument", test_call_single_argument);
	test_pass("Call with multiple arguments", test_call_multiple_arguments);
	test_pass("Call with return value", test_call_return);
	test_pass("Defining multiple functions", test_multiple_definitions);
	test_pass("Defining on stack", test_stack);
	test_pass("Nested calls", test_nested_calls);
	test_pass("Anonymous function", test_anonymous_function);
	test_pass("Call anonymous function", test_call_anonymous_function);
	test_pass("Override top level in arguments", test_override_top_level);
	return test_run(argc, argv);
}
