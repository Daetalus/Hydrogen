
//
//  Expression Parsing
//

#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include "../bytecode.h"
#include "../value.h"

#include "expr.h"
#include "jmp.h"
#include "local.h"
#include "fn.h"
#include "struct.h"


// Possible operator precedences.
typedef enum {
	PREC_NONE,
	// Boolean operators
	PREC_OR,
	PREC_AND,
	// Bitwise operators
	PREC_BIT_OR,
	PREC_BIT_XOR,
	PREC_BIT_AND,
	// Equal, not equal
	PREC_EQ,
	// Less than, less than equal, greater than, greater than equal
	PREC_ORD,
	// Addition, subtraction
	PREC_ADD,
	// Concatenation
	PREC_CONCAT,
	// Multiplication, division, modulo
	PREC_MUL,
} Precedence;


// Evaluates to true if an operand is a number.
#define IS_NUMBER(type) (type == OP_INTEGER || type == OP_NUMBER)

// Evaluates to true if an operand is a jump or local.
#define IS_JUMP_OR_LOCAL(type) ((type) == OP_LOCAL || (type) == OP_JUMP)


// Parses an expression, stopping when we reach a binary operator of lower
// precedence than the given precedence.
Operand expr_prec(Parser *parser, uint16_t slot, Precedence precedence);


// Returns the precedence of a binary operator.
Precedence binary_prec(TokenType operator) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
		return PREC_ADD;
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return PREC_MUL;
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return PREC_EQ;
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return PREC_ORD;
	case TOKEN_AND:
		return PREC_AND;
	case TOKEN_OR:
		return PREC_OR;
	case TOKEN_BIT_AND:
		return PREC_BIT_AND;
	case TOKEN_BIT_OR:
		return PREC_BIT_OR;
	case TOKEN_BIT_XOR:
		return PREC_BIT_XOR;
	default:
		return PREC_NONE;
	}
}


// Returns the opcode for an arithmetic operation (including concatenation).
// Either left or right must be a local.
Opcode arithmetic_opcode(TokenType operator, OperandType left,
		OperandType right) {
	if (operator == TOKEN_CONCAT) {
		// Concatenation
		int offset = (right == OP_STRING ? 1 : (left == OP_STRING ? 2 : 0));
		return CONCAT_LL + offset;
	} else {
		// Arithmetic
		int base = ADD_LL + (operator - TOKEN_ADD) * 5;
		return base + (left == OP_LOCAL ? right : left + 2);
	}
}


// Returns the inverted opcode for a comparison operation. Either left or right
// must be a local.
Opcode comparison_opcode(TokenType operator, OperandType left,
		OperandType right) {
	Opcode base;
	switch (operator) {
	case TOKEN_EQ:
		base = NEQ_LL;
		break;
	case TOKEN_NEQ:
		base = EQ_LL;
		break;
	case TOKEN_LT:
		base = GE_LL;
		break;
	case TOKEN_LE:
		base = GT_LL;
		break;
	case TOKEN_GT:
		base = LE_LL;
		break;
	case TOKEN_GE:
		base = LT_LL;
		break;
	default:
		return NO_OP;
	}

	return base + (left == OP_LOCAL ? right : left);
}


// Returns true if the given operands are valid for the given binary operation.
bool binary_valid(TokenType operator, OperandType left, OperandType right) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return (IS_NUMBER(left) || left == OP_LOCAL) &&
			(IS_NUMBER(right) || right == OP_LOCAL);
	case TOKEN_CONCAT:
		return (left == OP_STRING || left == OP_LOCAL) &&
			(right == OP_STRING || right == OP_LOCAL);
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return (left >= OP_LOCAL && left <= OP_PRIMITIVE) &&
			(right >= OP_LOCAL && right <= OP_PRIMITIVE);
	case TOKEN_AND:
	case TOKEN_OR:
		return left != OP_NONE && left != OP_PACKAGE &&
			right != OP_NONE && right != OP_PACKAGE;
	default:
		return false;
	}
}


// Performs an arithmetic operation on two integers.
int32_t binary_integer_arithmetic(TokenType operator, int16_t left,
		int16_t right) {
	switch (operator) {
	case TOKEN_ADD:
		return left + right;
	case TOKEN_SUB:
		return left - right;
	case TOKEN_MUL:
		return left * right;
	case TOKEN_MOD:
		return left % right;
	default:
		return 0;
	}
}


