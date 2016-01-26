
//
//  Expression Tests
//

extern "C" {
#include <hydrogen.h>
#include <vec.h>
#include <parser.h>
#include <pkg.h>
#include <vm.h>
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
	HyError *err = pkg_compile(pkg, src_index, &main_fn);            \
	if (err != NULL) {                                               \
		FAIL() << "Compilation error: " << err->description << "\n"; \
	}                                                                \
	Function *fn;                                                    \
	uint32_t index;                                                  \
	FN(main_fn);


// Frees resources allocated when creating a compiler.
#define FREE() \
	hy_free(state);


// Asserts the next instruction has the opcode `opcode` and 3 arguments `arg1`,
// `arg2`, `arg3`.
#define INS(opcode, arg1, arg2, arg3) {           \
	ASSERT_LT(index, vec_len(fn->bytecode));      \
	Instruct ins = vec_at(fn->bytecode, index++); \
	ASSERT_EQ(ins_arg(ins, 0), opcode);           \
	ASSERT_EQ(ins_arg(ins, 1), arg1);             \
	ASSERT_EQ(ins_arg(ins, 2), arg2);             \
	ASSERT_EQ(ins_arg(ins, 3), arg3);             \
}


// Tests assigning to new locals inside a block scope.
TEST(assign) {
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
TEST(reassign) {
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


int main(int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
