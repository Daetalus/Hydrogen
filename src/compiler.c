
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


// The maximum number of else if statements that are allowed to
// follow an if statement.
#define MAX_ELSE_IF_STATEMENTS 256


// Compile a block. Assumes the opening token for the block has
// been consumed. Stops when the terminating token is found.
//
// Does not consume the terminating token.
void block(Compiler *compiler, TokenType terminator);

// Compile a single statement. A statement is one construct in
// the language, like an if statement or variable assignment.
void statement(Compiler *compiler);

// Match statements against the lexer.
bool match_variable_assignment(Lexer *lexer);

// Compile statements.
void variable_assignment(Compiler *compiler);
void if_statement(Compiler *compiler);
void while_loop(Compiler *compiler);
void infinite_loop(Compiler *compiler);
void break_statement(Compiler *compiler);
void function_call_statement(Compiler *compiler);
void function_definition(Compiler *compiler);
void return_statement(Compiler *compiler);

// Iterate over the compiler's locals and close any upvalues.
void close_upvalue_locals(Compiler *compiler);

// Increment the compiler's scope depth.
void push_scope(Compiler *compiler);

// Decrement the compiler's scope depth, and pop off any local
// variables from the stack that are no longer in scope.
void pop_scope(Compiler *compiler);

// Creates a new local on the compiler. Returns the index of the
// new local in the compiler's index list.
int define_local(Compiler *compiler, char *name, int length);



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
	while (!lexer_match(lexer, terminator)) {
		statement(&compiler);
	}

	// Insert a final return instruction, pushing nil as the
	// return value.
	if (!compiler.explicit_return_statement) {
		close_upvalue_locals(&compiler);
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

	while (!lexer_match(lexer, terminator)) {
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
	} else if (match_function_call(lexer)) {
		function_call_statement(compiler);
	} else if (lexer_match(lexer, TOKEN_FUNCTION)) {
		function_definition(compiler);
	} else if (lexer_match(lexer, TOKEN_RETURN)) {
		return_statement(compiler);
	} else {
		Token token = lexer_current(lexer);
		error(lexer->line, "Unrecognized statement beginning with `%.*s`",
			token.length, token.location);
	}
}



//
//  Variable Assignment
//

