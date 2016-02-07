
//
//  Test Utilities
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
#define FN(fn_index)                            \
	fn = &vec_at(state->functions, (fn_index)); \
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
