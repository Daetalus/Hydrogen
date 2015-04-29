
//
//  Compiler
//


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "lib/operator.h"
#include "expression.h"
#include "compiler.h"
#include "error.h"


// Compile a block. Assumes the opening token for the block has
// been consumed. Stops when the terminating token is found.
//
// Does not consume the terminating token.
void block(Compiler *compiler, TokenType terminator);

// Compile a single statement. A statement is one construct in
// the language, like an if statement or variable assignment.
void statement(Compiler *compiler);

// Returns true if a variable with the name `name` exists.
bool variable_exists(Compiler *compiler, char *name, int length);

// Compilation functions for each type of statement.
bool identifier(Compiler *compiler);
void variable_assignment(Compiler *compiler);
void if_statement(Compiler *compiler);
void while_loop(Compiler *compiler);
void infinite_loop(Compiler *compiler);
void break_statement(Compiler *compiler);
void function_definition(Compiler *compiler);
void expression_statement(Compiler *compiler);
void return_statement(Compiler *compiler);
void class_definition(Compiler *compiler);

// Returns true if the lexer matches a variable assignment.
// Matches an identifier, followed by an assignment operator, or
// a let keyword.
bool match_variable_assignment(Lexer *lexer);

// Iterate over the compiler's locals and close any upvalues.
void close_captured_locals(Compiler *compiler);

// Increment the compiler's scope depth.
void push_scope(Compiler *compiler);

// Decrement the compiler's scope depth, and pop off any local
// variables from the stack that are no longer in scope.
void pop_scope(Compiler *compiler);

// Creates a new local on the compiler. Returns the index of the
// new local in the compiler's index list.
int define_local(Compiler *compiler, char *name, int length);

// Returns the appropriate instruction for storing a variable
// of type `type`.
void emit_store_variable(Bytecode *bytecode, Variable *variable);

// Stores a function onto the stack.
void emit_store_function(Bytecode *bytecode, int fn_index, int local_index);



//
//  Compilation
//

// Compile source code into bytecode, using the lexer in the
// virtual machine `vm` as input. Outputs bytecode directly into
// `fn`'s bytecode array.
//
// Stops compiling when `terminator` is found, or end of file is
// reached.
void compile(VirtualMachine *vm, Compiler *parent, Function *fn,
		TokenType terminator) {
	// Create a compiler for this function.
	Compiler compiler;
	compiler.vm = vm;
	compiler.parent = parent;
	compiler.fn = fn;
	compiler.local_count = 0;
	compiler.scope_depth = 0;
	compiler.loop_count = 0;
	compiler.explicit_return_statement = false;

	// Push the function's arguments as locals
	for (int i = 0; i < fn->arity; i++) {
		Local *local = &compiler.locals[compiler.local_count++];
		local->name = fn->arguments[i].location;
		local->length = fn->arguments[i].length;
		local->scope_depth = compiler.scope_depth;
		local->upvalue_index = -1;
	}

	// Treat the source code as a top level block without a
	// scope, stopping when we reach the terminator character.
	Lexer *lexer = &compiler.vm->lexer;
	while (!lexer_match(lexer, terminator) &&
			!lexer_match(lexer, TOKEN_END_OF_FILE)) {
		statement(&compiler);
	}

	// Insert a final return instruction, pushing nil as the
	// return value.
	if (!compiler.explicit_return_statement) {
		close_captured_locals(&compiler);
		Bytecode *bytecode = &compiler.fn->bytecode;
		emit(bytecode, CODE_PUSH_NIL);
		emit(bytecode, CODE_RETURN);
	}
}



//
//  Blocks and Statements
//

// Compile a block. Assumes the opening token for the block has
// been consumed. Stops when the terminating token is found.
//
// Does not consume the terminating token.
void block(Compiler *compiler, TokenType terminator) {
	Lexer *lexer = &compiler->vm->lexer;

	// Blocks define scopes, so push a new scope when we start
	// parsing this block, and pop it when we're finished.
	push_scope(compiler);

	while (!lexer_match(lexer, terminator) &&
			!lexer_match(lexer, TOKEN_END_OF_FILE)) {
		// Blocks consist of a sequence of statements.
		statement(compiler);
	}

	pop_scope(compiler);
}