// Performs an arithmetic operation on two doubles.
double binary_number_arithmetic(TokenType operator, double left, double right) {
	switch (operator) {
	case TOKEN_ADD:
		return left + right;
	case TOKEN_SUB:
		return left - right;
	case TOKEN_MUL:
		return left * right;
	case TOKEN_DIV:
		return left / right;
	case TOKEN_MOD:
		return fmod(left, right);
	default:
		return 0;
	}
}


// Returns the opcode for a unary operator.
Opcode unary_opcode(TokenType operator) {
	switch (operator) {
	case TOKEN_SUB:
		return NEG_L;
	default:
		return NO_OP;
	}
}


// Returns true if the given operand is valid for the given unary operation.
bool unary_valid(Opcode operator, OperandType operand) {
	switch (operator) {
	case NEG_L:
		return IS_NUMBER(operand) || operand == OP_LOCAL;
	default:
		return false;
	}
}


// Moves a top level variable at `index` in the parser's function's package
// into the given stack slot.
void expr_top_level_to_local(Parser *parser, uint16_t slot, uint16_t index) {
	Function *fn = &parser->vm->functions[parser->fn_index];
	uint16_t package_index = fn->package - parser->vm->packages;
	emit(fn, instr_new(MOV_LT, slot, package_index, index));
}


// Creates a new, empty operand.
Operand operand_new(void) {
	Operand operand;
	operand.type = OP_NONE;
	operand.self.type = SELF_NONE;
	operand.self.slot = 0;
	operand.self.package_index = 0;
	operand.self.is_method = false;
	return operand;
}


// Converts an integer or number operand into a double value.
double operand_to_number(Parser *parser, Operand operand) {
	if (operand.type == OP_NUMBER) {
		return val_to_num(parser->vm->numbers[operand.number]);
	} else {
		return (double) operand.integer;
	}
}


// Converts an operand (that isn't a local) into a true or false value.
bool operand_to_boolean(Operand operand) {
	return operand.type == OP_PRIMITIVE && operand.primitive == TRUE_TAG;
}


// Attempts to fold an `and` or `or` operation.
Operand fold_condition(TokenType operator, Operand left, Operand right) {
	Operand operand = operand_new();

	// Don't fold if both are locals
	if (IS_JUMP_OR_LOCAL(left.type) && IS_JUMP_OR_LOCAL(right.type)) {
		return operand;
	}

	// Convert each operand into a boolean
	bool first = operand_to_boolean(left);
	bool second = operand_to_boolean(right);

	// If neither are locals
	if (!IS_JUMP_OR_LOCAL(left.type) && !IS_JUMP_OR_LOCAL(right.type)) {
		bool result = (operator == TOKEN_OR) ? (first || second) :
			(first && second);
		operand.type = OP_PRIMITIVE;
		operand.primitive = result ? TRUE_TAG : FALSE_TAG;
		return operand;
	}

	// Make operand and constant agnostic of order
	Operand local = IS_JUMP_OR_LOCAL(left.type) ? left : right;
	bool constant = IS_JUMP_OR_LOCAL(left.type) ? second : first;

	// Either left or right is a local, but not both
	if (operator == TOKEN_AND) {
		if (constant) {
			// Something && true = Something
			return local;
		} else {
			// Something && false = false
			operand.type = OP_PRIMITIVE;
			operand.primitive = FALSE_TAG;
			return operand;
		}
	} else {
		if (!constant) {
			// Something || false = something
			return local;
		} else {
			// Something || true = true
			operand.type = OP_PRIMITIVE;
			operand.primitive = TRUE_TAG;
			return operand;
		}
	}
}


