
//
//  Jump Tests
//

#include "test.h"


TEST(next) {
	FUNCTION(
		NEQ_LL, 0, 3, 0,
		JMP, 5, 0, JUMP_AND,
		NEQ_LL, 1, 4, 0,
		JMP, 3, 2, JUMP_AND,
		EQ_LL, 2, 5, 0,
		JMP, 3, 2, JUMP_AND,
		MOV_LP, 4, FALSE_TAG, 0,
		JMP, 2, 0, JUMP_NONE,
		MOV_LP, 4, TRUE_TAG, 0,
		RET0, 0, 0, 0
	);

	int jump = 5;
	jump = jmp_next(&fn, jump);
	EQ(jump, 3);
	jump = jmp_next(&fn, jump);
	EQ(jump, 1);
	jump = jmp_next(&fn, jump);
	EQ(jump, -1);
}


TEST(last) {
	FUNCTION(
		NEQ_LL, 0, 3, 0,
		JMP, 5, 0, JUMP_AND,
		NEQ_LL, 1, 4, 0,
		JMP, 3, 2, JUMP_AND,
		EQ_LL, 2, 5, 0,
		JMP, 3, 2, JUMP_AND,
		MOV_LP, 4, FALSE_TAG, 0,
		JMP, 2, 0, JUMP_NONE,
		MOV_LP, 4, TRUE_TAG, 0,
		RET0, 0, 0, 0
	);

	EQ(jmp_last(&fn, 5), 1);
	EQ(jmp_last(&fn, 3), 1);
}


MAIN() {
	RUN(next);
	RUN(last);
}