// Compile a single statement. A statement is one construct in
// the language, like an if statement or variable assignment.
void statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	if (lexer_match(lexer, TOKEN_LINE)) {
		// Ignore empty lines
		lexer_consume(lexer);
	} else if (match_variable_assignment(lexer)) {
		variable_assignment(compiler);
	} else if (lexer_match(lexer, TOKEN_IF)) {
		if_statement(compiler);
	} else if (lexer_match(lexer, TOKEN_WHILE)) {
		while_loop(compiler);
	} else if (lexer_match(lexer, TOKEN_LOOP)) {
		infinite_loop(compiler);
	} else if (lexer_match(lexer, TOKEN_BREAK)) {
		break_statement(compiler);
	} else if (lexer_match(lexer, TOKEN_FUNCTION)) {
		function_definition(compiler);
	} else if (lexer_match(lexer, TOKEN_RETURN)) {
		return_statement(compiler);
	} else if (lexer_match(lexer, TOKEN_CLASS)) {
		class_definition(compiler);
	} else {
		expression_statement(compiler);
	}
}


// Returns true if the token is an assignment operator.
bool is_assignment_operator(TokenType token) {
	return token == TOKEN_ASSIGNMENT ||
		token == TOKEN_ADDITION_ASSIGNMENT ||
		token == TOKEN_SUBTRACTION_ASSIGNMENT ||
		token == TOKEN_DIVISION_ASSIGNMENT ||
		token == TOKEN_MODULO_ASSIGNMENT ||
		token == TOKEN_MULTIPLICATION_ASSIGNMENT;
}


// Returns true if the lexer matches a variable assignment.
// Matches an identifier, followed by an assignment operator, or
// a let keyword.
bool match_variable_assignment(Lexer *lexer) {
	// Check for a let keyword, which automatically means we're
	// assigning a variable.
	if (lexer_match(lexer, TOKEN_LET)) {
		return true;
	}

	// Check for an identifier, followed by an assignment
	// operator.
	if (lexer_match(lexer, TOKEN_IDENTIFIER)) {
		lexer_disable_newlines(lexer);
		Token token = lexer_peek(lexer, 1);
		lexer_enable_newlines(lexer);
		if (is_assignment_operator(token.type)) {
			return true;
		}
	}

	return false;
}


// Returns true if the lexer matches a function call. Matches an
// identifier followed by an open parethesis.
bool match_function_call(Lexer *lexer) {
	lexer_disable_newlines(lexer);
	bool result = lexer_match_two(lexer, TOKEN_IDENTIFIER,
		TOKEN_OPEN_PARENTHESIS);
	lexer_enable_newlines(lexer);

	return result;
}



//
//  Variable Assignment
//

// Compile a variable assignment.
void variable_assignment(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Indicates whether the variable we're assigning to has
	// been defined before, or whether we're defining it for the
	// first time.
	bool is_new_var = false;

	// Check for the let keyword.
	if (lexer_match(lexer, TOKEN_LET)) {
		is_new_var = true;

		// Consume the let keyword.
		lexer_consume(lexer);
	}

	// Ignore newlines until the expression
	lexer_disable_newlines(lexer);

	// Expect an identifier (the variable's name).
	Token name = expect(lexer, TOKEN_IDENTIFIER,
		"Expected variable name in assignment");
	if (name.type == TOKEN_NONE) {
		return;
	}

	// Check to see if the variable already exists.
	//
	// Allow the redefinition of locals (using `let`) over
	// potential upvalues.
	Variable variable = capture_variable(compiler, name.location, name.length);
	if (is_new_var && variable.type == VARIABLE_LOCAL) {
		// We're trying to create a new variable using a
		// variable name that's already taken.
		error(lexer->line, "Variable name `%.*s` already in use",
			name.length, name.location);
	} else if (!is_new_var && variable.type == VARIABLE_UNDEFINED) {
		// We're trying to assign a new value to an undefined
		// variable.
		error(lexer->line,
			"Undefined variable `%.*s`. Use `let` to define a new variable",
			name.length, name.location);
	}

	// Expect an assignment sign. If we find something other
	// than a normal equals sign, we need to perform some sort
	// of modification.
	NativeFunction modifier_fn = NULL;
	if (lexer_match(lexer, TOKEN_ADDITION_ASSIGNMENT)) {
		modifier_fn = &operator_addition;
	} else if (lexer_match(lexer, TOKEN_SUBTRACTION_ASSIGNMENT)) {
		modifier_fn = &operator_subtraction;
	} else if (lexer_match(lexer, TOKEN_MULTIPLICATION_ASSIGNMENT)) {
		modifier_fn = &operator_multiplication;
	} else if (lexer_match(lexer, TOKEN_DIVISION_ASSIGNMENT)) {
		modifier_fn = &operator_division;
	} else if (lexer_match(lexer, TOKEN_MODULO_ASSIGNMENT)) {
		modifier_fn = &operator_modulo;
	} else if (lexer_match(lexer, TOKEN_ASSIGNMENT)) {
		// No modification needed, but we don't want to trigger
		// an error.
	} else {
		// Missing an assignment operator
		error(lexer->line, "Expected `=` after `%.*s` in assignment",
			name.length, name.location);
	}

	// Disallow modifier operators on new variables (ie. ones
	// with the `let` keyword)
	if (is_new_var && modifier_fn != NULL) {
		error(lexer->line,
			"Expected `=` after `%.*s` in assignment of new variable",
			name.length, name.location);
	}

	// Consume the assignment sign
	lexer_consume(lexer);

	if (modifier_fn != NULL) {
		// Push the variable for the modifier function
		emit_push_variable(bytecode, &variable);
	}

	// Compile the expression that follows the assignment sign
	lexer_enable_newlines(lexer);
	Expression expression = expression_new(compiler, NULL);
	expression_compile(&expression);

	if (modifier_fn != NULL) {
		// Push a call to the modifier function.
		emit_call_native(bytecode, modifier_fn);
	}

	if (is_new_var) {
		// We're assigning to a new variable, so we need a new
		// local index for it.
		variable.index = define_local(compiler, name.location, name.length);
		variable.type = VARIABLE_LOCAL;
	}

	emit_store_variable(bytecode, &variable);
}