// Attempts to fold an equality test.
Operand fold_equal(Parser *parser, TokenType operator, Operand left,
		Operand right) {
	Operand operand = operand_new();

	// Only fold if types are equal
	if (left.type != right.type) {
		return operand;
	}

	// If their values are equal
	operand.type = OP_PRIMITIVE;
	if (left.value == right.value) {
		operand.primitive = (operator == TOKEN_EQ) ? TRUE_TAG : FALSE_TAG;
		return operand;
	}

	// Don't fold non-identical locals
	if (left.type == OP_LOCAL) {
		operand.type = OP_NONE;
		return operand;
	}

	// Try equality tests depending on the operand type
	switch (left.type) {
	case OP_NUMBER: {
		double first = parser->vm->numbers[left.number];
		double second = parser->vm->numbers[right.number];
		operand.primitive = (first == second) ? TRUE_TAG : FALSE_TAG;
		break;
	}
	case OP_STRING: {
		char *first = vm_string(parser->vm, left.string);
		char *second = vm_string(parser->vm, right.string);
		operand.primitive = (strcmp(first, second) == 0) ? TRUE_TAG : FALSE_TAG;
		break;
	}
	default:
		operand.primitive = FALSE_TAG;
		break;
	}

	// Invert the result if we're testing for inequality
	if (operator == TOKEN_NEQ && operand.type == OP_PRIMITIVE) {
		bool equal = (operand.primitive == TRUE_TAG);
		operand.primitive = equal ? FALSE_TAG : TRUE_TAG;
	}

	return operand;
}


// Returns the result of a comparison between two identical locals.
uint16_t compare_locals(TokenType operator) {
	switch (operator) {
	case TOKEN_LT: return FALSE_TAG;
	case TOKEN_LE: return TRUE_TAG;
	case TOKEN_GT: return FALSE_TAG;
	case TOKEN_GE: return TRUE_TAG;
	default: return FALSE_TAG;
	}
}


