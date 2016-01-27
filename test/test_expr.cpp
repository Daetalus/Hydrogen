
//
//  Expression Tests
//

extern "C" {
#include <hydrogen.h>
#include <vec.h>
#include <parser.h>
#include <pkg.h>
#include <vm.h>
#include <bytecode.h>
#include <debug.h>
#include <value.h>
}

#include <gtest/gtest.h>


// Selects the function whose bytecode we are asserting.
#define FN(index)                            \
	fn = &vec_at(state->functions, (index)); \
	index = 0;


// Creates a new compiler.
#define COMPILER(code)                                               \
	char *source = (code);                                           \
	HyState *state = hy_new();                                       \
	Index pkg_index = pkg_new(state);                                \
	Package *pkg = &vec_at(state->packages, pkg_index);              \
	Index src_index = pkg_add_string(pkg, source);                   \
	Index main_fn;                                                   \
	HyError *err = pkg_parse(pkg, src_index, &main_fn);              \
	if (err != NULL) {                                               \
		FAIL() << "Compilation error: " << err->description << "\n"; \
	}                                                                \
	Function *fn;                                                    \
	uint32_t index = 0;                                              \
	FN(main_fn);


// Frees resources allocated when creating a compiler.
#define FREE() \
	hy_free(state);


// Asserts the next instruction has the opcode `opcode` and 3 arguments `arg1`,
// `arg2`, `arg3`.
#define INS(opcode, arg1, arg2, arg3) {                  \
	ASSERT_LT(index, vec_len(fn->instructions));         \
	Instruction ins = vec_at(fn->instructions, index++); \
	ASSERT_EQ(ins_arg(ins, 0), opcode);                  \
	ASSERT_EQ(ins_arg(ins, 1), arg1);                    \
	ASSERT_EQ(ins_arg(ins, 2), arg2);                    \
	ASSERT_EQ(ins_arg(ins, 3), arg3);                    \
}


// Asserts the next instruction is a jump with offset `offset`.
#define JMP(offset) {                                    \
	ASSERT_LT(index, vec_len(fn->instructions));         \
	Instruction ins = vec_at(fn->instructions, index++); \
	ASSERT_EQ(ins_arg(ins, 0), JMP);                     \
	ASSERT_EQ(ins_arg(ins, 1), offset);                  \
}


// Converts a negative value into an unsigned one in a bitwise fashion.
#define TO_UNSIGNED(value) signed_to_unsigned(value)