//
//  If Statement
//

// Returns true if an if statement's or while loop's conditional
// expression should be terminated at `token`.
bool should_terminate_at_open_brace(Token token) {
	return token.type == TOKEN_OPEN_BRACE;
}


// Compile the part of an if or else if statement where we have
// a conditional expression followed by a block. Emits bytecode
// for the expression, a conditional jump and code for the
// block.
//
// Expects the lexer to start on the first token of the
// expression.
//
// Returns the index of the conditional jump emitted, which can
// be patched after the final jump statement (after an if or
// else if to jump to the end of entire statement).
int if_condition_and_block(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Expect an expression, terminated by the opening brace of
	// the block.
	// Leaves the result of the conditional expression on the
	// top of the stack.
	Expression expression = expression_new(compiler,
		&should_terminate_at_open_brace);
	expression_compile(&expression);

	// Emit a conditional jump instruction with a default
	// argument.
	// We'll patch the jump instruction once we know how big the
	// if statement's block is.
	int jump = emit_jump(bytecode, CODE_JUMP_IF_NOT);

	// Consume the opening brace of the if statement's block.
	lexer_disable_newlines(&compiler->vm->lexer);
	expect(lexer, TOKEN_OPEN_BRACE,
		"Expected `{` after conditional expression in if statement");

	// Compile the block.
	lexer_enable_newlines(&compiler->vm->lexer);
	block(compiler, TOKEN_CLOSE_BRACE);

	// Consume the closing brace.
	expect(lexer, TOKEN_CLOSE_BRACE,
		"Expected `}` to close if statement block");

	return jump;
}


