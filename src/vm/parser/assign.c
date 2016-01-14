
//
//  Assignment Parsing
//

#include "assign.h"
#include "expr.h"
#include "local.h"
#include "../bytecode.h"


// Parses an assignment to a new variable (using a `let` token).
void parse_initial_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

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
		int index = package_local_new(fn->package, name, length);

		// Store the result of the expression into the top level variable
		uint16_t package_index = parser_package_index(parser);
		parser_emit(parser, MOV_TL, index, package_index, slot);
	} else {
		// Save the local's name
		local->name = name;
		local->length = length;
	}
}


// Assigns an expression directly to a local (with no field access).
static void assign_local(Parser *parser, char *name, size_t length) {
	Variable var = local_capture(parser, name, length);
	if (var.type == VAR_LOCAL) {
		// Parse an expression directly into the variable's slot
		expr_emit(parser, var.slot);
	} else if (var.type == VAR_UPVALUE || var.type == VAR_TOP_LEVEL) {
		// Parse an expression into an empty local slot
		uint16_t index = var.slot;
		uint16_t slot = expr_emit_temporary(parser);

		if (var.type == VAR_UPVALUE) {
			// Store the local into an upvalue
			parser_emit(parser, MOV_UL, index, slot, 0);
		} else {
			// Store the local into a top level variable
			uint16_t package_index = parser_package_index(parser);
			parser_emit(parser, MOV_TL, index, package_index, slot);
		}
	} else if (var.type == VAR_PACKAGE) {
		ERROR("Attempt to assign to package `%.*s`", length, name);
	} else {
		ERROR("Assigning to undefined variable `%.*s`", length, name);
	}
}


// Assigns an expression to a struct field.
static void assign_struct_field(Parser *parser, Identifier *left, int count,
		uint16_t struct_slot) {
	// If we're not directly assigning to a field in the struct
	// Eg. `struct.field1.field2... = ...`
	if (count > 2) {
		// Create a new slot for the field
		uint16_t field_slot;
		local_new(parser, &field_slot);

		// Emit struct field access for all variables except the last one
		for (int i = 1; i < count - 1; i++) {
			// Emit a field access
			uint16_t index = vm_add_field(parser->vm, left[i]);
			parser_emit(parser, STRUCT_FIELD, field_slot, struct_slot, index);

			// The next field access will be on the field we just emitted
			struct_slot = field_slot;
		}
	}

	// Parse an expression into a temporary local
	uint16_t expr_slot;
	local_new(parser, &expr_slot);
	Operand operand = expr(parser, expr_slot);

	uint16_t result_slot;
	if (operand.type == OP_LOCAL) {
		// Normally, discharging an operand will emit a MOV_LL instruction which
		// is unnecessary in this case, so just store the local directly
		result_slot = operand.slot;
	} else {
		// Need to discharge the operand into a local
		expr_discharge(parser, expr_slot, operand);
		result_slot = expr_slot;
	}

	// Set the field of the struct in `slot` to `operand`
	uint16_t index = vm_add_field(parser->vm, left[count - 1]);
	parser_emit(parser, STRUCT_SET, struct_slot, index, result_slot);
}


// Parses an assignment to an already initialised variable.
// TODO: Add modifier assignment token support
void parse_assignment(Parser *parser, Identifier *left, int count) {
	Lexer *lexer = parser->lexer;

	// Skip the assignment token
	lexer_next(lexer);

	// If we're only assigning to a single variable
	if (count == 1) {
		assign_local(parser, left[0].start, left[0].length);
		return;
	}

	// There must be at least 2 elements in the left list now
	// Assigning to a struct field or top level local on another package
	scope_new(parser);

	// Capture the first element in the left list
	char *struct_name = left[0].start;
	size_t struct_length = left[0].length;
	uint16_t struct_slot;
	Variable var = local_capture(parser, struct_name, struct_length);

	switch (var.type) {
	case VAR_PACKAGE: {
		// Either assigning to a top level local in an external package, or
		// to a struct field on a top level local in an external package
		// Eg. `pkg.top_level = ...` or `pkg.top_level.struct_field = ...`
		uint32_t package_index = var.slot;
		Package *package = &parser->vm->packages[package_index];

		// Find the top level variable in the package from the first element
		// after the first `.` in the left hand side variable list
		char *field_name = left[1].start;
		size_t field_length = left[1].length;
		int field_index = package_local_find(package, field_name, field_length);
		if (field_index == -1) {
			ERROR("Attempt to assign to undefined top level variable `%.*s`"
				"in package `%s`", field_length, field_name, package->name);
			return;
		}

		if (count == 2) {
			// Assigning directly to a top level variable
			// Eg. `pkg.top_level = ...`
			scope_free(parser);

			// Parse the expression into a temporary slot and store it into
			// the top level local
			uint16_t slot = expr_emit_temporary(parser);
			parser_emit(parser, MOV_TL, field_index, package_index, slot);
			return;
		} else {
			// Assigning to a struct field on a top level variable
			// Store the top level variable into a new slot
			local_new(parser, &struct_slot);
			parser_emit(parser, MOV_LT, struct_slot, package_index, field_index);
			break;
		}
	}

	case VAR_UPVALUE:
		// Move the upvalue into a slot
		local_new(parser, &struct_slot);
		parser_emit(parser, MOV_LU, struct_slot, var.slot, 0);
		break;

	case VAR_TOP_LEVEL: {
		// Move the top level local into a slot
		uint16_t package_index = parser_package_index(parser);
		local_new(parser, &struct_slot);
		parser_emit(parser, MOV_LT, struct_slot, package_index, var.slot);
		break;
	}

	case VAR_UNDEFINED:
		// Undefined variable
		ERROR("Assigning to undefined variable `%.*s`", struct_length,
			struct_name);
		return;

	default:
		struct_slot = var.slot;
		break;
	}

	// Assign an expression to the struct field
	assign_struct_field(parser, left, count, struct_slot);

	// Free the scope we created
	scope_free(parser);
}
