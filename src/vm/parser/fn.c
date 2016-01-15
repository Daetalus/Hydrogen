
//
//  Function Parsing
//

#include "fn.h"
#include "expr.h"
#include "local.h"
#include "../value.h"


// Used as the name for the `self` local added to methods.
char method_self_name[] = "self";



//
//  Definitions
//

// Parses a function definition body (starting at the arguments list) for a
// function with the name `name`. `is_method` should be true if this function
// is a method on a struct. In this case, `self` will be added as the first
// local on the child function's local stack.
uint16_t parse_fn_definition_body(Parser *parser, char *name, size_t length,
		bool is_method) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Expect an opening parenthesis
	EXPECT(TOKEN_OPEN_PARENTHESIS,
		"Expected `(` after function name to begin arguments list");
	lexer_next(lexer);

	// Create the new child parser
	Parser child = parser_new(parser);
	Function *child_fn = fn_new(parser->vm, fn->package, &child.fn_index);
	child_fn->name = name;
	child_fn->length = length;

	// Add `self` if this is a method
	if (is_method) {
		Local *local = local_new(&child, NULL);
		local->name = method_self_name;
		local->length = 4;
		child_fn->arity++;
	}

	// Parse the arguments list into the child parser's locals list
	while (lexer->token.type == TOKEN_IDENTIFIER) {
		// Save the argument
		Local *local = local_new(&child, NULL);
		local->name = lexer->token.start;
		local->length = lexer->token.length;
		child_fn->arity++;
		lexer_next(lexer);

		// Skip a comma
		if (lexer->token.type == TOKEN_COMMA) {
			lexer_next(lexer);
		} else {
			break;
		}
	}

	// Expect a closing parenthesis
	// TODO: Free child parser when expect fails
	EXPECT(TOKEN_CLOSE_PARENTHESIS,
		"Expected `)` to close function arguments list");
	lexer_next(lexer);

	// Expect an opening brace to begin the function block
	// TODO: Free child parser when expect fails
	EXPECT(TOKEN_OPEN_BRACE,
		"Expected `{` after arguments list to open function block");
	lexer_next(lexer);

	// Parse the function body and emit a return instruction at the end of it
	parse_block(&child, TOKEN_CLOSE_BRACE);
	parser_emit(&child, RET0, 0, 0, 0);

	// Expect a closing brace
	// TODO: Free child parser when expect fails
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close function block");
	lexer_next(lexer);

	parser_free(&child);
	return child.fn_index;
}


// Parses a method definition.
void parse_method_definition(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the opening parenthesis
	lexer_next(lexer);

	// Expect the name of a struct
	EXPECT(TOKEN_IDENTIFIER, "Expected struct name in method definition");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Find a struct with the given name
	int def_index = struct_find(parser->vm, name, length);
	if (def_index < 0) {
		ERROR("Attempt to define method on undefined struct `%.*s`", length,
			name);
		return;
	}
	StructDefinition *def = &parser->vm->structs[def_index];

	// Expect a closing parenthesis
	EXPECT(TOKEN_CLOSE_PARENTHESIS, "Expected `)` after struct name");
	lexer_next(lexer);

	// Check if this is a constructor
	if (lexer->token.type == TOKEN_NEW) {
		// Skip the `new` token
		lexer_next(lexer);

		// Parse the remainder of the function and set the struct's constructor
		def->constructor = parse_fn_definition_body(parser, name, length, true);
		return;
	}

	// Expect the name of the method
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `fn`");
	name = lexer->token.start;
	length = lexer->token.length;
	lexer_next(lexer);

	// Parse the remainder of the function
	uint16_t fn_index = parse_fn_definition_body(parser, name, length, true);

	// Create a new field to store the method in
	int index = struct_new_field(def);
	Identifier *field = &def->fields[index];
	field->start = name;
	field->length = length;

	// Set the default value for this field to be the method function
	def->values[index] = FROM_FN(fn_index);
}