// Compile an if statement.
// TODO: Split into multiple functions
void if_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the if keyword.
	lexer_consume(lexer);

	// Compile the conditional expression and block.
	int previous_jump = if_condition_and_block(compiler);

	// Store all the unpatched jump statements at the end of if
	// or else if blocks so we can patch all of them once we've
	// compiled the entire statement.
	int unpatched_jumps[MAX_ELSE_IF_STATEMENTS];
	int jump_count = 0;

	// Check for multiple else if statements after the if
	// statement.
	bool had_else = false;
	bool had_else_if = false;
	lexer_disable_newlines(lexer);

	while (lexer_match(lexer, TOKEN_ELSE_IF)) {
		had_else_if = true;

		// Firstly append another instruction to the previous if
		// or else if statement's block. The instruction jumps
		// from the end of that block to after the entire
		// if/elseif/else statement.
		//
		// Because we need to compile all of the else if and
		// else blocks before we can patch these jump
		// instructions, store them in an array.
		unpatched_jumps[jump_count] = emit_jump(bytecode, CODE_JUMP_FORWARD);
		jump_count++;

		// Check we haven't gone over the else if limit
		if (jump_count > MAX_ELSE_IF_STATEMENTS) {
			error(lexer->line,
				"Cannot have more than %d else if statements after an if",
				MAX_ELSE_IF_STATEMENTS);
		}

		// Now that we've added the very last thing to the if
		// statement's block, we can patch it's conditional to
		// point here.
		patch_forward_jump(bytecode, previous_jump);

		// Consume the else if token
		lexer_consume(lexer);

		// Compile the conditional expression and block
		lexer_enable_newlines(lexer);
		previous_jump = if_condition_and_block(compiler);
		lexer_disable_newlines(lexer);
	}

	// If there's an else block to follow
	if (lexer_match(lexer, TOKEN_ELSE)) {
		had_else = true;

		// Consume the else token
		lexer_consume(lexer);

		// Emit an unpatched jump instruction for the if/elseif
		// statement that preceded this else statement.
		unpatched_jumps[jump_count] = emit_jump(bytecode, CODE_JUMP_FORWARD);
		jump_count++;

		// Check we haven't gone over the else if limit
		if (jump_count > MAX_ELSE_IF_STATEMENTS) {
			error(lexer->line,
				"Exceeded maximum number (%d) of else/else ifs after if",
				MAX_ELSE_IF_STATEMENTS);
		}

		// Patch the previous jump statement
		patch_forward_jump(bytecode, previous_jump);

		// Compile the else statement's block
		expect(lexer, TOKEN_OPEN_BRACE,
			"Expected `{` after `else`");
		lexer_enable_newlines(lexer);
		block(compiler, TOKEN_CLOSE_BRACE);
		expect(lexer, TOKEN_CLOSE_BRACE,
			"Expected `}` to close else statement block");
	} else {
		lexer_enable_newlines(lexer);
	}

	if (!had_else && !had_else_if) {
		// There were no else or else if statements, so the
		// original if's conditional still hasn't been patched.
		patch_forward_jump(bytecode, previous_jump);
	}

	// We've compiled the entire statement now, so patch all the
	// unpatched jump statements to point here.
	for (int i = 0; i < jump_count; i++) {
		patch_forward_jump(bytecode, unpatched_jumps[i]);
	}
}



//
//  While Loops
//

// Create a new loop at the compiler's current scope depth and
// push it onto the loop stack.
void push_loop(Compiler *compiler, Loop *loop) {
	loop->break_statement_count = 0;
	loop->scope_depth = compiler->scope_depth;
	compiler->loops[compiler->loop_count++] = loop;
}


// Pop the top most loop off the compiler's loop stack.
void pop_loop(Compiler *compiler) {
	compiler->loop_count--;
}


// Patch all break statements for a loop.
void patch_break_statements(Bytecode *bytecode, Loop *loop) {
	for (int i = 0; i < loop->break_statement_count; i++) {
		patch_forward_jump(bytecode, loop->break_statements[i]);
	}
}


// Compiles a while loop.
//
// Consist of a conditional expression evaluation, followed by
// a conditional jump to after the loop, followed by a block,
// followed by a jump back to the conditional expression.
void while_loop(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	if (compiler->loop_count >= MAX_LOOP_DEPTH) {
		// Too many nested loops
		error(compiler->vm->lexer.line,
			"Reached maximum nested loop limit (%d)", MAX_LOOP_DEPTH);
	}

	// Consume the while keyword
	lexer_consume(lexer);

	// Compile the expression
	int start_of_expression = bytecode->count;
	Expression expression = expression_new(compiler,
		&should_terminate_at_open_brace);
	expression_compile(&expression);
	lexer_disable_newlines(lexer);

	// Jump conditionally
	int condition_jump = emit_jump(bytecode, CODE_JUMP_IF_NOT);

	// Append a loop to the compiler.
	Loop loop;
	push_loop(compiler, &loop);

	// Compile the block.
	expect(lexer, TOKEN_OPEN_BRACE,
		"Expected `{` after expression in while loop");
	lexer_enable_newlines(lexer);
	block(compiler, TOKEN_CLOSE_BRACE);
	expect(lexer, TOKEN_CLOSE_BRACE,
		"Expected `}` to close while loop block");

	// Insert a jump statement to re-evaluate the condition
	emit_backward_jump(bytecode, start_of_expression);

	// Patch the conditional jump to point here (after the
	// block)
	patch_forward_jump(bytecode, condition_jump);

	// Patch break statements and pop the loop from the
	// compiler's loop stack
	patch_break_statements(bytecode, &loop);
	pop_loop(compiler);
}


