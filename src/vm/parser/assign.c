
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

	// If this is a top level variable
	bool top_level = parser_is_top_level(parser);
	if (top_level) {
		// Create a new scope so we can discard this local
		scope_new(parser);
	}

	// Parse an expression into a new local
	uint16_t slot;
	Local *local = local_new(parser, &slot);
	expr_emit(parser, slot);

	// If this is a top level variable
	if (top_level)  {
		// Free the scope to discard this local
		scope_free(parser);

		// Create a new top level variable
		int index = package_local_new(parser->fn->package, name, length);

		// Store the result of the expression into the top level variable
		uint16_t package_index = (parser->fn->package - parser->vm->packages) /
			sizeof(Package);
		emit(parser->fn, instr_new(MOV_TL, index, package_index, slot));
	} else {
		// Save the local's name
		local->name = name;
		local->length = length;
	}
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
	if (var.type == VAR_PACKAGE) {
		// Find the top level variable in the package from the first element
		// after the first `.` in the left hand side variable list
		Package *package = &parser->vm->packages[var.slot];
		char *var_name = left[0].start;
		size_t var_length = left[0].length;
		int pkg_var_index = package_local_find(package, var_name, var_length);
		if (pkg_var_index == -1) {
			ERROR("Attempt to assign to undefined top level variable `%.*s`"
				"in package `%s`", var_length, var_name, package->name);
			return;
		}

		// If we're assigning directly to a top level variable
		if (count == 1) {
			// Parse an expression into an empty slot
			local_new(parser, &slot);
			expr_emit(parser, slot);
			scope_free(parser);

			// Move the result into the top level variable
			emit(parser->fn, instr_new(MOV_TL, pkg_var_index, var.slot, slot));
			return;
		} else {
			// Assigning to a struct field on a top level variable
			// Store the top level variable into a new slot
			local_new(parser, &slot);
			previous = slot;
			emit(parser->fn, instr_new(MOV_LT, slot, var.slot, pkg_var_index));
		}
	} else if (var.type == VAR_LOCAL && count > 1) {
		// If this is a local and we have to replace this local with one of its
		// fields in the next step, allocate a new stack position for this local
		local_new(parser, &slot);
	} else if (var.type == VAR_UPVALUE || var.type == VAR_TOP_LEVEL) {
		// Create a new local to store the upvalue/top level variable in
		local_new(parser, &slot);
		previous = slot;

		if (var.type == VAR_UPVALUE) {
			emit(parser->fn, instr_new(MOV_LU, slot, var.slot, 0));
		} else {
			uint16_t package_index = (parser->fn->package -
				parser->vm->packages) / sizeof(Package);
			emit(parser->fn, instr_new(MOV_LT, slot, package_index, var.slot));
		}
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