// Parses a function or method definition.
void parse_fn_definition(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Skip the `fn` token
	lexer_next(lexer);

	// Check if we're parsing a method definition or not
	if (lexer->token.type == TOKEN_OPEN_PARENTHESIS) {
		parse_method_definition(parser);
		return;
	}

	// Expect an identifier (the name of the function)
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `fn`");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Ensure we haven't already defined this function
	if (local_exists(parser, name, length)) {
		ERROR("Function `%.*s` already defined", length, name);
		return;
	}

	// If this is a top level function
	bool top_level = parser_is_top_level(parser);
	uint32_t top_level_index;
	if (top_level) {
		// Create a new scope so we can discard this local
		scope_new(parser);

		// Create a new top level local
		top_level_index = package_local_new(fn->package, name, length);
	}

	// Create a new local to store the function in
	uint16_t slot;
	Local *local = local_new(parser, &slot);
	local->name = lexer->token.start;
	local->length = lexer->token.length;

	// Parse the remainder of the function
	uint16_t fn_index = parse_fn_definition_body(parser, name, length, false);

	// Since the `parse_fn_definition_body` could have reallocated the VM's
	// functions array, reset the function pointer
	fn = &parser->vm->functions[parser->fn_index];

	// Emit bytecode to store the function into the created local
	parser_emit(parser, MOV_LF, slot, fn_index, 0);

	// If this is a top level function
	if (top_level) {
		// Free the scope we created
		scope_free(parser);

		// Create a new top level variable
		uint16_t package_index = parser_package_index(parser);
		parser_emit(parser, MOV_TL, top_level_index, package_index, slot);
	}
}



//
//  Calls
//

// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
// If the `self` argument has its `is_method` field set to true, then the
// we are calling a method on a struct, and a `self` argument is pushed onto
// the argument's list. The `self` value is reconstructed from the data in the
// given argument.
void parse_fn_call_self(Parser *parser, Opcode call, uint16_t slot,
		uint16_t return_slot, OperandSelf *self) {
	Lexer *lexer = parser->lexer;

	// Expect an opening parenthesis
	EXPECT(TOKEN_OPEN_PARENTHESIS, "Expected `(` after name in function call");
	lexer_next(lexer);

	// Create a new scope for the function arguments
	scope_new(parser);

	// Add the `self` argument
	uint8_t arity = 0;
	uint16_t argument_start = parser->locals_count;
	if (self != NULL && self->type != SELF_NONE && self->is_method) {
		arity++;

		// Move the `self` value into a local
		uint16_t self_slot;
		local_new(parser, &self_slot);

		// Emit bytecode to store the local into the new slot
		if (self->type == SELF_LOCAL) {
			parser_emit(parser, MOV_LL, self_slot, self->slot, 0);
		} else if (self->type == SELF_UPVALUE) {
			parser_emit(parser, MOV_LU, self_slot, self->slot, 0);
		} else if (self->type == SELF_TOP_LEVEL) {
			parser_emit(parser, MOV_LT, self_slot, self->package_index,
				self->slot);
		}
	}

	// Parse function arguments into consecutive local slots
	while (lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
		// Create local for the argument
		uint16_t slot;
		local_new(parser, &slot);

		// Increment the number of arguments we have
		if (arity >= 255) {
			// Since the arity of the function call must be stored in a single
			// byte in the instruction, it cannot be greater than 255
			ERROR("Cannot pass more than 255 arguments to function call");
			return;
		}
		arity++;

		// Expect an expression
		expr_emit(parser, slot);

		// Expect a comma or closing parenthesis
		if (lexer->token.type == TOKEN_COMMA) {
			lexer_next(lexer);
		} else if (lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
			// Unexpected token
			UNEXPECTED("Expected `)` to close arguments list in function call");
			return;
		}
	}

	// Free the scope created for the arguments
	scope_free(parser);

	// Expect a closing parenthesis
	EXPECT(TOKEN_CLOSE_PARENTHESIS,
		"Expected `)` to close arguments list in function call");
	lexer_next(lexer);

	// If there are no arguments, then don't bother with the argument start
	if (arity == 0) {
		argument_start = 0;
	}

	// Emit the function call
	parser_emit_call(parser, call, arity, slot, argument_start, return_slot);
}


// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
		uint16_t return_slot) {
	parse_fn_call_self(parser, call, slot, return_slot, NULL);
}


// Parses a call to a function in a native package. `index` is the index of the
// native package in the VM's native package list. `return_slot` is the location
// to store the return value of the function call.
void parse_native_fn_call(Parser *parser, uint32_t index, uint16_t return_slot) {
	Lexer *lexer = parser->lexer;
	HyNativePackage *package = &parser->vm->native_packages[index];

	// Skip the package name
	lexer_next(lexer);

	// Expect a dot
	EXPECT(TOKEN_DOT, "Expected `.` after native package name `%s`",
		package->name);
	lexer_next(lexer);

	// Expect an identifier
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `.` in native package "
		"function call");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Look for the native function
	int fn_index = native_fn_find(package, name, length);
	if (fn_index == -1) {
		ERROR("Undefined native function `%.*s` on native package `%s`", length,
			name, package->name);
		return;
	}

	// Parse the rest of the function call
	parse_fn_call_slot(parser, CALL_NATIVE, fn_index, return_slot);
}