// Compiles a break statement.
void break_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	if (compiler->loop_count == 0) {
		// Not inside a loop
		error(lexer->line, "Break statement not within loop");
	}

	Loop *loop = compiler->loops[compiler->loop_count - 1];
	Bytecode *bytecode = &compiler->fn->bytecode;

	if (loop->break_statement_count >= MAX_BREAK_STATEMENTS) {
		// Too many break statements in a loop
		error(lexer->line,
			"Reached maximum break statement limit in loop (%d)",
			MAX_BREAK_STATEMENTS);
	}

	// Consume the break keyword
	lexer_consume(lexer);

	// Emit pop statements for each variable up to the scope of
	// the loop.
	for (int i = compiler->local_count - 1; i >= 0; i--) {
		Local *local = &compiler->locals[i];

		if (local->scope_depth > loop->scope_depth) {
			emit(bytecode, CODE_POP);
		} else {
			// Since the locals are kept in order of scope
			// depth, once we reach a point where the local's
			// scope is less than the scope we're looking for,
			// it's safe stop looking.
			break;
		}
	}

	// Emit a jump instruction and add it to the loop's break
	// list for patching later
	int jump = emit_jump(bytecode, CODE_JUMP_FORWARD);
	loop->break_statements[loop->break_statement_count++] = jump;
}



//
//  Infinite Loops
//

// Compile an infinite loop.
void infinite_loop(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the loop token
	lexer_consume(lexer);

	// Append a loop to the compiler
	Loop loop;
	push_loop(compiler, &loop);

	// Save the starting location to jump back to
	int start = bytecode->count;

	// Compile the block
	expect(lexer, TOKEN_OPEN_BRACE, "Expected `{` after `loop` keyword");
	block(compiler, TOKEN_CLOSE_BRACE);
	expect(lexer, TOKEN_CLOSE_BRACE, "Expected `}` to close `{` for loop");

	// Insert a jump statement back to the start of the loop
	emit_backward_jump(bytecode, start);

	// Patch break statements and pop the loop
	patch_break_statements(bytecode, &loop);
	pop_loop(compiler);
}



//
//  Function Definitions
//

// Parses the arguments list for `fn`. Expects the lexer's
// cursor to be on the opening parenthesis of the arguments
// list.
//
// Consumes the final closing parenthesis of the arguments.
void function_definition_arguments(Compiler *compiler, Function *fn) {
	Lexer *lexer = &compiler->vm->lexer;
	lexer_disable_newlines(lexer);

	// Reset the argument count
	fn->arity = 0;

	// Expect the opening token to the arguments list
	expect(lexer, TOKEN_OPEN_PARENTHESIS,
		"Expected `(` after name in function definition");

	while (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS) &&
			!lexer_match(lexer, TOKEN_END_OF_FILE)) {
		// Expect the name of the function's argument
		Token name = expect(lexer, TOKEN_IDENTIFIER,
			"Expected argument name in function arguments list");

		SourceString *argument = &fn->arguments[fn->arity++];
		argument->location = name.location;
		argument->length = name.length;

		// Expect a comma or closing parenthesis
		if (lexer_match(lexer, TOKEN_COMMA)) {
			lexer_consume(lexer);
		} else if (lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
			// Don't trigger an error
		} else {
			error(lexer->line,
				"Unexpected token `%.*s` in function definition",
				name.length, name.location);
		}
	}

	// Expect the closing parenthesis
	expect(lexer, TOKEN_CLOSE_PARENTHESIS,
		"Expected `)` to finish function arguments list");

	lexer_enable_newlines(lexer);
}


