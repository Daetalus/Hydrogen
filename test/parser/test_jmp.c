
//
//  Jump Tests
//

#include <mock_fn.h>
#include <test.h>

#include <jmp.h>
#include <value.h>


// Tests finding the next instruction in a jump list
void test_next(void) {
	MOCK_FN(fn,
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
	eq_uint(jump, 3);
	jump = jmp_next(&fn, jump);
	eq_uint(jump, 1);
	jump = jmp_next(&fn, jump);
	eq_uint(jump, NOT_FOUND);

	mock_fn_free(&fn);
}


// Tests finding the last instruction in a jump list
void test_last(void) {
	MOCK_FN(fn,
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

	eq_uint(jmp_last(&fn, 5), 1);
	eq_uint(jmp_last(&fn, 3), 1);

	mock_fn_free(&fn);
}


// Tests setting the target of a jump instruction
void test_target(void) {
	MOCK_FN(fn,
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		RET0, 0, 0, 0
	);

	jmp_target(&fn, 0, 2);
	eq_uint(ins_arg(vec_at(fn.instructions, 0), 1), 2);
	jmp_target(&fn, 1, 2);
	eq_uint(ins_arg(vec_at(fn.instructions, 1), 1), 1);

	mock_fn_free(&fn);
}


// Tests setting the target of every jump instruction in a jump list
void test_target_all(void) {
	MOCK_FN(fn,
		JMP, 0, 0, 0,
		JMP, 0, 1, 0,
		JMP, 0, 1, 0,
		JMP, 0, 1, 0,
		RET0, 0, 0, 0
	);

	jmp_target_all(&fn, 3, 4);
	eq_uint(ins_arg(vec_at(fn.instructions, 0), 1), 4);
	eq_uint(ins_arg(vec_at(fn.instructions, 1), 1), 3);
	eq_uint(ins_arg(vec_at(fn.instructions, 2), 1), 2);
	eq_uint(ins_arg(vec_at(fn.instructions, 3), 1), 1);

	mock_fn_free(&fn);
}


// Tests appending a jump instruction to a jump list
void test_append(void) {
	MOCK_FN(fn,
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		JMP, 0, 0, 0,
		RET0, 0, 0, 0
	);

	jmp_append(&fn, 2, 1);
	eq_uint(ins_arg(vec_at(fn.instructions, 2), 2), 1);
	eq_uint(ins_arg(vec_at(fn.instructions, 1), 2), 0);
	jmp_append(&fn, 2, 0);
	eq_uint(ins_arg(vec_at(fn.instructions, 2), 2), 1);
	eq_uint(ins_arg(vec_at(fn.instructions, 1), 2), 1);

	mock_fn_free(&fn);
}


int main(int argc, char *argv[]) {
	test_pass("Next in jump list", test_next);
	test_pass("Last in jump list", test_last);
	test_pass("Set target", test_target);
	test_pass("Set target of all", test_target_all);
	test_pass("Append to jump list", test_append);
	return test_run(argc, argv);
}