// Returns the result of a comparison between two numbers.
uint16_t compare_numbers(TokenType operator, double left, double right) {
	switch (operator) {
	case TOKEN_LT: return (left < right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_LE: return (left <= right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_GT: return (left > right) ? TRUE_TAG : FALSE_TAG;
	case TOKEN_GE: return (left >= right) ? TRUE_TAG : FALSE_TAG;
	default: return FALSE_TAG;
	}
}


// Attempts to fold an order operation.
Operand fold_order(Parser *parser, TokenType operator, Operand left,
		Operand right) {
	Operand operand = operand_new();

	if (left.type == OP_LOCAL && right.type == OP_LOCAL &&
			left.slot == right.slot) {
		// Two identical locals
		operand.type = OP_PRIMITIVE;
		operand.primitive = compare_locals(operator);
	} else if (IS_NUMBER(left.type) && IS_NUMBER(right.type)) {
		// Convert operands into numbers
		double first = operand_to_number(parser, left);
		double second = operand_to_number(parser, right);

		// Compare them
		operand.type = OP_PRIMITIVE;
		operand.primitive = compare_numbers(operator, first, second);
	}

	// Can't fold otherwise
	return operand;
}


// Attempts to fold a concatenation operation.
Operand fold_concat(Parser *parser, Operand left, Operand right) {
	Operand operand = operand_new();

	// Only fold if both are strings
	if (left.type != OP_STRING || right.type != OP_STRING) {
		return operand;
	}

	// Extract both strings from the VM
	char *first = vm_string(parser->vm, left.string);
	char *second = vm_string(parser->vm, right.string);
	size_t length = strlen(first);

	// Combine both strings
	char result[length + strlen(second) + 1];
	strcpy(result, first);
	strcpy(&result[length], second);

	operand.type = OP_STRING;
	operand.string = vm_add_string(parser->vm, result);
	return operand;
}


// Attempts to fold an arithmetic operation.
Operand fold_arithmetic(Parser *parser, TokenType operator, Operand left,
		Operand right) {
	Operand operand = operand_new();

	// Only fold if both are numbers
	if (!IS_NUMBER(left.type) || !IS_NUMBER(right.type)) {
		return operand;
	}

	// If both are integers and the operation is not division, return an integer
	if (left.type == OP_INTEGER && right.type == OP_INTEGER &&
			operator != TOKEN_DIV) {
		int32_t result = binary_integer_arithmetic(operator,
			left.integer, right.integer);

		// Return a double if the result is beyond the bounds of a 16 bit
		// integer
		if (result > SHRT_MAX || result < SHRT_MIN) {
			// Convert to a double
			operand.type = OP_NUMBER;
			operand.number = vm_add_number(parser->vm, (double) result);
		} else {
			// Fits inside the range of an unsigned 16 bit integer
			operand.type = OP_INTEGER;
			operand.integer = (int16_t) result;
		}
	} else {
		// Convert both to numbers and compute the operation
		double left_value = operand_to_number(parser, left);
		double right_value = operand_to_number(parser, right);
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm,
			binary_number_arithmetic(operator, left_value, right_value));
	}

	return operand;
}


// Attempts to fold a binary operation. Assumes the operands are valid for the
// operation.
Operand fold_binary(Parser *parser, TokenType operator, Operand left,
		Operand right) {
	Operand operand = operand_new();

	if (operator == TOKEN_CONCAT) {
		// Concatenation
		return fold_concat(parser, left, right);
	} else if (operator >= TOKEN_ADD && operator <= TOKEN_MOD) {
		// Arithmetic
		return fold_arithmetic(parser, operator, left, right);
	} else if (operator == TOKEN_EQ || operator == TOKEN_NEQ) {
		// Equality
		return fold_equal(parser, operator, left, right);
	} else if (operator >= TOKEN_LT && operator <= TOKEN_GE) {
		// Ordering
		return fold_order(parser, operator, left, right);
	} else if (operator == TOKEN_AND || operator == TOKEN_OR) {
		// Conditional
		return fold_condition(operator, left, right);
	} else {
		// Unknown operator
		return operand;
	}
}


// Returns the complementary conditional operation to that specified by
// `opcode`.
Opcode inverted_conditional_opcode(Opcode opcode) {
	if (opcode == IS_TRUE_L) {
		return IS_FALSE_L;
	} else if (opcode == IS_FALSE_L) {
		return IS_TRUE_L;
	} else if (opcode >= EQ_LL && opcode <= EQ_LP) {
		return NEQ_LL + (opcode - EQ_LL);
	} else if (opcode >= NEQ_LL && opcode <= NEQ_LP) {
		return EQ_LL + (opcode - NEQ_LL);
	} else if (opcode >= LT_LL && opcode <= LT_LN) {
		return GE_LL + (opcode - LT_LL);
	} else if (opcode >= LE_LL && opcode <= LE_LN) {
		return GT_LL + (opcode - LE_LL);
	} else if (opcode >= GT_LL && opcode <= GT_LN) {
		return LE_LL + (opcode - GT_LL);
	} else if (opcode >= GE_LL && opcode <= GE_LN) {
		return LT_LL + (opcode - GE_LL);
	} else {
		return NO_OP;
	}
}


// Inverts the condition of a conditional instruction.
void invert_condition(Function *fn, int index) {
	uint64_t condition = fn->bytecode[index];
	Opcode current = (Opcode) instr_opcode(condition);
	Opcode inverted = inverted_conditional_opcode(current);
	fn->bytecode[index] = instr_modify_opcode(condition, inverted);
}


// Emits bytecode to convert a local operand into a jump.
Operand operand_to_jump(Parser *parser, Operand operand) {
	Function *fn = &parser->vm->functions[parser->fn_index];
	Operand result;
	result.type = OP_JUMP;

	// Emit a comparison and empty jump instruction
	emit(fn, instr_new(IS_FALSE_L, operand.slot, 0, 0));
	result.jump = jmp_new(fn);
	return result;
}


// Emits bytecode for an `and` operation. The left operand is expected to
// have the jump operand type.
Operand expr_and(Parser *parser, Operand left, Operand right) {
	// Convert right into a jump
	if (right.type != OP_JUMP) {
		right = operand_to_jump(parser, right);
	}

	Function *fn = &parser->vm->functions[parser->fn_index];

	// Point end of right's jump list to left
	int last = jmp_last(fn, right.jump);
	jmp_append(fn, last, left.jump);

	// Make both operands part of an `and` operation
	if (jmp_type(fn, left.jump) == JUMP_NONE) {
		jmp_set_type(fn, left.jump, JUMP_AND);
	}
	if (jmp_type(fn, right.jump) == JUMP_NONE) {
		jmp_set_type(fn, right.jump, JUMP_AND);
	}

	return right;
}


// Emits bytecode for an `or` operation. The left operand is expected to
// have the jump operand type.
Operand expr_or(Parser *parser, Operand left, Operand right) {
	// Convert right into a jump
	if (right.type != OP_JUMP) {
		right = operand_to_jump(parser, right);
	}

	Function *fn = &parser->vm->functions[parser->fn_index];

	// Point end of right's jump list to left
	int last = jmp_last(fn, right.jump);
	jmp_append
		(fn, last, left.jump);

	// Invert left's condition
	invert_condition(fn, left.jump - 1);

	// Iterate over left's jump list
	int current = left.jump;
	while (current != -1) {
		// Point to after right by default
		int target = right.jump + 1;

		// For conditions part of AND statements
		if (jmp_type(fn, current) == JUMP_AND) {
			// Point to last element in right's jump list
			target = last - 1;
		}

		// Set jump target
		jmp_target(fn, current, target);

		// Get next element in jump list
		current = jmp_next(fn, current);
	}

	// Point left to after right
	jmp_target(fn, left.jump, right.jump + 1);

	// Make both operands part of an `or` operation
	if (jmp_type(fn, left.jump) == JUMP_NONE) {
		jmp_set_type(fn, left.jump, JUMP_OR);
	}
	if (jmp_type(fn, right.jump) == JUMP_NONE) {
		jmp_set_type(fn, right.jump, JUMP_OR);
	}

	return right;
}


// Emits bytecode for a binary operator.
Operand expr_binary(Parser *parser, uint16_t slot, TokenType operator,
		Operand left, Operand right) {
	Function *fn = &parser->vm->functions[parser->fn_index];
	Operand operand = operand_new();

	// Ensure the operands are valid for the operator
	if (!binary_valid(operator, left.type, right.type)) {
		UNEXPECTED("Invalid operand to binary operator");
		return operand;
	}

	// Attempt to fold the operation
	operand = fold_binary(parser, operator, left, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Handle conditional operations
	if (operator == TOKEN_AND) {
		return expr_and(parser, left, right);
	} else if (operator == TOKEN_OR) {
		return expr_or(parser, left, right);
	} else if (operator >= TOKEN_ADD && operator <= TOKEN_CONCAT) {
		// Arithmetic
		operand.type = OP_LOCAL;
		operand.slot = slot;

		// Emit the operation
		Opcode opcode = arithmetic_opcode(operator, left.type, right.type);
		emit(fn, instr_new(opcode, slot, left.value, right.value));
	} else if (operator >= TOKEN_EQ && operator <= TOKEN_GE) {
		// Comparison
		operand.type = OP_JUMP;

		// The local must be on the left if we're not comparing two locals
		Opcode opcode = comparison_opcode(operator, left.type, right.type);
		uint16_t left_value = left.value;
		uint16_t right_value = right.value;
		if (left.type != OP_LOCAL && right.type == OP_LOCAL) {
			// Only invert if it's not comparing equality
			if (operator != TOKEN_EQ && operator != TOKEN_NEQ) {
				opcode = inverted_conditional_opcode(opcode);
			}

			// Swap the arguments to the instruction
			left_value = right.value;
			right_value = left.value;
		}

		// Emit the comparison and the empty jump instruction following it
		emit(fn, instr_new(opcode, left_value, right_value, 0));
		operand.jump = jmp_new(fn);
	}

	return operand;
}


// Emits bytecode for the left operator in a binary expression.
Operand expr_binary_left(Parser *parser, TokenType operator, Operand left) {
	// Turn the operand into a jump statement if we're parsing an `and` or `or`
	// operator
	if ((operator == TOKEN_AND || operator == TOKEN_OR) &&
			left.type == OP_LOCAL) {
		operand_to_jump(parser, left);
	}

	// Don't make any modification to the operand
	return left;
}


// Attempts to fold a unary operation.
Operand fold_unary(Parser *parser, Opcode opcode, Operand right) {
	Operand operand = operand_new();

	// Can only fold negation
	if (opcode != NEG_L) {
		return operand;
	}

	// Can only fold numbers and integers
	if (right.type == OP_NUMBER) {
		double value = parser->vm->numbers[right.number];
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm, -value);
	} else if (right.type == OP_INTEGER) {
		operand.type = OP_INTEGER;
		operand.integer = -right.integer;
	}

	return operand;
}


// Emits bytecode for a unary operator.
Operand expr_unary(Parser *parser, Opcode opcode, Operand right) {
	Operand operand = operand_new();
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Ensure the operand is valid for the operator
	if (!unary_valid(opcode, right.type)) {
		UNEXPECTED("Invalid operand to unary operator");
		return operand;
	}

	// Attempt to fold the operation
	operand = fold_unary(parser, opcode, right);
	if (operand.type != OP_NONE) {
		return operand;
	}

	// Allocate a temporary local on the stack
	operand.type = OP_LOCAL;
	scope_new(parser);
	local_new(parser, &operand.slot);

	// Emit operation
	emit(fn, instr_new(opcode, operand.slot, right.value, 0));

	// Return a the local in which we stored the result of the operation
	return operand;
}


// Modifies the targets of the jump instructions in a conditional expression
// to update the location of the false case to `false_case`.
void expr_patch_false_case(Parser *parser, Operand operand, int false_case) {
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Iterate over jump list
	int current = operand.jump;
	while (current != -1) {
		jmp_lazy_target(fn, current, false_case);
		current = jmp_next(fn, current);
	}

	// Point the operand to the false case
	jmp_target(fn, operand.jump, false_case);
}


// Stores the value of an operand into `slot` on the stack.
void expr_discharge(Parser *parser, uint16_t slot, Operand operand) {
	Function *fn = &parser->vm->functions[parser->fn_index];

	if (operand.type == OP_LOCAL) {
		// Copy a local if isn't in a deallocated scope
		if (operand.slot != slot && operand.slot < parser->locals_count) {
			emit(fn, instr_new(MOV_LL, slot, operand.slot, 0));
		}
	} else if (operand.type == OP_JUMP) {
		// Emit true case, jump over false case, and false case
		emit(fn, instr_new(MOV_LP, slot, TRUE_TAG, 0));
		emit(fn, instr_new(JMP, 2, 0, 0));
		uint32_t false_case = emit(fn, instr_new(MOV_LP, slot, FALSE_TAG, 0));

		// Finish the condition now that we know the location of the false case
		expr_patch_false_case(parser, operand, false_case);
	} else {
		// Emit a store instruction for the appropriate type
		Opcode opcode = MOV_LL + operand.type;
		emit(fn, instr_new(opcode, slot, operand.value, 0));
	}
}


// Parses an operand into `slot`.
Operand expr_operand(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;
	Operand operand = operand_new();

	switch (lexer->token.type) {
	case TOKEN_INTEGER:
		operand.type = OP_INTEGER;
		operand.integer = lexer->token.integer;
		lexer_next(lexer);
		break;

	case TOKEN_NUMBER:
		operand.type = OP_NUMBER;
		operand.number = vm_add_number(parser->vm, lexer->token.number);
		lexer_next(lexer);
		break;

	case TOKEN_STRING: {
		char *string = lexer_extract_string(lexer, &lexer->token);
		operand.type = OP_STRING;
		operand.string = vm_add_string(parser->vm, string);
		free(string);
		lexer_next(lexer);
		break;
	}

	case TOKEN_IDENTIFIER: {
		// Find an existing variable with the given name
		char *name = lexer->token.start;
		size_t length = lexer->token.length;
		Variable var = local_capture(parser, name, length);
		Function *fn = &parser->vm->functions[parser->fn_index];

		if (var.type == VAR_LOCAL) {
			operand.type = OP_LOCAL;
			operand.slot = var.slot;

			operand.self.type = SELF_LOCAL;
			operand.self.slot = var.slot;
		} else if (var.type == VAR_UPVALUE) {
			// Store the upvalue into a local slot
			emit(fn, instr_new(MOV_LU, slot, var.slot, 0));
			operand.type = OP_LOCAL;

			operand.slot = slot;
			operand.self.type = SELF_UPVALUE;
			operand.self.slot = var.slot;
		} else if (var.type == VAR_PACKAGE) {
			operand.type = OP_PACKAGE;
			operand.index = var.slot;
		} else if (var.type == VAR_NATIVE_PACKAGE) {
			// Parse a native function call and set the operand to its return
			// slot
			parse_native_fn_call(parser, var.slot, slot);
			operand.type = OP_LOCAL;
			operand.slot = slot;

			// Don't consume another token
			break;
		} else if (var.type == VAR_TOP_LEVEL) {
			// Store the top level variable into a local
			expr_top_level_to_local(parser, slot, var.slot);
			operand.type = OP_LOCAL;
			operand.slot = slot;

			operand.self.type = SELF_TOP_LEVEL;
			operand.self.slot = var.slot;
			operand.self.package_index = fn->package - parser->vm->packages;
		} else {
			// Undefined
			ERROR("Undefined variable `%.*s` in expression", length, name);
			break;
		}

		lexer_next(lexer);
		break;
	}

	case TOKEN_TRUE:
		operand.type = OP_PRIMITIVE;
		operand.primitive = TRUE_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_FALSE:
		operand.type = OP_PRIMITIVE;
		operand.primitive = FALSE_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_NIL:
		operand.type = OP_PRIMITIVE;
		operand.primitive = NIL_TAG;
		lexer_next(lexer);
		break;

	case TOKEN_OPEN_PARENTHESIS:
		// Skip the opening parenthesis
		lexer_next(lexer);

		// Parse the expression within the parentheses
		operand = expr_prec(parser, slot, 0);

		// Expect a closing parenthesis
		EXPECT(TOKEN_CLOSE_PARENTHESIS,
			"Expected `)` to close `(` in expression");
		lexer_next(lexer);
		break;

	case TOKEN_FN:
		// Skip the `fn` token
		lexer_next(lexer);

		// Parse an anonymous function definition
		operand.type = OP_FN;
		operand.fn_index = parse_fn_definition_body(parser, NULL, 0, false);
		break;

	case TOKEN_NEW:
		parse_struct_instantiation(parser, slot);
		operand.type = OP_LOCAL;
		operand.slot = slot;
		break;

	default:
		UNEXPECTED("Expected operand in expression");
		break;
	}

	return operand;
}


// Parses a postfix operator after an operand.
Operand expr_postfix(Parser *parser, Operand operand, uint16_t slot) {
	Lexer *lexer = parser->lexer;
	Operand result = operand_new();

	switch (lexer->token.type) {
	case TOKEN_OPEN_PARENTHESIS: {
		// Function call
		if (operand.type == OP_LOCAL) {
			parse_fn_call_self(parser, CALL_L, operand.slot, slot,
				&operand.self);
		} else if (operand.type == OP_FN) {
			parse_fn_call_self(parser, CALL_F, operand.fn_index, slot,
				&operand.self);
		} else {
			ERROR("Attempt to call non-function");
			return result;
		}

		result.type = OP_LOCAL;
		result.slot = slot;
		break;
	}

	case TOKEN_DOT: {
		// Struct field or package top level variable access
		if (operand.type != OP_LOCAL && operand.type != OP_PACKAGE) {
			ERROR("Attempt to index non-local");
			return result;
		}

		Function *fn = &parser->vm->functions[parser->fn_index];

		// Skip the dot
		lexer_next(lexer);

		// Expect an identifier
		EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `.`");

		// Update the result
		result.type = OP_LOCAL;
		result.slot = slot;

		if (operand.type == OP_LOCAL) {
			// Emit field access
			Identifier ident;
			ident.start = lexer->token.start;
			ident.length = lexer->token.length;
			uint16_t index = vm_add_field(parser->vm, ident);
			emit(fn, instr_new(STRUCT_FIELD, slot, operand.slot, index));

			// Set the struct the local was referenced from in case of a
			// function call, when we need to give the struct to the method
			// for the `self` argument
			if (operand.self.is_method) {
				// This isn't the first index of a struct (ie. `a.b.c`)
				result.self.type = SELF_LOCAL;
				result.self.slot = operand.slot;
				result.self.is_method = true;
			} else {
				// The first index (ie. `a.b`)
				result.self = operand.self;
				result.self.is_method = true;
			}
		} else {
			// Get the index of the top level variable
			char *name = lexer->token.start;
			size_t length = lexer->token.length;
			Package *package = &parser->vm->packages[operand.index];
			int index = package_local_find(package, name, length);

			// Check the variable exists
			if (index == -1) {
				ERROR("Undefined top level variable `%.*s` in package `%s`",
					length, name, package->name);
				return result;
			}

			// Emit package top level variable access
			emit(fn, instr_new(MOV_LT, slot, operand.index, index));

			// Set the struct slot
			result.self.type = SELF_TOP_LEVEL;
			result.self.slot = index;
			result.self.package_index = operand.index;
			result.self.is_method = false;
		}
		lexer_next(lexer);
		break;
	}

	default:
		break;
	}

	return result;
}


// Parses the left hand side of a binary operation, including unary operators,
// an operand, and postfix operators.
Operand expr_left(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;

	// Check for unary operators
	TokenType unary = lexer->token.type;
	Opcode opcode = unary_opcode(unary);
	if (opcode != NO_OP) {
		// Consume the unary operator
		lexer_next(lexer);

		// Parse another unary operator, or the operand itself
		Operand right = expr_left(parser, slot);

		// Emit the unary operand instruction
		return expr_unary(parser, opcode, right);
	} else {
		// Parse an operand
		Operand operand = expr_operand(parser, slot);

		// Check for multiple postfix operators
		Operand postfix = expr_postfix(parser, operand, slot);
		while (postfix.type != OP_NONE) {
			operand = postfix;
			postfix = expr_postfix(parser, operand, slot);
		}

		return operand;
	}
}


// Parses an expression into `slot`, stopping when we reach a binary operator
// of lower precedence than `limit`.
Operand expr_prec(Parser *parser, uint16_t slot, Precedence limit) {
	Lexer *lexer = parser->lexer;

	// Expect a left hand side operand
	Operand left = expr_left(parser, slot);

	// Parse a binary operator
	while (binary_prec(lexer->token.type) > limit) {
		// Consume the operator
		TokenType operator = lexer->token.type;
		lexer_next(lexer);

		// Emit bytecode for the left operand
		left = expr_binary_left(parser, operator, left);

		// Create a local to use for this level
		uint16_t new_slot;
		scope_new(parser);
		local_new(parser, &new_slot);

		// Parse the right hand side
		Precedence prec = binary_prec(operator);
		Operand right = expr_prec(parser, new_slot, prec);
		scope_free(parser);

		// Emit the binary operator
		left = expr_binary(parser, slot, operator, left, right);
	}

	return left;
}


// Parses an expression into `slot`, returning the value of the expression.
// For some expressions, nothing may need to be stored (ie. expressions
// consisting of only a constant), so `slot` will remain unused.
Operand expr(Parser *parser, uint16_t slot) {
	return expr_prec(parser, slot, PREC_NONE);
}


// Parses an expression, storing the result into `slot`.
void expr_emit(Parser *parser, uint16_t slot) {
	Operand operand = expr(parser, slot);
	expr_discharge(parser, slot, operand);
}


// Parses an expression, storing the result into the local with the given name.
// Triggers an error if the local doesn't exist.
void expr_emit_local(Parser *parser, char *name, size_t length) {
	Variable var = local_capture(parser, name, length);
	if (var.type == VAR_LOCAL) {
		// Parse an expression
		expr_emit(parser, var.slot);
	} else if (var.type == VAR_UPVALUE || var.type == VAR_TOP_LEVEL) {
		Function *fn = &parser->vm->functions[parser->fn_index];

		// Parse an expression into an empty local slot
		scope_new(parser);
		uint16_t slot;
		local_new(parser, &slot);
		expr_emit(parser, slot);
		scope_free(parser);

		if (var.type == VAR_UPVALUE) {
			// Store the local into an upvalue
			emit(fn, instr_new(MOV_UL, var.slot, slot, 0));
		} else {
			// Store the local into a top level variable
			uint16_t package_index = fn->package - parser->vm->packages;
			emit(fn, instr_new(MOV_TL, var.slot, package_index, slot));
		}
	} else if (var.type == VAR_PACKAGE) {
		ERROR("Attempt to assign to package `%.*s`", length, name);
	} else {
		ERROR("Assigning to undefined variable `%.*s`", length, name);
	}
}


// Returns true if `token` can begin an expression.
bool expr_exists(TokenType token) {
	return token == TOKEN_IDENTIFIER || token == TOKEN_STRING ||
		token == TOKEN_INTEGER || token == TOKEN_NUMBER ||
		token == TOKEN_TRUE || token == TOKEN_FALSE || token == TOKEN_NIL ||
		token == TOKEN_FN || token == TOKEN_SUB || token == TOKEN_NOT ||
		token == TOKEN_BIT_NOT;
}