// Compiles a function definition.
void function_definition(Compiler *compiler) {
	VirtualMachine *vm = compiler->vm;
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function keyword
	lexer_consume(lexer);

	// Expect the function name identifier
	lexer_disable_newlines(lexer);
	Token name = expect(lexer, TOKEN_IDENTIFIER,
		"Expected identifier after `fn` keyword");

	// Define the function on the virtual machine
	Function *fn;
	int fn_index = vm_new_function(compiler->vm, &fn);

	// Check the function isn't already defined
	if (variable_exists(compiler, name.location, name.length)) {
		error(lexer->line, "Function name `%.*s` is already in use",
			name.length, name.location);
	}

	// Compile the function's arguments list
	lexer_enable_newlines(lexer);
	function_definition_arguments(compiler, fn);
	lexer_disable_newlines(lexer);

	// Check library functions
	int native = vm_find_native(vm, name.location, name.length);
	if (native != -1) {
		error(lexer->line, "Function `%.*s` is already defined in a library",
			name.length, name.location);
	}

	// Add the function as a local
	int local_index = define_local(compiler, name.location, name.length);

	// Save the function onto the stack since it's now a local
	// variable
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit_store_function(bytecode, fn_index, local_index);

	// Expect an opening brace to open the function's block
	expect(lexer, TOKEN_OPEN_BRACE, "Expected `{` to begin function block");

	// Compile the function
	fn->bytecode = bytecode_new(DEFAULT_INSTRUCTIONS_CAPACITY);
	lexer_enable_newlines(lexer);
	compile(compiler->vm, compiler, fn, TOKEN_CLOSE_BRACE);

	// Expect a closing brace to close the function's block
	expect(lexer, TOKEN_CLOSE_BRACE, "Expected `}` to close function block");
}



//
//  Expression Statements
//

// Compile an expression that exists as a statement.
void expression_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Start an expression here
	Expression expression = expression_new(compiler, NULL);
	expression_compile(&expression);

	if (!expression.is_only_function_call) {
		// We have something other than a single function call
		// on this line, so trigger an error
		Token token = lexer_current(lexer);
		error(lexer->line, "Unexpected expression at `%.*s`",
			token.length, token.location);
	}

	// Pop the result of the expression
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_POP);

	// Check we have a newline after the expression
	if (!lexer_match(lexer, TOKEN_LINE) &&
			!lexer_match(lexer, TOKEN_END_OF_FILE)) {
		error(lexer->line, "Expected newline after function call");
	}
}



//
//  Return Statements
//

// Emits bytecode to close an upvalue.
void emit_close_upvalue(Compiler *compiler, int index) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	emit(bytecode, CODE_CLOSE_UPVALUE);
	emit_arg_2(bytecode, index);

	Upvalue *upvalue = &compiler->vm->upvalues[index];
	upvalue->name = NULL;
	upvalue->length = 0;
}


// Iterate over the compiler's locals and close any upvalues.
void close_captured_locals(Compiler *compiler) {
	// Iterate over locals and close any upvalues
	for (int i = 0; i < compiler->local_count; i++) {
		Local *local = &compiler->locals[i];

		if (local->upvalue_index != -1) {
			emit_close_upvalue(compiler, local->upvalue_index);
		}
	}
}


// Compile a return statement.
//
// Functions return by pushing the return value onto the top of
// the stack (or nil if the function doesn't return a value),
// and emitting a return statement.
void return_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the return keyword
	lexer_consume(lexer);

	// Check for an expression to return
	if (lexer_match(lexer, TOKEN_LINE)) {
		// Implicitly returning nil
		emit(bytecode, CODE_PUSH_NIL);
	} else {
		// Return an expression terminated by a newline
		Expression expression = expression_new(compiler, NULL);
		expression_compile(&expression);
	}

	// Close any upvalues
	close_captured_locals(compiler);

	emit(bytecode, CODE_RETURN);
	compiler->explicit_return_statement = true;
}



//
//  Class Definitions
//

// Compile a class' fields list.
void class_field_list(Compiler *compiler, ClassDefinition *definition) {
	Lexer *lexer = &compiler->vm->lexer;

	lexer_disable_newlines(lexer);

	// Consume the opening brace
	lexer_consume(lexer);

	// Expect a list of comma separated identifiers, acting as
	// field names
	while (!lexer_match(lexer, TOKEN_CLOSE_BRACE) &&
			!lexer_match(lexer, TOKEN_END_OF_FILE)) {
		// Expect an identifier (the name of the field)
		Token name = expect(lexer, TOKEN_IDENTIFIER,
			"Expected identifier in class field list");

		// Add the field to the class definition
		SourceString *field = &definition->fields[definition->field_count++];
		field->location = name.location;
		field->length = name.length;

		// Expect a comma or a closing brace
		if (lexer_match(lexer, TOKEN_COMMA)) {
			// Consume the comma separating the field names
			lexer_consume(lexer);
		} else if (lexer_match(lexer, TOKEN_CLOSE_BRACE)) {
			// Don't trigger an error
		} else {
			// Unexpected token, so trigger an error
			Token after = lexer_current(lexer);
			error(lexer->line,  "Expected `,` after field name in class "
				"definition, found `%.*s`", after.length, after.location);
		}
	}

	// Expect the closing brace
	expect(lexer, TOKEN_CLOSE_BRACE,
		"Expected `}` to finish class definition fields list");

	lexer_enable_newlines(lexer);
}


