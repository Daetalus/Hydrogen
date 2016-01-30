
//
//  Jump Tests
//

extern "C" {
#include <hydrogen.h>
#include <jmp.h>
#include <value.h>
}

#include <gtest/gtest.h>


// Creates a function with the bytecode provided in the variable arguments.
#define FUNCTION(...)                                                     \
	uint16_t bytecode[] = {__VA_ARGS__};                                  \
	uint32_t count = sizeof(bytecode) / sizeof(uint16_t);                 \
	Function fn;                                                          \
	fn.name = NULL;                                                       \
	fn.length = 0;                                                        \
	fn.package = 0;                                                       \
	fn.source = 0;                                                        \
	fn.line = 0;                                                          \
	fn.arity = 0;                                                         \
	fn.frame_size = 0;                                                    \
	vec_new(fn.instructions, Instruction, count / 4);                     \
	for (uint32_t i = 0; i < count; i += 4) {                             \
		vec_len(fn.instructions)++;                                       \
		vec_last(fn.instructions) = ins_new((BytecodeOpcode) bytecode[i], \
			bytecode[i + 1], bytecode[i + 2], bytecode[i + 3]);           \
	}


// Frees resources allocated by creating a function.
#define FREE() \
	vec_free(fn.instructions);


// Tests finding the next instruction in a jump list.
TEST(Jump, Next) {
	FUNCTION(
		NEQ_LL, 0, 3, 0,
		JMP, 5, 0, JMP_AND,
		NEQ_LL, 1, 4, 0,
		JMP, 3, 2, JMP_AND,
		EQ_LL, 2, 5, 0,
		JMP, 3, 2, JMP_AND,
		MOV_LP, 4, TAG_FALSE, 0,
		JMP, 2, 0, JMP_NONE,
		MOV_LP, 4, TAG_TRUE, 0,
		RET0, 0, 0, 0
	);

	Index jump = 5;
	jump = jmp_next(&fn, jump);
	ASSERT_EQ(jump, (Index) 3);
	jump = jmp_next(&fn, jump);
	ASSERT_EQ(jump, (Index) 1);
	jump = jmp_next(&fn, jump);
	ASSERT_EQ(jump, NOT_FOUND);

	FREE();
}


// Tests finding the last instruction in a jump list.
TEST(Jump, Last) {
	FUNCTION(
		NEQ_LL, 0, 3, 0,
		JMP, 5, 0, JMP_AND,
		NEQ_LL, 1, 4, 0,
		JMP, 3, 2, JMP_AND,
		EQ_LL, 2, 5, 0,
		JMP, 3, 2, JMP_AND,
		MOV_LP, 4, TAG_FALSE, 0,
		JMP, 2, 0, JMP_NONE,
		MOV_LP, 4, TAG_TRUE, 0,
		RET0, 0, 0, 0
	);

	ASSERT_EQ(jmp_last(&fn, 5), (Index) 1);
	ASSERT_EQ(jmp_last(&fn, 3), (Index) 1);

	FREE();
}


// Tests setting the target of a jump instruction.
TEST(Jump, Target) {
	FUNCTION(
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		RET0, 0, 0, 0
	);

	jmp_target(&fn, 0, 2);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 0), 1), 2);
	jmp_target(&fn, 1, 2);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 1), 1), 1);
}


// Tests setting the target of every jump instruction in a jump list.
TEST(Jump, TargetAll) {
	FUNCTION(
		JMP, 0, 0, 0,
		JMP, 0, 1, 0,
		JMP, 0, 1, 0,
		JMP, 0, 1, 0,
		RET0, 0, 0, 0
	);

	jmp_target_all(&fn, 3, 4);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 0), 1), 4);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 1), 1), 3);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 2), 1), 2);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 3), 1), 1);
}


// Tests appending a jump instruction to a jump list.
TEST(Jump, Append) {
	FUNCTION(
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		RET0, 0, 0, 0
	);

	jmp_append(&fn, 2, 1);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 2), 2), 1);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 1), 2), 0);
	jmp_append(&fn, 2, 0);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 2), 2), 1);
	ASSERT_EQ(ins_arg(vec_at(fn.instructions, 1), 2), 1);
}
