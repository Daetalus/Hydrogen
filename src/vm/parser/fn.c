
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

	// Expect an opening parenthesis
	if (lexer->token.type != TOKEN_OPEN_PARENTHESIS) {
		UNEXPECTED("Expected `(` after function name to begin arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Create the new child parser
	uint16_t fn_index;
	Parser child = parser_new(parser);
	child.fn = fn_new(parser->vm, parser->fn->package, &fn_index);
	child.fn->name = name;
	child.fn->length = length;

	// Add `self` if this is a method
	if (is_method) {
		Local *local = &child.locals[child.locals_count++];
		local->name = method_self_name;
		local->length = 4;
		local->scope_depth = 0;
		local->upvalue_index = -1;
		child.fn->arity++;
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
		child.fn->arity++;

		// Skip a comma
		if (lexer->token.type == TOKEN_COMMA) {
			lexer_next(lexer);
		}
	}

	// Expect a closing parenthesis
	if (lexer->token.type != TOKEN_CLOSE_PARENTHESIS) {
		UNEXPECTED("Expected `)` to close function arguments list");
		return 0;
	}
	lexer_next(lexer);

	// Expect an opening brace to begin the function block
	if (lexer->token.type != TOKEN_OPEN_BRACE) {
		UNEXPECTED("Expected `{` after arguments list to open function block");
		return 0;
	}
	lexer_next(lexer);

	// Parse the function body
	parse_block(&child, TOKEN_CLOSE_BRACE);

	// Emit a return instruction at the end of the body
	emit(child.fn, instr_new(RET, 0, 0, 0));

	// Expect a closing brace
	if (lexer->token.type != TOKEN_CLOSE_BRACE) {
		UNEXPECTED("Expected `}` to close function block");
		return 0;
	}
	lexer_next(lexer);

	return fn_index;
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
	StructDefinition *def = struct_find(parser->vm, name, length, NULL);
	if (def == NULL) {
		ERROR("Attempt to define method on undefined struct `%.*s`", length,
			name);
		return;
	}

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

	// Parse the remainder of the function
	uint16_t fn_index = parse_fn_definition_body(parser, name, length, false);

	// Create a new local to store the function in
	uint16_t slot;
	Local *local = local_new(parser, &slot);
	if (local == NULL) {
		return;
	}
	local->name = name;
	local->length = length;

	// Emit bytecode to store the function into the created local
	emit(parser->fn, instr_new(MOV_LF, slot, fn_index, 0));
}



//
//  Calls
//

// Parses a call to the function in `slot`, storing the return value in
// `return_slot`. Starts at the opening parenthesis of the arguments list.
// If we're calling a method on a struct, the `self` argument is supplied in
// the stack slot specified by `self_slot`.
void parse_fn_call_slot(Parser *parser, Opcode call, uint16_t slot,
		uint16_t return_slot, int self_slot) {
	Lexer *lexer = parser->lexer;

	// Skip the opening parenthesis
	lexer_next(lexer);

	// Create a new scope for the function arguments
	scope_new(parser);

	// Add the `self` argument
	uint8_t arity = 0;
	if (self_slot >= 0) {
		// Create a local for the argument
		uint16_t slot;
		local_new(parser, &slot);

		// Move the `self` value into this slot
		emit(parser->fn, instr_new(MOV_LL, slot, self_slot, 0));
		arity++;
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

	// Skip the closing parenthesis
	lexer_next(lexer);

	// Call the function
	uint16_t arg_start = (arity == 0) ? 0 : parser->locals_count;
	emit(parser->fn, instr_new_4(call, arity, slot, arg_start, return_slot));
}


// Parses a function call, starting at the opening parenthesis of the arguments
// list. `ident` is the name of the function to call.
void parse_fn_call(Parser *parser, Token ident) {
	// Parse the result of the function call into a temporary slot
	uint16_t return_slot;
	scope_new(parser);
	local_new(parser, &return_slot);
	scope_free(parser);

	// Find a local with the name of the function
	char *name = ident.start;
	size_t length = ident.length;
	Variable var = local_capture(parser, name, length);
	if (var.type == VAR_LOCAL) {
		parse_fn_call_slot(parser, CALL_L, var.slot, return_slot, -1);
	} else if (var.type == VAR_UPVALUE) {
		// Store the upvalue into a temporary local
		uint16_t slot;
		scope_new(parser);
		local_new(parser, &slot);
		emit(parser->fn, instr_new(MOV_LU, slot, var.slot, 0));

		// Call the function
		parse_fn_call_slot(parser, CALL_L, slot, return_slot, -1);

		// Free the temporary local
		scope_free(parser);
	} else {
		// Undefined function
		ERROR("Attempt to call undefined function `%.*s`", length, name);
	}
}



//
//  Return Statements
//

// Parses a return statement.
void parse_return(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `return` token
	lexer_next(lexer);

	// Check for a return value
	if (!expr_exists(lexer->token.type)) {
		// Emit close upvalue instructions for all locals in this function
		local_close_upvalues(parser);

		// No return value
		emit(parser->fn, instr_new(RET, 0, 0, 0));
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
		emit(parser->fn, instr_new(opcode, operand.value, 0, 0));
	}
}