// Compile a class definition.
void class_definition(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the class keyword
	lexer_consume(lexer);

	// Expect an identifier (the class' name)
	lexer_disable_newlines(lexer);
	Token name = expect(lexer, TOKEN_IDENTIFIER,
		"Expected identifier (a class name) after `class` keyword");

	// Create the class definition
	ClassDefinition *definition;
	vm_new_class_definition(compiler->vm, &definition);
	definition->name = name.location;
	definition->length = name.length;

	// Check for the optional opening brace after the class
	// name.
	if (lexer_match(lexer, TOKEN_OPEN_BRACE)) {
		lexer_enable_newlines(lexer);
		class_field_list(compiler, definition);
	}
}



//
//  Scoping
//

// Increment the compiler's scope depth.
void push_scope(Compiler *compiler) {
	compiler->scope_depth++;
}


// Decrement the compiler's scope depth, and pop off any local
// variables from the stack that are no longer in scope.
void pop_scope(Compiler *compiler) {
	compiler->scope_depth--;

	int i = compiler->local_count - 1;
	Local *local = &compiler->locals[i];
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Loop over all the variables in the compiler's locals list.
	//
	// The locals are sorted from lowest to highest scope depth,
	// so just loop over from the end of the list until we find
	// a variable that's in scope.
	while (i >= 0 && local->scope_depth > compiler->scope_depth) {
		if (local->upvalue_index != -1) {
			emit_close_upvalue(compiler, local->upvalue_index);
		}

		// Emit a pop instruction
		emit(bytecode, CODE_POP);

		// Decrement counters
		compiler->local_count--;
		i--;

		// Prevent invalid memory access before accessing the
		// local
		if (i >= 0) {
			local = &compiler->locals[i];
		}
	}
}



//
//  Locals
//

// Searches the compiler's locals list for a local with the name
// `name`, returning true and setting `result` if it does, else
// returns false.
bool find_local(Compiler *compiler, Variable *result, char *name, int length) {
	for (int i = 0; i < compiler->local_count; i++) {
		Local *local = &compiler->locals[i];

		if (local->length == length &&
				strncmp(name, local->name, length) == 0) {
			if (result != NULL) {
				result->type = VARIABLE_LOCAL;
				result->index = i;
				result->local = local;
			}
			return true;
		}
	}

	return false;
}


// Searches the virtual machine's upvalue list for an upvalue
// with the name `name`, returning true and setting `result` if
// one is found, else returns false.
bool find_upvalue(Compiler *compiler, Variable *result, char *name,
		int length) {
	VirtualMachine *vm = compiler->vm;

	for (int i = 0; i < vm->upvalue_count; i++) {
		Upvalue *upvalue = &vm->upvalues[i];

		if (upvalue->name != NULL && upvalue->length == length &&
				strncmp(name, upvalue->name, length) == 0) {
			if (result != NULL) {
				result->type = VARIABLE_UPVALUE;
				result->index = i;
				result->upvalue = upvalue;
			}
			return true;
		}
	}

	return false;
}


// Performs a recursive search through parent compilers to find
// a pointer to a local and stack position relative to its
// enclosing function.
//
// Sets `fn` to the function that defines the local.
int find_local_in_all_scopes(Compiler *compiler, Local **local, Function **fn,
		char *name, int length) {
	if (compiler == NULL) {
		// Scanned through all compilers, and couldn't find the
		// local, hence it's undefined.
		return -1;
	}

	Variable result;
	if (find_local(compiler, &result, name, length)) {
		// Found the local
		if (local != NULL) {
			*local = result.local;
		}

		if (fn != NULL) {
			*fn = compiler->fn;
		}

		return result.index;
	} else {
		// Search the parent compiler
		return find_local_in_all_scopes(compiler->parent, local, fn, name,
			length);
	}
}


