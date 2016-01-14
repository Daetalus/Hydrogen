
//
//  Expression Tests
//

#include "test.h"


// Tests we can perform simple operations.
TEST(Expression, SingleOperators) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a + b\n"
		"let d = a * c\n"
		"let e = a - 3\n"
		"let f = 5 / b\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(ADD_LL, 2, 0, 1);
	ASSERT_INSTR(MUL_LL, 3, 0, 2);
	ASSERT_INSTR(SUB_LI, 4, 0, 3);
	ASSERT_INSTR(DIV_IL, 5, 5, 1);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests the compiler obeys operator precedence.
TEST(Expression, OperatorPrecedence) {
	COMPILER(
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(MOV_LI, 2, 5, 0);

	// a * b + c
	ASSERT_INSTR(MUL_LL, 3, 0, 1);
	ASSERT_INSTR(ADD_LL, 3, 3, 2);

	// a + b * c
	ASSERT_INSTR(MUL_LL, 5, 1, 2);
	ASSERT_INSTR(ADD_LL, 4, 0, 5);

	// a * b + c * d
	ASSERT_INSTR(MUL_LL, 5, 0, 1);
	ASSERT_INSTR(MUL_LL, 6, 2, 3);
	ASSERT_INSTR(ADD_LL, 5, 5, 6);

	// a * b * c
	ASSERT_INSTR(MUL_LL, 6, 0, 1);
	ASSERT_INSTR(MUL_LL, 6, 6, 2);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests parentheses in overriding operator precedence.
TEST(Expression, Parentheses) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = (a + b) * a\n"
		"let d = (a + b) * (c + a)\n"
		"let e = (a + b) * (c + a) * (b + a)\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);

	// (a + b) * a
	ASSERT_INSTR(ADD_LL, 2, 0, 1);
	ASSERT_INSTR(MUL_LL, 2, 2, 0);

	// (a + b) * (c + a)
	ASSERT_INSTR(ADD_LL, 3, 0, 1);
	ASSERT_INSTR(ADD_LL, 4, 2, 0);
	ASSERT_INSTR(MUL_LL, 3, 3, 4);

	// (a + b) * (c + a) * (b + a)
	ASSERT_INSTR(ADD_LL, 4, 0, 1);
	ASSERT_INSTR(ADD_LL, 5, 2, 0);
	ASSERT_INSTR(MUL_LL, 4, 4, 5);
	ASSERT_INSTR(ADD_LL, 5, 1, 0);
	ASSERT_INSTR(MUL_LL, 4, 4, 5);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests the unary negation operator.
TEST(Expression, Negation) {
	COMPILER(
		"{\n"
		"let a = -3\n"
		"let b = -(3 + 8 - 2)\n"
		"let c = -a\n"
		"let d = -a + b\n"
		"let e = b * -a + c\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, TO_UNSIGNED(-3), 0);
	ASSERT_INSTR(MOV_LI, 1, TO_UNSIGNED(-9), 0);
	ASSERT_INSTR(NEG_L, 2, 0, 0);

	ASSERT_INSTR(NEG_L, 3, 0, 0);
	ASSERT_INSTR(ADD_LL, 3, 3, 1);

	ASSERT_INSTR(NEG_L, 5, 0, 0);
	ASSERT_INSTR(MUL_LL, 4, 1, 5);
	ASSERT_INSTR(ADD_LL, 4, 4, 2);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests conditional operations when assigning to variables.
TEST(Expression, Conditional) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == b\n"
		"let d = a < b\n"
		"let e = b >= c\n"
		"let f = a != c\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);

	// a == b
	ASSERT_INSTR(NEQ_LL, 0, 1, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 2, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 2, FALSE_TAG, 0);

	// a < b
	ASSERT_INSTR(GE_LL, 0, 1, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 3, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 3, FALSE_TAG, 0);

	// b >= c
	ASSERT_INSTR(LT_LL, 1, 2, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 4, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 4, FALSE_TAG, 0);

	// a != c
	ASSERT_INSTR(EQ_LL, 0, 2, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 5, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 5, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests combining conditionals using only `and` operators.
TEST(Expression, And) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == 3 && b == 4\n"
		"let d = a == 3 && b == 4 && c == 5\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);

	// a == 3 && b == 4
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 2, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 2, FALSE_TAG, 0);

	// a == 3 && b == 4 && c == 5
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 3, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 3, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests combining conditionals using only `or` operators.
TEST(Expression, Or) {
	COMPILER(
		"{\n"
		"let a = 3\n"
		"let b = 4\n"
		"let c = a == 3 || b == 4\n"
		"let d = a == 3 || b == 4 || c == 5\n"
		"}\n"
	);

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);

	// a == 3 || b == 4
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 2, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 2, FALSE_TAG, 0);

	// a == 3 || b == 4 || c == 5
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 3, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 3, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests `and` and `or` operations where one of the two arguments is a jump
// list.
TEST(Expression, AndOrSingleJumpList) {
	COMPILER(
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(MOV_LI, 2, 5, 0);

	// a == 3 && b == 4 || c == 5
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 3, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 3, FALSE_TAG, 0);

	// a == 3 || b == 4 && c == 5
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 4, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 4, FALSE_TAG, 0);

	// a == 3 && (b == 4 || c == 5)
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 5, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 || b == 4) && c == 5
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 6, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 6, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests `or` operations where both arguments are jump lists.
TEST(Expression, AndOrOrJumpList) {
	COMPILER(
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(MOV_LI, 2, 5, 0);
	ASSERT_INSTR(MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) || (c == 5 && d == 6)
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 4, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 4, FALSE_TAG, 0);

	// (a == 3 || b == 4) || (c == 5 && d == 6)
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 5, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 && b == 4) || (c == 5 || d == 6)
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(EQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 6, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 6, FALSE_TAG, 0);

	// (a == 3 || b == 4) || (c == 5 || d == 6)
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(EQ_LI, 1, 4, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(EQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 7, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 7, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}


// Tests `and` operations where both arguments are jump lists.
TEST(Expression, AndOrAndJumpList) {
	COMPILER(
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

	ASSERT_INSTR(MOV_LI, 0, 3, 0);
	ASSERT_INSTR(MOV_LI, 1, 4, 0);
	ASSERT_INSTR(MOV_LI, 2, 5, 0);
	ASSERT_INSTR(MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) && (c == 5 && d == 6)
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(9);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 4, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 4, FALSE_TAG, 0);

	// (a == 3 || b == 4) && (c == 5 && d == 6)
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(NEQ_LI, 2, 5, 0);
	ASSERT_JMP(5);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 5, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 && b == 4) && (c == 5 || d == 6)
	ASSERT_INSTR(NEQ_LI, 0, 3, 0);
	ASSERT_JMP(9);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(EQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 6, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 6, FALSE_TAG, 0);

	// (a == 3 || b == 4) && (c == 5 || d == 6)
	ASSERT_INSTR(EQ_LI, 0, 3, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 1, 4, 0);
	ASSERT_JMP(7);
	ASSERT_INSTR(EQ_LI, 2, 5, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(NEQ_LI, 3, 6, 0);
	ASSERT_JMP(3);
	ASSERT_INSTR(MOV_LP, 7, TRUE_TAG, 0);
	ASSERT_JMP(2);
	ASSERT_INSTR(MOV_LP, 7, FALSE_TAG, 0);

	ASSERT_RET();
	COMPILER_FREE();
}