// Returns true if the sequence of tokens at the lexer's current
// cursor position represent a variable assignment.
bool match_variable_assignment(Lexer *lexer) {
	// Recognise either a let token (for new variables) or
	// an identifier followed by an assignment token.
	lexer_disable_newlines(lexer);
	bool result = lexer_match(lexer, TOKEN_LET) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_ADDITION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_SUBTRACTION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_DIVISION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_MODULO_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER,
			TOKEN_MULTIPLICATION_ASSIGNMENT);
	lexer_enable_newlines(lexer);

	return result;
}


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
		// No modification needed, but we don't want it to
		// trigger an expected `=` sign error.
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
	expression(compiler, NULL);

	if (modifier_fn != NULL) {
		// Push a call to the modifier function.
		emit_native_call(bytecode, modifier_fn);
	}

	// Emit the bytecode to store the expression result.
	emit(bytecode, storage_instruction(variable.type));

	int index = variable.index;
	if (is_new_var) {
		// We're assigning to a new variable, so we need a new
		// local index for it.
		index = define_local(compiler, name.location, name.length);
	}

	emit_arg_2(bytecode, index);
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
	expression(compiler, &should_terminate_at_open_brace);

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

		// Now that we've added the very last thing to the if
		// statement's block, we can patch it's conditional to
		// point here.
		patch_forward_jump(bytecode, previous_jump);

		// Consume the else if token.
		lexer_consume(lexer);

		// Compile the conditional expression and block.
		lexer_enable_newlines(lexer);
		previous_jump = if_condition_and_block(compiler);
		lexer_disable_newlines(lexer);
	}

	// If there's an else block to follow.
	if (lexer_match(lexer, TOKEN_ELSE)) {
		had_else = true;

		// Consume the else token.
		lexer_consume(lexer);

		// Emit an unpatched jump instruction for the if/elseif
		// statement that preceded this else statement.
		unpatched_jumps[jump_count] = emit_jump(bytecode, CODE_JUMP_FORWARD);
		jump_count++;

		// Patch the previous jump statement
		patch_forward_jump(bytecode, previous_jump);

		// Compile the else statement's block.
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

// Create a new loop and push it onto the compiler's loop stack.
void push_new_loop(Compiler *compiler, Loop *loop) {
	loop->break_statement_count = 0;
	loop->scope_depth = compiler->scope_depth;
	compiler->loops[compiler->loop_count++] = loop;
}


// Patch all break statements for a loop.
void patch_break_statements(Bytecode *bytecode, Loop *loop) {
	for (int i = 0; i < loop->break_statement_count; i++) {
		patch_forward_jump(bytecode, loop->break_statements[i]);
	}
}


// Pop the top most loop off the compiler's loop stack.
void pop_loop(Compiler *compiler) {
	// Pop the loop from the loop stack.
	compiler->loop_count--;
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
	expression(compiler, &should_terminate_at_open_brace);
	lexer_disable_newlines(lexer);

	// Jump conditionally
	int condition_jump = emit_jump(bytecode, CODE_JUMP_IF_NOT);

	// Append a loop to the compiler.
	Loop loop;
	push_new_loop(compiler, &loop);

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
	if (compiler->loop_count == 0) {
		// Not inside a loop
		error(compiler->vm->lexer.line,
			"Break statement not within loop");
	}

	Lexer *lexer = &compiler->vm->lexer;
	Loop *loop = compiler->loops[compiler->loop_count - 1];
	Bytecode *bytecode = &compiler->fn->bytecode;

	if (loop->break_statement_count >= MAX_BREAK_STATEMENTS) {
		// Too many break statements in a loop
		error(compiler->vm->lexer.line,
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
	push_new_loop(compiler, &loop);

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
//  Function Calls
//

// Returns true if the lexer matches a function call.
bool match_function_call(Lexer *lexer) {
	// Allow newlines between the identifier and open
	// parenthesis, so disable newlines when checking for a
	// match.
	lexer_disable_newlines(lexer);
	bool result = lexer_match_two(lexer, TOKEN_IDENTIFIER,
		TOKEN_OPEN_PARENTHESIS);
	lexer_enable_newlines(lexer);

	return result;
}


// Returns true if the token should terminate a function call
// argument.
bool should_terminate_function_call(Token token) {
	return token.type == TOKEN_COMMA ||
		token.type == TOKEN_CLOSE_PARENTHESIS;
}


// Compiles the arguments to a function call.
int function_call_arguments(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the opening parenthesis
	lexer_disable_newlines(lexer);
	expect(lexer, TOKEN_OPEN_PARENTHESIS,
		"Expected `(` to begin function call arguments");

	// Consume expressions separated by commas
	int count = 0;
	if (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
		while (1) {
			// Compile an expression
			lexer_enable_newlines(lexer);
			expression(compiler, &should_terminate_function_call);
			lexer_disable_newlines(lexer);
			count++;

			// Consume either a comma, indicating another
			// expression, or a terminating closing parenthesis
			if (lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
				// Finish expression
				lexer_consume(lexer);
				break;
			} else if (lexer_match(lexer, TOKEN_COMMA)) {
				// Another argument
				lexer_consume(lexer);
			} else {
				// Unrecognised operator
				Token token = lexer_current(lexer);
				error(lexer->line,
					"Unexpected `%.*s` in arguments to function call",
					token.length, token.location);
			}
		}
	} else {
		// No arguments, consume the closing parenthesis
		lexer_consume(lexer);
	}

	lexer_enable_newlines(lexer);
	return count;
}


// Compiles a function call, leaving the return value of the
// function on the top of the stack.
void function_call(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function's name
	Token name = lexer_consume(lexer);

	// Compile the function's arguments
	int arity = function_call_arguments(compiler);

	// Check the function exists as a user defined function
	int fn_index = vm_find_function(compiler->vm, name.location, name.length,
		arity);
	if (fn_index != -1) {
		emit_bytecode_call(bytecode, fn_index);
		return;
	}

	// Not a user defined function, so check the standard
	// library
	NativeFunction native_fn = vm_find_native_function(compiler->vm,
		name.location, name.length, arity);
	if (native_fn != NULL) {
		emit_native_call(bytecode, native_fn);
		return;
	}

	// Finally, push a local variable with the same name in hope
	// that the local is a closure.
	Variable variable = capture_variable(compiler, name.location, name.length);
	if (variable.type != VARIABLE_UNDEFINED) {
		emit_push_variable(bytecode, &variable);
		emit(bytecode, CODE_CALL_STACK);
		return;
	}

	// Undefined function
	error(lexer->line, "Undefined function `%.*s`",
		name.length, name.location);
}


// Compiles a function call, but popping the function's return
// value off the stack, as we assume it's of no use.
void function_call_statement(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	function_call(compiler);
	emit(bytecode, CODE_POP);
}



//
//  Function Definitions
//

// Parses the arguments list for `fn`. Expects the lexer's
// cursor  to be on the opening parenthesis of the arguments
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

	if (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
		while (1) {
			// Expect an identifier, the function argument
			Token argument = expect(lexer, TOKEN_IDENTIFIER,
				"Expected argument name in function argument list");

			int index = fn->arity;
			fn->arity++;
			fn->arguments[index].location = argument.location;
			fn->arguments[index].length = argument.length;

			// Expect a comma or closing parenthesis
			if (lexer_match(lexer, TOKEN_COMMA)) {
				lexer_consume(lexer);
			} else if (lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
				// End of the arguments list
				lexer_consume(lexer);
				break;
			} else {
				Token token = lexer_peek(lexer, 1);
				error(lexer->line,
					"Unexpected `%.*s` in arguments to function definition",
					token.length, token.location);
			}
		}
	} else {
		// Consume the closing parenthesis
		lexer_consume(lexer);
	}

	lexer_enable_newlines(lexer);
}


// Compiles a function definition.
void function_definition(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function keyword
	lexer_consume(lexer);

	// Expect the function name identifier
	lexer_disable_newlines(lexer);
	Token name = expect(lexer, TOKEN_IDENTIFIER,
		"Expected identifier after `fn` keyword");

	// Compile the arguments list.
	//
	// Set the name and length after we've checked that the
	// function isn't already defined, because if we set them
	// here, then we've defined the function already and we'll
	// trigger a compilation error.
	Function *fn;
	vm_new_function(compiler->vm, &fn);
	lexer_enable_newlines(lexer);
	function_definition_arguments(compiler, fn);
	lexer_disable_newlines(lexer);

	// Check the function isn't already defined
	int index = vm_find_function(compiler->vm, name.location, name.length,
		fn->arity);
	if (index != -1) {
		error(lexer->line, "Function `%.*s` is already defined",
			name.length, name.location);
	}

	NativeFunction ptr = vm_find_native_function(compiler->vm, name.location,
		name.length, fn->arity);
	if (ptr != NULL) {
		error(lexer->line, "Function `%.*s` is already defined in a library",
			name.length, name.location);
	}

	fn->name = name.location;
	fn->length = name.length;

	// Expect an opening brace, to open the function's block
	expect(lexer, TOKEN_OPEN_BRACE, "Expected `{` to begin function block");

	// Compile the function
	fn->bytecode = bytecode_new(DEFAULT_INSTRUCTIONS_CAPACITY);
	lexer_enable_newlines(lexer);
	compile(compiler->vm, compiler, fn, TOKEN_CLOSE_BRACE);

	// Expect a closing brace to close the function's block
	expect(lexer, TOKEN_CLOSE_BRACE, "Expected `}` to close function block");
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
void close_upvalue_locals(Compiler *compiler) {
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
		if (compiler->fn->is_main) {
			// We're trying to return an expression from within
			// the "main" function. This isn't allowed, so
			// trigger an error.
			error(lexer->line,
				"Attempting to return value from outside function definition");
		}

		// Return an expression terminated by a newline
		expression(compiler, NULL);
	}

	// Close any upvalues
	close_upvalue_locals(compiler);

	emit(bytecode, CODE_RETURN);
	compiler->explicit_return_statement = true;
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
			result->type = VARIABLE_LOCAL;
			result->index = i;
			result->local = local;
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
	for (int i = 0; i < compiler->vm->upvalue_count; i++) {
		Upvalue *upvalue = &compiler->vm->upvalues[i];

		if (upvalue->name != NULL && upvalue->length == length &&
				strncmp(name, upvalue->name, length) == 0) {
			result->type = VARIABLE_UPVALUE;
			result->index = i;
			result->upvalue = upvalue;
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
		*local = result.local;
		*fn = compiler->fn;
		return result.index;
	} else {
		// Search the parent compiler
		return find_local_in_all_scopes(compiler->parent, local, fn,
			name, length);
	}
}


// Adds `upvalue` to the list of all upvalues closed over by the
// compiler if it doesn't yet exist in the upvalue's list.
void add_upvalue(Compiler *compiler, Upvalue *upvalue) {
	for (int i = 0; i < compiler->fn->captured_upvalue_count; i++) {
		if (compiler->fn->captured_upvalues[i] == upvalue) {
			// The upvalue already exists in the list of all
			// upvalues closed over by the function, so don't
			// bother adding it again
			return;
		}
	}

	// We haven't seen this upvalue before, so add it to the
	// upvalues list
	Function *fn = compiler->fn;
	fn->captured_upvalues[fn->captured_upvalue_count++] = upvalue;

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


// Creates a new local on the compiler. Returns the index of the
// new local in the compiler's index list.
int define_local(Compiler *compiler, char *name, int length) {
	// Check for overflow
	if (compiler->local_count + 1 > MAX_LOCALS) {
		Lexer *lexer = &compiler->vm->lexer;

		// We've used up as many locals as we're allowed, so
		// trigger an error.
		error(lexer->line, "Cannot have more than %d locals in scope",
			MAX_LOCALS);
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


// Pushes a string onto the stack.
// Returns a pointer to an unallocated string,
// so the string that will be pushed can be modified.
String ** push_string(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	int index = compiler->vm->literal_count;
	compiler->vm->literal_count++;

	emit(bytecode, CODE_PUSH_STRING);
	emit_arg_2(bytecode, index);
	return &compiler->vm->literals[index];
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
Instruction storage_instruction(VariableType type) {
	if (type == VARIABLE_UPVALUE) {
		return CODE_STORE_UPVALUE;
	}
	return CODE_STORE_LOCAL;
}