// Adds `upvalue` to the list of all upvalues closed over by the
// compiler if it doesn't yet exist in the upvalue's list.
void add_upvalue(Compiler *compiler, Upvalue *upvalue) {
	Function *fn = compiler->fn;

	for (int i = 0; i < fn->captured_upvalue_count; i++) {
		if (fn->captured_upvalues[i] == upvalue) {
			// The upvalue already exists in the list of all
			// upvalues closed over by the function, so don't
			// bother adding it again
			return;
		}
	}

	// We haven't seen this upvalue before, so add it to the
	// upvalues list
	fn->captured_upvalues[fn->captured_upvalue_count++] = upvalue;

	// We need to add the upvalue to the list of upvalues the
	// upvalue's definition function closes over here, because
	// at the time we create the local, we don't know if it'll
	// be used later as an upvalue.
	fn = upvalue->defining_function;
	fn->defined_upvalues[fn->defined_upvalue_count++] = upvalue;
}


// Searches for a variable with the name `name`.
//
// Search order:
// * Compiler locals
// * Existing upvalues in the virtual machine
// * Parent compilers' locals
Variable capture_variable(Compiler *compiler, char *name, int length) {
	Variable result;

	// Search the compiler's locals list
	if (find_local(compiler, &result, name, length)) {
		return result;
	}

	// Search for existing upvalues
	if (find_upvalue(compiler, &result, name, length)) {
		// Add the upvalue to all the upvalues this function
		// closes over
		add_upvalue(compiler, result.upvalue);
		return result;
	}

	// Search parent compiler locals for a new upvalue. Start
	// the search in the parent compiler because we've already
	// searched through this compiler's locals.
	Function *defining_function;
	Local *local;
	int local_index = find_local_in_all_scopes(compiler->parent, &local,
		&defining_function, name, length);
	if (local_index == -1) {
		// Undefined variable
		result.type = VARIABLE_UNDEFINED;
		return result;
	}

	// Create the upvalue
	Upvalue *upvalue;
	int upvalue_index = vm_new_upvalue(compiler->vm, &upvalue);
	upvalue->name = local->name;
	upvalue->length = local->length;
	upvalue->local_index = local_index;
	upvalue->defining_function = defining_function;

	result.type = VARIABLE_UPVALUE;
	result.index = upvalue_index;
	result.upvalue = upvalue;

	add_upvalue(compiler, upvalue);

	// Mark the local as an upvalue
	local->upvalue_index = upvalue_index;

	return result;
}


// Returns true if a variable with the name `name` exists.
bool variable_exists(Compiler *compiler, char *name, int length) {
	if (find_local(compiler, NULL, name, length)) {
		return true;
	}

	if (find_upvalue(compiler, NULL, name, length)) {
		return true;
	}

	if (find_local_in_all_scopes(compiler->parent, NULL, NULL, name,
			length) != -1) {
		return true;
	}

	return false;
}


// Creates a new local on the compiler. Returns the index of the
// new local in the compiler's index list.
int define_local(Compiler *compiler, char *name, int length) {
	// Check we haven't exceeded the maximum number of local
	// variables we're allowed to define.
	if (compiler->local_count + 1 > MAX_LOCALS) {
		error(compiler->vm->lexer.line,
			"Cannot have more than %d locals in scope", MAX_LOCALS);
	}

	// Create the local
	int index = compiler->local_count;
	Local *local = &compiler->locals[index];
	compiler->local_count++;
	local->name = name;
	local->length = length;
	local->scope_depth = compiler->scope_depth;
	local->upvalue_index = -1;
	return index;
}


// Emits bytecode to push a variable onto the stack, handling
// possible cases when the variable could be a local or upvalue.
void emit_push_variable(Bytecode *bytecode, Variable *variable) {
	if (variable->type == VARIABLE_LOCAL) {
		emit(bytecode, CODE_PUSH_LOCAL);
	} else if (variable->type == VARIABLE_UPVALUE) {
		emit(bytecode, CODE_PUSH_UPVALUE);
	}

	emit_arg_2(bytecode, variable->index);
}


// Returns the appropriate instruction for storing a variable
// of type `type`.
void emit_store_variable(Bytecode *bytecode, Variable *variable) {
	if (variable->type == VARIABLE_UPVALUE) {
		emit(bytecode, CODE_STORE_UPVALUE);
	} else if (variable->type == VARIABLE_LOCAL) {
		emit(bytecode, CODE_STORE_LOCAL);
	}

	emit_arg_2(bytecode, variable->index);
}


// Stores a function onto the stack.
void emit_store_function(Bytecode *bytecode, int fn_index, int local_index) {
	emit_push_function(bytecode, fn_index);
	emit(bytecode, CODE_STORE_LOCAL);
	emit_arg_2(bytecode, local_index);
}