// Parses a function call, starting at the opening parenthesis of the arguments
// list.
void parse_fn_call(Parser *parser, Identifier *left, int count) {
	// Parse the result of the function call into a temporary slot
	uint16_t return_slot;
	scope_new(parser);
	local_new(parser, &return_slot);
	scope_free(parser);

	// Create a new scope for the struct fields we might have to index
	scope_new(parser);

	// Create a `self` variable in case this is a method call
	OperandSelf self;
	self.is_method = (count > 1);

	// Move the first element in the left into a variable
	char *root_name = left[0].start;
	size_t root_length = left[0].length;
	Variable var = local_capture(parser, root_name, root_length);
	uint16_t root_slot;

	switch (var.type) {
	case VAR_NATIVE_PACKAGE: {
		// Parse a native package call only
		if (count > 2) {
			// Expect only one other element in the variable list
			ERROR("Expected `()` after identifier in native function call");
			return;
		}

		char *name = left[1].start;
		size_t length = left[1].length;
		HyNativePackage *package = &parser->vm->native_packages[var.slot];

		// Look for the native function
		int fn_index = native_fn_find(package, name, length);
		if (fn_index == -1) {
			ERROR("Undefined native function `%.*s` on native package `%s`",
				length, name, package->name);
			return;
		}

		// Parse the rest of the function call
		parse_fn_call_slot(parser, CALL_NATIVE, fn_index, return_slot);

		// Free the scope we allocated for the struct fields
		scope_free(parser);
		return;
	}

	case VAR_PACKAGE: {
		uint16_t package_index = var.slot;
		Package *package = &parser->vm->packages[package_index];
		char *field_name = left[1].start;
		size_t field_length = left[1].length;

		// Find the top level variable
		int field_index = package_local_find(package, field_name, field_length);
		if (field_index == -1) {
			ERROR("Attempt to assign to undefined top level variable `%.*s`"
				"in package `%s`", field_length, field_name, package->name);
			return;
		}

		// Move value into a local
		local_new(parser, &root_slot);
		parser_emit(parser, MOV_LT, root_slot, package_index, field_index);

		self.type = SELF_TOP_LEVEL;
		self.package_index = package_index;
		self.slot = field_index;
		break;
	}

	case VAR_UPVALUE:
		// Store the upvalue into a new local
		local_new(parser, &root_slot);
		parser_emit(parser, MOV_LU, root_slot, var.slot, 0);
		self.type = SELF_UPVALUE;
		self.slot = var.slot;
		break;

	case VAR_TOP_LEVEL: {
		// Store the top level variable into a new local
		uint16_t package_index = parser_package_index(parser);
		local_new(parser, &root_slot);
		parser_emit(parser, MOV_LT, root_slot, package_index, var.slot);

		self.type = SELF_TOP_LEVEL;
		self.slot = var.slot;
		self.package_index = package_index;
		break;
	}

	case VAR_UNDEFINED:
		// Undefined variable
		ERROR("Undefined variable `%.*s` in function call", root_length,
			root_name);
		return;

	default:
		root_slot = var.slot;
		self.type = SELF_LOCAL;
		self.slot = root_slot;
		break;
	}

	// Index all left variables
	for (int i = 1; i < count; i++) {
		// Replace the struct in the current slot with its field
		uint16_t index = vm_add_field(parser->vm, left[i]);
		parser_emit(parser, STRUCT_FIELD, root_slot, root_slot, index);
	}

	// Parse a function call
	parse_fn_call_self(parser, CALL_L, root_slot, return_slot, &self);

	// Free the scope we created for struct fields
	scope_free(parser);
}



//
//  Return Statements
//

// Parses a return statement.
// TODO: Trigger error if we try and return a value from a custom constructor
void parse_return(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `return` token
	lexer_next(lexer);

	// Check for a return value
	Opcode opcode;
	uint16_t slot;
	if (expr_exists(lexer->token.type)) {
		// Parse expression into a new local
		slot = expr_emit_temporary(parser);
		opcode = RET1;
	} else {
		slot = 0;
		opcode = RET0;
	}

	// Emit close upvalue instructions for all locals in this function
	local_close_upvalues(parser);

	// Return instruction
	parser_emit(parser, opcode, slot, 0, 0);
}
