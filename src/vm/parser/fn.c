
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
	fn_new(parser->vm, fn->package, &child.fn_index);
	Function *child_fn = &parser->vm->functions[child.fn_index];
	child_fn->name = name;
	child_fn->length = length;

	// Add `self` if this is a method
	if (is_method) {
		Local *local = local_new(&child, NULL);
		local->name = method_self_name;
		local->length = 4;
		local->scope_depth = 0;
		local->upvalue_index = -1;
		child_fn->arity++;
	}

	// Parse the arguments list into the child parser's locals list
	while (lexer->token.type == TOKEN_IDENTIFIER) {
		// Save the argument
		Local *local = &child.locals[child.locals_count++];
		local->name = lexer->token.start;
		local->length = lexer->token.length;
		local->scope_depth = 0;
		local->upvalue_index = -1;
		lexer_next(lexer);
		child_fn->arity++;

		// Skip a comma
		if (lexer->token.type == TOKEN_COMMA) {
			lexer_next(lexer);
		}
	}

	// Expect a closing parenthesis
	if (lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
		parser_free(&child);
		UNEXPECTED("Expected `)` to close function arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Expect an opening brace to begin the function block
	if (lexer->token.type != TOKEN_OPEN_BRACE) {
		parser_free(&child);
		UNEXPECTED("Expected `{` after arguments list to open function block");
		return 0;
	}
	lexer_next(lexer);

	// Parse the function body
	parse_block(&child, TOKEN_CLOSE_BRACE);

	// Emit a return instruction at the end of the body
	emit(child_fn, instr_new(RET, 0, 0, 0));

	// Expect a closing brace
	if (lexer->token.type != TOKEN_CLOSE_BRACE) {
		parser_free(&child);
		UNEXPECTED("Expected `}` to close function block");
		return 0;
	}
	lexer_next(lexer);

	parser_free(&child);
	return child.fn_index;
}


// Parses a method definition.
// TODO: Method recursion
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

	// Set the default value of the struct field
	def->values[index] = INDEX_TO_VALUE(fn_index, FN_TAG);
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
	emit(fn, instr_new(MOV_LF, slot, fn_index, 0));

	// If this is a top level function
	if (top_level) {
		// Free the scope we created
		scope_free(parser);

		// Create a new top level variable
		uint16_t package_index = fn->package - parser->vm->packages;
		emit(fn, instr_new(MOV_TL, top_level_index, package_index, slot));
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
	Function *fn = &parser->vm->functions[parser->fn_index];

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
			emit(fn, instr_new(MOV_LL, self_slot, self->slot, 0));
		} else if (self->type == SELF_UPVALUE) {
			emit(fn, instr_new(MOV_LU, self_slot, self->slot, 0));
		} else if (self->type == SELF_TOP_LEVEL) {
			emit(fn, instr_new(MOV_LT, self_slot, self->package_index,
				self->slot));
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
	emit(fn, instr_new_4(call, arity, slot, argument_start, return_slot));
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
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Parse the result of the function call into a temporary slot
	uint16_t return_slot;
	scope_new(parser);
	local_new(parser, &return_slot);
	scope_free(parser);

	// Create a new scope for the struct fields we might have to index
	scope_new(parser);

	// Create a `self` variable in case this is a method call
	OperandSelf self;
	self.is_method = false;

	// Move the first element in the left into a variable
	Variable var = local_capture(parser, left[0].start, left[0].length);
	uint16_t previous = var.slot;
	uint16_t slot = var.slot;

	// TODO: Remove duplication between here and parse_assignment
	if (var.type == VAR_PACKAGE) {
		// Find the top level variable
		Package *package = &parser->vm->packages[var.slot];
		char *var_name = left[1].start;
		size_t var_length = left[1].length;
		int pkg_var_index = package_local_find(package, var_name, var_length);
		if (pkg_var_index == -1) {
			ERROR("Attempt to assign to undefined top level variable `%.*s`"
				"in package `%s`", var_length, var_name, package->name);
			return;
		}

		// Move value into a local
		local_new(parser, &slot);
		previous = slot;
		emit(fn, instr_new(MOV_LT, slot, var.slot, pkg_var_index));
		self.type = SELF_TOP_LEVEL;
		self.package_index = var.slot;
		self.slot = pkg_var_index;
	} else if (var.type == VAR_NATIVE_PACKAGE) {
		// Expect only one other element in the variable list
		if (count > 2) {
			ERROR("Expected `()` after identifier in native package function "
				"call");
			return;
		}

		// Look for the native function
		char *name = left[1].start;
		size_t length = left[1].length;
		HyNativePackage *package = &parser->vm->native_packages[var.slot];
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
	} else if (var.type == VAR_LOCAL && count > 1) {
		// If we have to replace this local with one of its fields, allocate
		// a new local for it
		local_new(parser, &slot);
		self.type = SELF_LOCAL;
		self.slot = var.slot;
	} else if (var.type == VAR_UPVALUE || var.type == VAR_TOP_LEVEL) {
		// Create a new local to store the upvalue/top level variable in
		local_new(parser, &slot);
		previous = slot;

		if (var.type == VAR_UPVALUE) {
			emit(fn, instr_new(MOV_LU, slot, var.slot, 0));
			self.type = SELF_UPVALUE;
			self.slot = var.slot;
		} else {
			expr_top_level_to_local(parser, slot, var.slot);
			self.type = SELF_TOP_LEVEL;
			self.slot = var.slot;
			self.package_index = fn->package - parser->vm->packages;
		}
	} else if (var.type == VAR_UNDEFINED) {
		ERROR("Undefined variable `%.*s` in function call", left[0].length,
			left[0].start);
		return;
	}

	// Index all left variables
	for (int i = 1; i < count; i++) {
		// Replace the struct in the current slot with its field
		uint16_t index = vm_add_field(parser->vm, left[i]);
		emit(fn, instr_new(STRUCT_FIELD, slot, previous, index));
		previous = slot;

		// Update the `self` value
		if (self.is_method) {
			// Up to third index (ie. `a.b.c`)
			self.type = SELF_LOCAL;
			self.slot = slot;
		} else {
			self.is_method = true;
		}
	}

	// Parse a function call
	parse_fn_call_self(parser, CALL_L, slot, return_slot, &self);

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
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Skip the `return` token
	lexer_next(lexer);

	// Check for a return value
	if (!expr_exists(lexer->token.type)) {
		// Emit close upvalue instructions for all locals in this function
		local_close_upvalues(parser);

		// No return value
		emit(fn, instr_new(RET, 0, 0, 0));
	} else {
		// Parse expression into a new local
		uint16_t slot;
		scope_new(parser);
		local_new(parser, &slot);
		Operand operand = expr(parser, slot);
		scope_free(parser);

		// Emit close upvalue instructions for all locals in this function
		local_close_upvalues(parser);

		// Emit return instruction
		Opcode opcode = RET_L + operand.type;
		emit(fn, instr_new(opcode, operand.value, 0, 0));
	}
}