// Tests assigning to new locals inside a block scope.
TEST(Expression, Assign) {
	COMPILER(
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LS, 2, 0, 0);
	INS(MOV_LP, 3, FALSE_TAG, 0);
	INS(MOV_LP, 4, NIL_TAG, 0);
	INS(MOV_LP, 5, TRUE_TAG, 0);
	INS(MOV_LN, 6, 0, 0);
	INS(MOV_LN, 7, 1, 0);
	INS(MOV_LL, 8, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests reassigning to existing locals inside a block scope.
TEST(Expression, Reassign) {
	COMPILER(
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LI, 0, 1, 0);
	INS(MOV_LI, 1, 2, 0);
	INS(MOV_LS, 1, 0, 0);
	INS(MOV_LL, 2, 1, 0);
	INS(MOV_LI, 0, 9, 0);
	INS(MOV_LL, 2, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests assigning to top level variables.
TEST(Expression, TopLevelAssign) {
	COMPILER(
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

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_TI, 1, 4, 0);
	INS(MOV_TS, 2, 0, 0);
	INS(MOV_TP, 3, FALSE_TAG, 0);
	INS(MOV_TP, 4, NIL_TAG, 0);
	INS(MOV_TP, 5, TRUE_TAG, 0);
	INS(MOV_TN, 6, 0, 0);
	INS(MOV_TN, 7, 1, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_TL, 8, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests reassigning to top level variables.
TEST(Expression, TopLevelReassign) {
	COMPILER(
		"let a = 3\n"
		"let b = 4\n"
		"a = 1\n"
		"b = 2\n"
		"b = 'hello'\n"
		"let c = b\n"
		"a = 9\n"
		"c = a\n"
	);

	INS(MOV_TI, 0, 3, 0);
	INS(MOV_TI, 1, 4, 0);
	INS(MOV_TI, 0, 1, 0);
	INS(MOV_TI, 1, 2, 0);
	INS(MOV_TS, 1, 0, 0);
	INS(MOV_LT, 0, 1, 0);
	INS(MOV_TL, 2, 0, 0);
	INS(MOV_TI, 0, 9, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(MOV_TL, 2, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests single operations.
TEST(Expression, Operations) {
	COMPILER(
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(ADD_LL, 2, 0, 1);
	INS(MUL_LL, 3, 0, 2);
	INS(SUB_IL, 4, 3, 0);
	INS(SUB_LI, 5, 0, 3);
	INS(DIV_IL, 6, 5, 1);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests operator precedence.
TEST(Expression, Precedence) {
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LI, 2, 5, 0);

	// a * b + c
	INS(MUL_LL, 3, 0, 1);
	INS(ADD_LL, 3, 3, 2);

	// a + b * c
	INS(MUL_LL, 5, 1, 2);
	INS(ADD_LL, 4, 0, 5);

	// a * b + c * d
	INS(MUL_LL, 5, 0, 1);
	INS(MUL_LL, 6, 2, 3);
	INS(ADD_LL, 5, 5, 6);

	// a * b * c
	INS(MUL_LL, 6, 0, 1);
	INS(MUL_LL, 6, 6, 2);

	INS(RET0, 0, 0, 0);
	FREE();
}


// Tests parentheses in expressions to override operator precedence.
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);

	// (a + b) * a
	INS(ADD_LL, 2, 0, 1);
	INS(MUL_LL, 2, 2, 0);

	// (a + b) * (c + a)
	INS(ADD_LL, 3, 0, 1);
	INS(ADD_LL, 4, 2, 0);
	INS(MUL_LL, 3, 3, 4);

	// (a + b) * (c + a) * (b + a)
	INS(ADD_LL, 4, 0, 1);
	INS(ADD_LL, 5, 2, 0);
	INS(MUL_LL, 4, 4, 5);
	INS(ADD_LL, 5, 1, 0);
	INS(MUL_LL, 4, 4, 5);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, TO_UNSIGNED(-3), 0);
	INS(MOV_LI, 1, TO_UNSIGNED(-9), 0);
	INS(NEG_L, 2, 0, 0);

	INS(NEG_L, 3, 0, 0);
	INS(ADD_LL, 3, 3, 1);

	INS(NEG_L, 5, 0, 0);
	INS(MUL_LL, 4, 1, 5);
	INS(ADD_LL, 4, 4, 2);

	INS(RET0, 0, 0, 0);
	FREE();
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
		"let g = a == 3\n"
		"let h = 3 == a\n"
		"let i = 3 > a\n"
		"}\n"
	);

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);

	// a == b
	INS(NEQ_LL, 0, 1, 0);
	JMP(3);
	INS(MOV_LP, 2, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 2, FALSE_TAG, 0);

	// a < b
	INS(GE_LL, 0, 1, 0);
	JMP(3);
	INS(MOV_LP, 3, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 3, FALSE_TAG, 0);

	// b >= c
	INS(LT_LL, 1, 2, 0);
	JMP(3);
	INS(MOV_LP, 4, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 4, FALSE_TAG, 0);

	// a != c
	INS(EQ_LL, 0, 2, 0);
	JMP(3);
	INS(MOV_LP, 5, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 5, FALSE_TAG, 0);

	// a == 3
	INS(NEQ_LI, 0, 3, 0);
	JMP(3);
	INS(MOV_LP, 6, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 6, FALSE_TAG, 0);

	// 3 == a
	INS(NEQ_LI, 0, 3, 0);
	JMP(3);
	INS(MOV_LP, 7, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 7, FALSE_TAG, 0);

	// 3 > a
	INS(GT_LI, 0, 3, 0);
	JMP(3);
	INS(MOV_LP, 8, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 8, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);

	// a == 3 && b == 4
	INS(NEQ_LI, 0, 3, 0);
	JMP(5);
	INS(NEQ_LI, 1, 4, 0);
	JMP(3);
	INS(MOV_LP, 2, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 2, FALSE_TAG, 0);

	// a == 3 && b == 4 && c == 5
	INS(NEQ_LI, 0, 3, 0);
	JMP(7);
	INS(NEQ_LI, 1, 4, 0);
	JMP(5);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 3, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 3, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);

	// a == 3 || b == 4
	INS(EQ_LI, 0, 3, 0);
	JMP(3);
	INS(NEQ_LI, 1, 4, 0);
	JMP(3);
	INS(MOV_LP, 2, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 2, FALSE_TAG, 0);

	// a == 3 || b == 4 || c == 5
	INS(EQ_LI, 0, 3, 0);
	JMP(5);
	INS(EQ_LI, 1, 4, 0);
	JMP(3);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 3, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 3, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LI, 2, 5, 0);

	// a == 3 && b == 4 || c == 5
	INS(NEQ_LI, 0, 3, 0);
	JMP(3);
	INS(EQ_LI, 1, 4, 0);
	JMP(3);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 3, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 3, FALSE_TAG, 0);

	// a == 3 || b == 4 && c == 5
	INS(EQ_LI, 0, 3, 0);
	JMP(5);
	INS(NEQ_LI, 1, 4, 0);
	JMP(5);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 4, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 4, FALSE_TAG, 0);

	// a == 3 && (b == 4 || c == 5)
	INS(NEQ_LI, 0, 3, 0);
	JMP(7);
	INS(EQ_LI, 1, 4, 0);
	JMP(3);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 5, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 || b == 4) && c == 5
	INS(EQ_LI, 0, 3, 0);
	JMP(3);
	INS(NEQ_LI, 1, 4, 0);
	JMP(5);
	INS(NEQ_LI, 2, 5, 0);
	JMP(3);
	INS(MOV_LP, 6, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 6, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LI, 2, 5, 0);
	INS(MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) || (c == 5 && d == 6)
	INS(NEQ_LI, 0, 3, 0);
	JMP(3);
	INS(EQ_LI, 1, 4, 0);
	JMP(5);
	INS(NEQ_LI, 2, 5, 0);
	JMP(5);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 4, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 4, FALSE_TAG, 0);

	// (a == 3 || b == 4) || (c == 5 && d == 6)
	INS(EQ_LI, 0, 3, 0);
	JMP(7);
	INS(EQ_LI, 1, 4, 0);
	JMP(5);
	INS(NEQ_LI, 2, 5, 0);
	JMP(5);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 5, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 && b == 4) || (c == 5 || d == 6)
	INS(NEQ_LI, 0, 3, 0);
	JMP(3);
	INS(EQ_LI, 1, 4, 0);
	JMP(5);
	INS(EQ_LI, 2, 5, 0);
	JMP(3);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 6, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 6, FALSE_TAG, 0);

	// (a == 3 || b == 4) || (c == 5 || d == 6)
	INS(EQ_LI, 0, 3, 0);
	JMP(7);
	INS(EQ_LI, 1, 4, 0);
	JMP(5);
	INS(EQ_LI, 2, 5, 0);
	JMP(3);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 7, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 7, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
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

	INS(MOV_LI, 0, 3, 0);
	INS(MOV_LI, 1, 4, 0);
	INS(MOV_LI, 2, 5, 0);
	INS(MOV_LI, 3, 6, 0);

	// (a == 3 && b == 4) && (c == 5 && d == 6)
	INS(NEQ_LI, 0, 3, 0);
	JMP(9);
	INS(NEQ_LI, 1, 4, 0);
	JMP(7);
	INS(NEQ_LI, 2, 5, 0);
	JMP(5);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 4, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 4, FALSE_TAG, 0);

	// (a == 3 || b == 4) && (c == 5 && d == 6)
	INS(EQ_LI, 0, 3, 0);
	JMP(3);
	INS(NEQ_LI, 1, 4, 0);
	JMP(7);
	INS(NEQ_LI, 2, 5, 0);
	JMP(5);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 5, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 5, FALSE_TAG, 0);

	// (a == 3 && b == 4) && (c == 5 || d == 6)
	INS(NEQ_LI, 0, 3, 0);
	JMP(9);
	INS(NEQ_LI, 1, 4, 0);
	JMP(7);
	INS(EQ_LI, 2, 5, 0);
	JMP(3);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 6, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 6, FALSE_TAG, 0);

	// (a == 3 || b == 4) && (c == 5 || d == 6)
	INS(EQ_LI, 0, 3, 0);
	JMP(3);
	INS(NEQ_LI, 1, 4, 0);
	JMP(7);
	INS(EQ_LI, 2, 5, 0);
	JMP(3);
	INS(NEQ_LI, 3, 6, 0);
	JMP(3);
	INS(MOV_LP, 7, TRUE_TAG, 0);
	JMP(2);
	INS(MOV_LP, 7, FALSE_TAG, 0);

	INS(RET0, 0, 0, 0);
	FREE();
}
