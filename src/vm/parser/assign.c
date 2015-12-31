
//
//  Assignment Parsing
//

#include "assign.h"
#include "expr.h"
#include "local.h"
#include "../bytecode.h"


// The maximum number of identifiers that can exist on the left hand side of an
// assignment.
#define MAX_ASSIGN_DEPTH 64


// Parses an assignment to a new variable (using a `let` token).
void parse_initial_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Consume the `let` token
	lexer_next(lexer);

	// Expect an identifier
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `let`");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Check the variable isn't already defined
	if (local_exists(parser, name, length)) {
		ERROR("Variable `%.*s` is already defined", length, name);
		return;
	}

	// Expect an equals sign
	EXPECT(TOKEN_ASSIGN, "Expected `=` after identifier in assignment");
	lexer_next(lexer);

	// Create a new local
	uint16_t slot;
	Local *local = local_new(parser, &slot);

	// Expect an expression
	expr_emit(parser, slot);

	// Save the local's name
	local->name = name;
	local->length = length;
}


// Parses an assignment to an already initialised variable.
void parse_assignment(Parser *parser, Token name) {
	Lexer *lexer = parser->lexer;

	Identifier left[MAX_ASSIGN_DEPTH];
	int count = 0;

	// Parse left hand side variable list
	while (lexer->token.type == TOKEN_DOT) {
		lexer_next(lexer);

		// Expect an identifier
		EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `.` in assignment");
		left[count++].start = lexer->token.start;
		left[count - 1].length = lexer->token.length;
		lexer_next(lexer);
	}

	// Expect an assignment operator
	if (lexer->token.type < TOKEN_ASSIGN ||
			lexer->token.type > TOKEN_DIV_ASSIGN) {
		UNEXPECTED("Expected `=` after identifier in assignment");
		return;
	}

	// Save the token used to perform the assignment
	// TODO: Add modifier assignment token support (currently failing test)
	TokenType assign = lexer->token.type;
	lexer_next(lexer);

	// If we're only assigning to a single variable
	if (count == 0) {
		expr_emit_local(parser, name.start, name.length);
		return;
	}

	// Assigning to a struct field
	scope_new(parser);

	// Move the first element in the left variable list into a local
	Variable var = local_capture(parser, name.start, name.length);
	uint16_t previous = var.slot;
	uint16_t slot = var.slot;
	if (var.type == VAR_LOCAL && count > 1) {
		// If this is a local and we have to replace this local with one of its
		// fields in the next step, allocate a new stack position for this local
		previous = var.slot;
		local_new(parser, &slot);
	} else if (var.type == VAR_UPVALUE) {
		// Store the upvalue into a new local
		local_new(parser, &slot);
		previous = slot;
		emit(parser->fn, instr_new(MOV_LU, slot, var.slot, 0));
	} else if (var.type == VAR_UNDEFINED) {
		ERROR("Assigning to undefined variable `%.*s`", name.length, name.start);
		return;
	}

	// Index all left hand variables except the last one
	for (int i = 0; i < count - 1; i++) {
		// Replace the struct that's in the slot at the moment
		uint16_t index = vm_add_field(parser->vm, left[i]);
		emit(parser->fn, instr_new(STRUCT_FIELD, slot, previous, index));
		previous = slot;
	}

	// Parse an expression into a temporary local
	uint16_t expr_slot;
	local_new(parser, &expr_slot);
	Operand operand = expr(parser, expr_slot);
	if (operand.type == OP_JUMP) {
		expr_discharge(parser, expr_slot, operand);
	}

	// Set the field of the struct in `slot` to `operand`
	Opcode opcode = STRUCT_SET_L + operand.type;
	uint16_t index = vm_add_field(parser->vm, left[count - 1]);
	emit(parser->fn, instr_new(opcode, slot, index, operand.value));

	// Free the scope we created
	scope_free(parser);
}
