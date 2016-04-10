
//
//  Expression Tests
//

#include <mock_parser.h>
#include <test.h>
#include <value.h>


// Tests assigning to new locals inside a block scope
void test_assign(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = 'hello'\n"
		"let d = false\n"
		"let e = nil\n"
		"let f = true\n"
		"let g = 3.141592653\n"
		"let h = 65539\n"
		"let i = a\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LS, 2, 0, 0);
	ins(&p, MOV_LP, 3, TAG_FALSE, 0);
	ins(&p, MOV_LP, 4, TAG_NIL, 0);
	ins(&p, MOV_LP, 5, TAG_TRUE, 0);
	ins(&p, MOV_LN, 6, 0, 0);
	ins(&p, MOV_LN, 7, 1, 0);
	ins(&p, MOV_LL, 8, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests reassigning to existing locals inside a block scope
void test_reassign(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"a = 1\n"
		"b = 2\n"
		"b = 'hello'\n"
		"let c = b\n"
		"a = 9\n"
		"c = a\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LI, 0, 1, 0);
	ins(&p, MOV_LI, 1, 2, 0);
	ins(&p, MOV_LS, 1, 0, 0);
	ins(&p, MOV_LL, 2, 1, 0);
	ins(&p, MOV_LI, 0, 9, 0);
	ins(&p, MOV_LL, 2, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests assigning to top level variables
void test_top_level_assign(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"let b = 4\n"
		"let c = 'hello'\n"
		"let d = false\n"
		"let e = nil\n"
		"let f = true\n"
		"let g = 3.141592653\n"
		"let h = 65539\n"
		"let i = a\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_TI, 1, 4, 0);
	ins(&p, MOV_TS, 2, 0, 0);
	ins(&p, MOV_TP, 3, TAG_FALSE, 0);
	ins(&p, MOV_TP, 4, TAG_NIL, 0);
	ins(&p, MOV_TP, 5, TAG_TRUE, 0);
	ins(&p, MOV_TN, 6, 0, 0);
	ins(&p, MOV_TN, 7, 1, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_TL, 8, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests reassigning to top level variables
void test_top_level_reassign(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"let b = 4\n"
		"a = 1\n"
		"b = 2\n"
		"b = 'hello'\n"
		"let c = b\n"
		"a = 9\n"
		"c = a\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_TI, 1, 4, 0);
	ins(&p, MOV_TI, 0, 1, 0);
	ins(&p, MOV_TI, 1, 2, 0);
	ins(&p, MOV_TS, 1, 0, 0);
	ins(&p, MOV_LT, 0, 1, 0);
	ins(&p, MOV_TL, 2, 0, 0);
	ins(&p, MOV_TI, 0, 9, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, MOV_TL, 2, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests single operations
void test_operations(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a + b\n"
		"let d = a * c\n"
		"let e = 3 - a\n"
		"let f = a - 3\n"
		"let g = 5 / b\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, ADD_LL, 2, 0, 1);
	ins(&p, MUL_LL, 3, 0, 2);
	ins(&p, SUB_IL, 4, 3, 0);
	ins(&p, SUB_LI, 5, 0, 3);
	ins(&p, DIV_IL, 6, 5, 1);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests operator precedence
void test_precedence(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = 5\n"
		"let d = a * b + c\n"
		"let e = a + b * c\n"
		"let f = a * b + c * d\n"
		"let g = a * b * c\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LI, 2, 5, 0);

	// a * b + c
	ins(&p, MUL_LL, 3, 0, 1);
	ins(&p, ADD_LL, 3, 3, 2);

	// a + b * c
	ins(&p, MUL_LL, 5, 1, 2);
	ins(&p, ADD_LL, 4, 0, 5);

	// a * b + c * d
	ins(&p, MUL_LL, 5, 0, 1);
	ins(&p, MUL_LL, 6, 2, 3);
	ins(&p, ADD_LL, 5, 5, 6);

	// a * b * c
	ins(&p, MUL_LL, 6, 0, 1);
	ins(&p, MUL_LL, 6, 6, 2);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests parentheses in expressions to override operator precedence
void test_parentheses(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = (a + b) * a\n"
		"let d = (a + b) * (c + a)\n"
		"let e = (a + b) * (c + a) * (b + a)\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);

	// (a + b) * a
	ins(&p, ADD_LL, 2, 0, 1);
	ins(&p, MUL_LL, 2, 2, 0);

	// (a + b) * (c + a)
	ins(&p, ADD_LL, 3, 0, 1);
	ins(&p, ADD_LL, 4, 2, 0);
	ins(&p, MUL_LL, 3, 3, 4);

	// (a + b) * (c + a) * (b + a)
	ins(&p, ADD_LL, 4, 0, 1);
	ins(&p, ADD_LL, 5, 2, 0);
	ins(&p, MUL_LL, 4, 4, 5);
	ins(&p, ADD_LL, 5, 1, 0);
	ins(&p, MUL_LL, 4, 4, 5);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests the unary negation operator
void test_negation(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = -3\n"
		"let b = -(3 + 8 - 2)\n"
		"let c = -a\n"
		"let d = -a + b\n"
		"let e = b * -a + c\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, signed_to_unsigned(-3), 0);
	ins(&p, MOV_LI, 1, signed_to_unsigned(-9), 0);
	ins(&p, NEG_L, 2, 0, 0);

	ins(&p, NEG_L, 3, 0, 0);
	ins(&p, ADD_LL, 3, 3, 1);

	ins(&p, NEG_L, 5, 0, 0);
	ins(&p, MUL_LL, 4, 1, 5);
	ins(&p, ADD_LL, 4, 4, 2);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}



// Tests conditional operations when assigning to variables
void test_conditional(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == b\n"
		"let d = a < b\n"
		"let e = b >= c\n"
		"let f = a != c\n"
		"let g = a == 3\n"
		"let h = 3 == a\n"
		"let i = 3 > a\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);

	// a == b
	ins(&p, NEQ_LL, 0, 1, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 2, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 2, TAG_FALSE, 0);

	// a < b
	ins(&p, GE_LL, 0, 1, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 3, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 3, TAG_FALSE, 0);

	// b >= c
	ins(&p, LT_LL, 1, 2, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 4, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 4, TAG_FALSE, 0);

	// a != c
	ins(&p, EQ_LL, 0, 2, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 5, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 5, TAG_FALSE, 0);

	// a == 3
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 6, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 6, TAG_FALSE, 0);

	// 3 == a
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 7, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 7, TAG_FALSE, 0);

	// 3 > a
	ins(&p, GT_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 8, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 8, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests combining conditionals using only `and` operators
void test_and(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == 3 && b == 4\n"
		"let d = a == 3 && b == 4 && c == 5\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);

	// a == 3 && b == 4
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 2, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 2, TAG_FALSE, 0);

	// a == 3 && b == 4 && c == 5
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 7);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 3, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 3, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests combining conditionals using only `or` operators
void test_or(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == 3 || b == 4\n"
		"let d = a == 3 || b == 4 || c == 5\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);

	// a == 3 || b == 4
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 2, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 2, TAG_FALSE, 0);

	// a == 3 || b == 4 || c == 5
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 5);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 3, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 3, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests `and` and `or` operations where one of the two arguments is a jump
// list
void test_single_jump_list(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = 5\n"
		"let d = a == 3 && b == 4 || c == 5\n"
		"let e = a == 3 || b == 4 && c == 5\n"
		"let f = a == 3 && (b == 4 || c == 5)\n"
		"let g = (a == 3 || b == 4) && c == 5\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LI, 2, 5, 0);

	// a == 3 && b == 4 || c == 5
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 3, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 3, TAG_FALSE, 0);

	// a == 3 || b == 4 && c == 5
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 4, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 4, TAG_FALSE, 0);

	// a == 3 && (b == 4 || c == 5)
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 7);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 5, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 5, TAG_FALSE, 0);

	// (a == 3 || b == 4) && c == 5
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 6, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 6, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests `or` operations where both arguments are jump lists
void test_or_jump_list(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = 5\n"
		"let d = 6\n"
		"let e = (a == 3 && b == 4) || (c == 5 && d == 6)\n"
		"let f = (a == 3 || b == 4) || (c == 5 && d == 6)\n"
		"let g = (a == 3 && b == 4) || (c == 5 || d == 6)\n"
		"let h = (a == 3 || b == 4) || (c == 5 || d == 6)\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LI, 2, 5, 0);
	ins(&p, MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) || (c == 5 && d == 6)
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 4, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 4, TAG_FALSE, 0);

	// (a == 3 || b == 4) || (c == 5 && d == 6)
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 7);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 5, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 5, TAG_FALSE, 0);

	// (a == 3 && b == 4) || (c == 5 || d == 6)
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, EQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 6, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 6, TAG_FALSE, 0);

	// (a == 3 || b == 4) || (c == 5 || d == 6)
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 7);
	ins(&p, EQ_LI, 1, 4, 0);
	jmp(&p, 5);
	ins(&p, EQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 7, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 7, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests `and` operations where both arguments are jump lists
void test_and_jump_list(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = 5\n"
		"let d = 6\n"
		"let e = (a == 3 && b == 4) && (c == 5 && d == 6)\n"
		"let f = (a == 3 || b == 4) && (c == 5 && d == 6)\n"
		"let g = (a == 3 && b == 4) && (c == 5 || d == 6)\n"
		"let h = (a == 3 || b == 4) && (c == 5 || d == 6)\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 1, 4, 0);
	ins(&p, MOV_LI, 2, 5, 0);
	ins(&p, MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) && (c == 5 && d == 6)
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 9);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 7);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 4, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 4, TAG_FALSE, 0);

	// (a == 3 || b == 4) && (c == 5 && d == 6)
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 7);
	ins(&p, NEQ_LI, 2, 5, 0);
	jmp(&p, 5);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 5, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 5, TAG_FALSE, 0);

	// (a == 3 && b == 4) && (c == 5 || d == 6)
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 9);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 7);
	ins(&p, EQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 6, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 6, TAG_FALSE, 0);

	// (a == 3 || b == 4) && (c == 5 || d == 6)
	ins(&p, EQ_LI, 0, 3, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 1, 4, 0);
	jmp(&p, 7);
	ins(&p, EQ_LI, 2, 5, 0);
	jmp(&p, 3);
	ins(&p, NEQ_LI, 3, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LP, 7, TAG_TRUE, 0);
	jmp(&p, 2);
	ins(&p, MOV_LP, 7, TAG_FALSE, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Local assignment", test_assign);
	test_pass("Local reassignment", test_reassign);
	test_pass("Top level assignment", test_top_level_assign);
	test_pass("Top level reassignment", test_top_level_reassign);
	test_pass("Operations", test_operations);
	test_pass("Operator precedence", test_precedence);
	test_pass("Parentheses", test_parentheses);
	test_pass("Negation", test_negation);
	test_pass("Conditionals", test_conditional);
	test_pass("Boolean and", test_and);
	test_pass("Boolean or", test_or);
	test_pass("Single jump list in conditionals", test_single_jump_list);
	test_pass("Or conditionals with jump lists", test_or_jump_list);
	test_pass("And conditionals with jump lists", test_and_jump_list);
	return test_run(argc, argv);
}
