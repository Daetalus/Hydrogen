
//
//  Jump Lists
//

#include "jmp.h"


// Modify the target of all jumps in a conditional expression to point the
// location of the false case to `target`.
void jmp_false_case(Function *fn, Index jump, Index target) {
	// Iterate over jump list
	Index current = jump;
	while (current != NOT_FOUND) {
		jmp_lazy_target(fn, current, target);
		current = jmp_next(fn, current);
	}

	// Point the original jump to the false case (not lazily)
	jmp_target(fn, jump, target);
}


// Return the inverted opcode for a conditional opcode.
static BytecodeOpcode jmp_inverted_opcode(BytecodeOpcode opcode) {
	if (opcode == IS_TRUE_L) {
		return IS_FALSE_L;
	} else if (opcode == IS_FALSE_L) {
		return IS_TRUE_L;
	} else if (opcode >= EQ_LL && opcode <= EQ_LP) {
		return (BytecodeOpcode) (NEQ_LL + (opcode - EQ_LL));
	} else if (opcode >= NEQ_LL && opcode <= NEQ_LP) {
		return (BytecodeOpcode) (EQ_LL + (opcode - NEQ_LL));
	} else if (opcode >= LT_LL && opcode <= LT_LN) {
		return (BytecodeOpcode) (GE_LL + (opcode - LT_LL));
	} else if (opcode >= LE_LL && opcode <= LE_LN) {
		return (BytecodeOpcode) (GT_LL + (opcode - LE_LL));
	} else if (opcode >= GT_LL && opcode <= GT_LN) {
		return (BytecodeOpcode) (LE_LL + (opcode - GT_LL));
	} else if (opcode >= GE_LL && opcode <= GE_LN) {
		return (BytecodeOpcode) (LT_LL + (opcode - GE_LL));
	} else {
		return NO_OP;
	}
}


// Invert the condition of a conditional jump operation.
void jmp_invert_condition(Function *fn, Index jump) {
	// Get the current condition opcode
	Instruction ins = vec_at(fn->instructions, jump - 1);
	BytecodeOpcode current = (BytecodeOpcode) ins_arg(ins, 0);

	// Set it to the inverted version of this opcode
	BytecodeOpcode inverted = jmp_inverted_opcode(current);
	vec_at(fn->instructions, jump - 1) = ins_set(ins, 0, inverted);
}
