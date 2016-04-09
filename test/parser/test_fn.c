
//
//  Function Tests
//

#include "test.h"


// Tests we can define a function with no arguments or return values
TEST(Functions, Definition) {
	COMPILER(
		"fn test() {\n"
		"	let a = 1\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can define a function with one argument, and use that argument in
// an expression
TEST(Functions, SingleArgument) {
	COMPILER(
		"fn test(arg1) {\n"
		"	let a = arg1 + 1\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LI, 1, 0, 1);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can define a function with more than one argument
TEST(Functions, MultipleArguments) {
	COMPILER(
		"fn test(arg1, arg2) {\n"
		"	let a = arg1 + arg2\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LL, 2, 0, 1);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests a function can contain a return statement that has no return value
TEST(Functions, ReturnNothing) {
	COMPILER(
		"fn test() {\n"
		"	let a = 3\n"
		"	if a == 3 {\n"
		"		return\n"
		"	}\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 0, 3, 0);
	INS(NEQ_LI, 0, 3, 0);
	JMP(2);
	INS(RET0, 0, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can call a function with a return statement that has an expression
// following it (indicating a return value)
TEST(Functions, ReturnValue) {
	COMPILER(
		"fn test() {\n"
		"	let a = 3\n"
		"	return a + 3\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 0, 3, 0);
	INS(ADD_LI, 1, 0, 3);
	INS(RET_L, 0, 1, 0);

	FREE();
}


// Tests we can call a function that has both multiple arguments and a return
// value
TEST(Functions, ArgumentsAndReturn) {
	COMPILER(
		"fn test(arg1, arg2) {\n"
		"	return arg1 * arg2 * 2\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MUL_LL, 2, 0, 1);
	INS(MUL_LI, 2, 2, 2);
	INS(RET_L, 0, 2, 0);

	FREE();
}


// Tests we can call a function
TEST(Functions, Call) {
	COMPILER(
		"fn test() {\n"
		"	let a = 1\n"
		"}\n"
		"test()\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(CALL, 0, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can call a function with an argument
TEST(Functions, CallWithArgument) {
	COMPILER(
		"fn test(arg1) {\n"
		"	let a = arg1\n"
		"}\n"
		"test(2)"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_LI, 1, 2, 0);
	INS(CALL, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can call a function with multiple arguments
TEST(Functions, CallWithMultipleArguments) {
	COMPILER(
		"fn test(arg1, arg2, arg3) {\n"
		"	let a = arg1 + arg2 + arg3\n"
		"}\n"
		"test(1, 2, 3)\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_LI, 1, 1, 0);
	INS(MOV_LI, 2, 2, 0);
	INS(MOV_LI, 3, 3, 0);
	INS(CALL, 0, 3, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LL, 3, 0, 1);
	INS(ADD_LL, 3, 3, 2);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests we can call a function with a return value and save it into a local
TEST(Functions, CallWithReturnValue) {
	COMPILER(
		"fn test() {\n"
		"	return 3\n"
		"}\n"
		"let a = test() * 2\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(CALL, 0, 0, 0);
	INS(MUL_LI, 0, 0, 2);
	INS(MOV_TL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(RET_I, 0, 3, 0);

	FREE();
}


// Tests we can define two functions
TEST(Functions, MultipleDefinitions) {
	COMPILER(
		"fn square(num) {\n"
		"	return num * num\n"
		"}\n"
		"fn mul(num, other) {\n"
		"	return num * other\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_TF, 1, 2, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MUL_LL, 1, 0, 0);
	INS(RET_L, 0, 1, 0);

	FN(2);
	INS(MUL_LL, 2, 0, 1);
	INS(RET_L, 0, 2, 0);

	FREE();
}


// Tests we can define functions on the stack
TEST(Functions, Stack) {
	COMPILER(
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

	FN(0);
	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LF, 2, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 1, 11, 0);
	INS(MOV_LL, 2, 0, 0);
	INS(ADD_LL, 3, 1, 2);
	INS(RET_L, 0, 3, 0);

	FREE();
}


// Tests we can have a function call's return value as an argument to another
// function call
TEST(Functions, CallAsArgument) {
	COMPILER(
		"fn test(arg) {\n"
		"	return arg + 1\n"
		"}\n"
		"let a = test(test(1))\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_LT, 1, 0, 0);
	INS(MOV_LI, 2, 1, 0);
	INS(CALL, 1, 1, 1);
	INS(CALL, 0, 1, 0);
	INS(MOV_TL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LI, 1, 0, 1);
	INS(RET_L, 0, 1, 0);

	FREE();
}


// Tests defining and calling an anonymous function
TEST(Functions, AnonymousFunction) {
	COMPILER(
		"let test = fn(arg1, arg2) {\n"
		"	return arg1 + arg2\n"
		"}\n"
		"test(1, 2)\n"
	);

	FN(0);
	INS(MOV_TF, 0, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_LI, 1, 1, 0);
	INS(MOV_LI, 2, 2, 0);
	INS(CALL, 0, 2, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LL, 2, 0, 1);
	INS(RET_L, 0, 2, 0);

	FREE();
}


// Tests we can call an anonymous function without saving the function into a
// local first
TEST(Functions, CallAnonymousFunction) {
	COMPILER(
		"let tree = (fn(arg1, arg2) {\n"
		"	return arg1 + arg2\n"
		"})(1, 2)\n"
	);

	FN(0);
	INS(MOV_LF, 1, 1, 0);
	INS(MOV_LI, 2, 1, 0);
	INS(MOV_LI, 3, 2, 0);
	INS(CALL, 1, 2, 0);
	INS(MOV_TL, 0, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(ADD_LL, 2, 0, 1);
	INS(RET_L, 0, 2, 0);

	FREE();
}


// Tests overriding top level variables in function arguments and locals
TEST(Functions, OverrideTopLevel) {
	COMPILER(
		"let a = 3\n"
		"let b = 4\n"
		"fn test(a) {\n"
		"	let b = a\n"
		"}\n"
	);

	FN(0);
	INS(MOV_TI, 0, 3, 0);
	INS(MOV_TI, 1, 4, 0);
	INS(MOV_TF, 2, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}
