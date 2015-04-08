
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


// The maximum number of else if statements that are allowed to
// follow an if statement.
#define MAX_ELSE_IF_STATEMENTS 256


// Forward definitions.
void block(Compiler *compiler, TokenType terminator);
void statement(Compiler *compiler);

bool match_variable_assignment(Compiler *compiler);
void variable_assignment(Compiler *compiler);

bool match_if_statement(Compiler *compiler);
void if_statement(Compiler *compiler);

bool match_while_loop(Compiler *compiler);
void while_loop(Compiler *compiler);

bool match_function_call(Compiler *compiler);
void emit_bytecode_call(Compiler *compiler, int index);

bool match_function_definition(Compiler *compiler);
void function_definition(Compiler *compiler);

void push_scope(Compiler *compiler);
void pop_scope(Compiler *compiler);

void push_local_at_index(Compiler *compiler, int index);
int find_local(Compiler *compiler, char *name, int length);
uint16_t define_local(Compiler *compiler, char *name, int length);


// Compile source code into bytecode.
//
// The compiler uses the lexer in the given virtual machine as
// its input.
//
// The compiler generates the bytecode output directly into the
// given function's bytecode array.
//
// It stops compiling when the given terminator token is found,
// or end of file is reached.
//
// Constants are added to the constants list.
void compile(VirtualMachine *vm, Function *fn, TokenType terminator) {
	// Create a compiler for this function.
	Compiler compiler;
	compiler.vm = vm;
	compiler.fn = fn;
	compiler.local_count = 0;
	compiler.scope_depth = -1;

	// Push the function's arguments as locals
	for (int i = 0; i < fn->argument_count; i++) {
		Local *local = &compiler.locals[compiler.local_count];
		compiler.local_count++;

		local->name = fn->arguments[i].location;
		local->length = fn->arguments[i].length;
		local->scope_depth = 0;
	}

	// Treat the source code as a top level block, stopping when
	// we reach the terminator character.
	block(&compiler, terminator);

	// Insert an ending instruction
	emit(&compiler.fn->bytecode, CODE_RETURN);
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

	// Blocks consist of a sequence of statements, so continually
	// compile statements.
	while (!lexer_match(lexer, terminator)) {
		statement(compiler);
	}

	pop_scope(compiler);
}


// Compile a single statement. A statement is effectively one
// line of code.
//
// Possible statements include:
// * Variable assignment
// * If statements
void statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	if (lexer_match(lexer, TOKEN_LINE)) {
		// Ignore empty lines
		lexer_consume(lexer);
	} else if (match_variable_assignment(compiler)) {
		variable_assignment(compiler);
	} else if (match_if_statement(compiler)) {
		if_statement(compiler);
	} else if (match_while_loop(compiler)) {
		while_loop(compiler);
	} else if (match_function_call(compiler)) {
		function_call(compiler);
	} else if (match_function_definition(compiler)) {
		function_definition(compiler);
	} else {
		Token current = lexer_current(lexer);
		error(compiler, "Unrecognized statement beginning with `%.*s`",
			current.length, current.location);
	}
}



//
//  Variable Assignment
//

// Returns true if the current sequence of tokens represent a
// variable assignment.
bool match_variable_assignment(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Recognise either a let token (for new variables) or
	// an identifier followed by an assignment token.
	return lexer_match(lexer, TOKEN_LET) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_ADDITION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_SUBTRACTION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_DIVISION_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_MODULO_ASSIGNMENT) ||
		lexer_match_two(lexer, TOKEN_IDENTIFIER, TOKEN_MULTIPLICATION_ASSIGNMENT);
}


// Compile a variable assignment.
void variable_assignment(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Indicates whether the variable we're assigning to has been
	// defined before, or whether we're defining it for the first
	// time here.
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
	Token name = expect(compiler, TOKEN_IDENTIFIER,
		"Expected variable name");
	if (name.type == TOKEN_NONE) {
		return;
	}

	// Check to see if the variable already exists.
	int index = find_local(compiler, name.location, name.length);
	if (is_new_var && index != -1) {
		// We're trying to create a new variable using a variable
		// name that's already taken.
		error(compiler, "Variable name `%.*s` already taken in assignment",
			name.length, name.location);
	} else if (!is_new_var && index == -1) {
		// We're trying to assign a new value to a variable that
		// doesn't exist.
		error(compiler, "Variable `%.*s` doesn't exist in assignment",
			name.length, name.location);
	}

	// Expect an equals sign.
	NativeFunction fn = NULL;
	if (lexer_match(lexer, TOKEN_ADDITION_ASSIGNMENT)) {
		fn = &operator_addition;
	} else if (lexer_match(lexer, TOKEN_SUBTRACTION_ASSIGNMENT)) {
		fn = &operator_subtraction;
	} else if (lexer_match(lexer, TOKEN_MULTIPLICATION_ASSIGNMENT)) {
		fn = &operator_multiplication;
	} else if (lexer_match(lexer, TOKEN_DIVISION_ASSIGNMENT)) {
		fn = &operator_division;
	} else if (lexer_match(lexer, TOKEN_MODULO_ASSIGNMENT)) {
		fn = &operator_modulo;
	} else if (lexer_match(lexer, TOKEN_ASSIGNMENT)) {
		// No modification needed
	} else {
		error(compiler, "Expected `=` after variable name in assignment");
	}
	lexer_consume(lexer);

	// Disallow modifier operators on new variables
	if (is_new_var && fn != NULL) {
		error(compiler, "Expected `=` after variable name in assignment");
	}

	if (fn != NULL) {
		// Push the variable for the modifier function
		push_local_at_index(compiler, index);
	}

	// Compile the expression after this. This will push bytecode
	// that will leave the resulting expression on top of the
	// stack.
	lexer_enable_newlines(lexer);
	expression(compiler, TOKEN_LINE);

	if (fn != NULL) {
		// Push the modifier function itself.
		emit_native(compiler, fn);
	}

	// Emit the bytecode to store the item that's on the top of
	// the stack into a stack slot.
	//
	// We use the local's index in the compiler's locals list as
	// the stack slot.
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_STORE);

	if (index == -1) {
		// We're assigning to a new variable, so we need a new
		// index for it.
		index = define_local(compiler, name.location, name.length);
	}

	emit_arg_2(bytecode, index);
}



//
//  If Statement
//

// Returns true if the current sequence of tokens represents an
// if statement.
bool match_if_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// An if statement starts with the if token.
	return lexer_match(lexer, TOKEN_IF);
}


// Compile the part of an if or else if statement where we have a
// conditional expression followed by a block. Emits bytecode for
// the expression, a conditional jump and code for the block.
//
// Expects the lexer to start on the first token of the
// expression.
//
// Returns the index of the conditional jump emitted, which can be
// patched after the final jump statement (after an if or else if
// to jump to the end of entire statement).
int if_condition_and_block(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Expect an expression, terminated by the opening brace of
	// the block.
	// Leaves the result of the conditional expression on the top
	// of the stack.
	expression(compiler, TOKEN_OPEN_BRACE);

	// Emit a conditional jump instruction with a default
	// argument.
	// We'll patch the jump instruction once we know how big the
	// if statement's block is.
	int jump = emit_jump(bytecode, CODE_JUMP_IF_NOT);

	// Consume the opening brace of the if statement's block.
	lexer_disable_newlines(&compiler->vm->lexer);
	expect(compiler, TOKEN_OPEN_BRACE,
		"Expected `{` after conditional expression in if statement");

	// Compile the block.
	lexer_enable_newlines(&compiler->vm->lexer);
	block(compiler, TOKEN_CLOSE_BRACE);

	// Consume the closing brace.
	expect(compiler, TOKEN_CLOSE_BRACE,
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
		// Because we need to compile all of the else if and else
		// blocks before we can patch these jump instructions,
		// store them in an array.
		unpatched_jumps[jump_count] = emit_jump(bytecode, CODE_JUMP_FORWARD);
		jump_count++;

		// Now that we've added the very last thing to the if
		// statement's block, we can patch it's conditional to
		// point here.
		patch_jump(bytecode, previous_jump);

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
		patch_jump(bytecode, previous_jump);

		// Compile the else statement's block.
		expect(compiler, TOKEN_OPEN_BRACE,
			"Expected `{` after `else`");
		lexer_enable_newlines(lexer);
		block(compiler, TOKEN_CLOSE_BRACE);
		expect(compiler, TOKEN_CLOSE_BRACE,
			"Expected `}` to close else statement block");
	} else {
		lexer_enable_newlines(lexer);
	}

	if (!had_else && !had_else_if) {
		// There were no else or else if statements, so the
		// original if's conditional still hasn't been patched.
		patch_jump(bytecode, previous_jump);
	}

	// We've compiled the entire statement now, so patch all the
	// unpatched jump statements to point here.
	for (int i = 0; i < jump_count; i++) {
		patch_jump(bytecode, unpatched_jumps[i]);
	}
}



//
//  While Loops
//

// Returns true if the lexer matches a while loop.
bool match_while_loop(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	return lexer_match(lexer, TOKEN_WHILE);
}


// Compiles a while loop.
void while_loop(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the while keyword.
	lexer_consume(lexer);

	// Compile the expression
	int start_of_expression = bytecode->count;
	expression(compiler, TOKEN_OPEN_BRACE);
	lexer_disable_newlines(lexer);

	// Jump conditionally
	int condition_jump = emit_jump(bytecode, CODE_JUMP_IF_NOT);

	// Compile the block.
	expect(compiler, TOKEN_OPEN_BRACE,
		"Expected `{` after expression in while loop");
	lexer_enable_newlines(lexer);
	block(compiler, TOKEN_CLOSE_BRACE);
	expect(compiler, TOKEN_CLOSE_BRACE,
		"Expected `}` to close while loop block");

	// Insert a jump statement to re-evaluate the condition
	emit(bytecode, CODE_JUMP_BACK);
	emit_arg_2(bytecode, bytecode->count - start_of_expression + 2);

	// Patch the conditional jump to point to here (after
	// the block)
	patch_jump(bytecode, condition_jump);
}



//
//  Function Calls
//

// Returns true if the lexer matches a function call.
bool match_function_call(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Allow newlines between the identifier and open parenthesis,
	// so disable newlines when checking.
	lexer_disable_newlines(lexer);
	bool result = lexer_match_two(lexer, TOKEN_IDENTIFIER,
		TOKEN_OPEN_PARENTHESIS);
	lexer_enable_newlines(lexer);

	return result;
}


// Compiles the arguments to a function call.
int function_call_arguments(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the opening parenthesis
	lexer_disable_newlines(lexer);
	expect(compiler, TOKEN_OPEN_PARENTHESIS,
		"Expected `(` to begin function call arguments");

	// Consume expressions separated by commas.
	int count = 0;
	if (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
		while (1) {
			lexer_enable_newlines(lexer);
			expression(compiler, TOKEN_COMMA);
			lexer_disable_newlines(lexer);
			count++;

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
				error(compiler,
					"Unexpected `%.*s` in arguments to function call.",
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


// Compiles a function call.
void function_call(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function's name.
	Token name = lexer_consume(lexer);

	// Compile the function's arguments.
	int argument_count = function_call_arguments(compiler);

	// Check the function exists.
	int index = find_function(compiler->vm, name.location, name.length,
		argument_count);
	if (index != -1) {
		emit_bytecode_call(compiler, index);
		return;
	}

	// Not a user function, so check for native functions.
	NativeFunction fn = find_native_function(compiler->vm,
		name.location, name.length, argument_count);
	if (fn != NULL) {
		emit_native(compiler, fn);
		return;
	}

	// Undefined function if we reach here
	error(compiler, "Undefined function `%.*s`", name.length, name.location);
}



//
//  Function Definitions
//

// Returns true if the lexer matches a function definition.
bool match_function_definition(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	return lexer_match(lexer, TOKEN_FUNCTION);
}


// Compiles a function definition.
void function_definition(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the function keyword.
	lexer_consume(lexer);

	// Expect the function name identifier.
	lexer_disable_newlines(lexer);
	Token name = expect(compiler, TOKEN_IDENTIFIER,
		"Expected identifier after `fn` keyword");

	// Expect the opening token to the arguments list
	expect(compiler, TOKEN_OPEN_PARENTHESIS,
		"Expected `(` after name in function definition.");

	// Compile the arguments list
	Function *fn = define_bytecode_function(compiler->vm);
	fn->argument_count = 0;

	// Set the name and length after we've checked that the
	// function isn't already defined, because if we set them
	// here, then we've defined the function already and we'll
	// trigger a compilation error.
	fn->name = NULL;
	fn->length = 0;

	if (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
		while (1) {
			// Expect an identifier, the function argument
			Token name = expect(compiler, TOKEN_IDENTIFIER,
				"Expected argument name in function argument list");

			int index = fn->argument_count;
			fn->argument_count++;
			fn->arguments[index].location = name.location;
			fn->arguments[index].length = name.length;

			// Expect a comma or closing parenthesis
			if (lexer_match(lexer, TOKEN_COMMA)) {
				lexer_consume(lexer);
			} else if (lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
				lexer_consume(lexer);
				break;
			} else {
				Token token = lexer_peek(lexer, 1);
				error(compiler,
					"Unexpected `%.*s` in arguments to function definition",
					token.length, token.location);
			}
		}
	} else {
		// Consume the closing parenthesis
		lexer_consume(lexer);
	}

	// Expect the opening brace to the function block.
	expect(compiler, TOKEN_OPEN_BRACE, "Expected `{` to begin function block");

	// Check the function isn't already defined.
	int index = find_function(compiler->vm, name.location, name.length,
		fn->argument_count);
	if (index != -1) {
		error(compiler, "Function `%.*s` is already defined",
			name.length, name.location);
	}

	NativeFunction ptr = find_native_function(compiler->vm, name.location,
		name.length, fn->argument_count);
	if (ptr != NULL) {
		error(compiler, "Function `%.*s` is already defined in a library",
			name.length, name.location);
	}

	fn->name = name.location;
	fn->length = name.length;

	// Compile the function
	bytecode_new(&fn->bytecode, DEFAULT_INSTRUCTION_CAPACITY);
	compile(compiler->vm, fn, TOKEN_CLOSE_BRACE);

	// Consume the closing brace.
	expect(compiler, TOKEN_CLOSE_BRACE,
		"Expected `}` to close function block.");
}



//
//  Function Call Emission
//

// Returns the function pointer for an operator.
// Returns NULL if the token is not an operator.
NativeFunction operator_ptr(TokenType operator) {
	#define OPERATOR_CASE(token, function) \
		case token:                        \
			return &function;

	switch(operator) {
		// Mathematical Operators
		OPERATOR_CASE(TOKEN_ADDITION, operator_addition)
		OPERATOR_CASE(TOKEN_SUBTRACTION, operator_subtraction)
		OPERATOR_CASE(TOKEN_MULTIPLICATION, operator_multiplication)
		OPERATOR_CASE(TOKEN_DIVISION, operator_division)
		OPERATOR_CASE(TOKEN_MODULO, operator_modulo)
		OPERATOR_CASE(TOKEN_NEGATION, operator_negation)

		// Boolean Operators
		OPERATOR_CASE(TOKEN_BOOLEAN_AND, operator_boolean_and)
		OPERATOR_CASE(TOKEN_BOOLEAN_OR, operator_boolean_or)
		OPERATOR_CASE(TOKEN_BOOLEAN_NOT, operator_boolean_not)
		OPERATOR_CASE(TOKEN_EQUAL, operator_equal)
		OPERATOR_CASE(TOKEN_NOT_EQUAL, operator_not_equal)

		OPERATOR_CASE(TOKEN_LESS_THAN,
			operator_less_than)
		OPERATOR_CASE(TOKEN_LESS_THAN_EQUAL_TO,
			operator_less_than_equal_to)
		OPERATOR_CASE(TOKEN_GREATER_THAN,
			operator_greater_than)
		OPERATOR_CASE(TOKEN_GREATER_THAN_EQUAL_TO,
			operator_greater_than_equal_to)

		// Bitwise Operators
		OPERATOR_CASE(TOKEN_LEFT_SHIFT, operator_left_shift)
		OPERATOR_CASE(TOKEN_RIGHT_SHIFT, operator_right_shift)
		OPERATOR_CASE(TOKEN_BITWISE_AND, operator_bitwise_and)
		OPERATOR_CASE(TOKEN_BITWISE_OR, operator_bitwise_or)
		OPERATOR_CASE(TOKEN_BITWISE_NOT, operator_bitwise_not)
		OPERATOR_CASE(TOKEN_BITWISE_XOR, operator_bitwise_xor)

		// If we don't recognise the operator, just return.
		default:
			return NULL;
	}
}


// Emits a call to a native function.
void emit_native(Compiler *compiler, NativeFunction fn) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_CALL_NATIVE);
	emit_arg_8(bytecode, ptr_to_value(fn));
}


// Emits a call to a user function.
void emit_bytecode_call(Compiler *compiler, int index) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_CALL);
	emit_arg_2(bytecode, index);
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
	// TODO: Check for underflow
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
		// Emit a pop instruction.
		emit(bytecode, CODE_POP);

		// Decrement counters
		compiler->local_count--;
		i--;

		// Prevent invalid memory access before accessing the local
		if (i > 0) {
			local = &compiler->locals[i];
		}
	}
}



//
//  Locals
//

// Returns the index of a variable in the locals list, or -1
// if the variable doesn't exist.
int find_local(Compiler *compiler, char *name, int length) {
	// Starting from the end of the locals list is going to be
	// (hopefully) faster, as people are more likely to use
	// variables they've defined recently.
	//
	// This isn't based on any scientific fact! It's just
	// speculation.
	for (int i = compiler->local_count - 1; i >= 0; i--) {
		Local *local = &compiler->locals[i];

		if (local->length == length &&
				strncmp(name, local->name, length) == 0) {
			return i;
		}
	}

	return -1;
}


// Creates a new local on the compiler. Returns the index of the
// new local in the compiler's index list.
uint16_t define_local(Compiler *compiler, char *name, int length) {
	// Check for overflow
	if (compiler->local_count + 1 > MAX_LOCALS) {
		// We've used up as many locals as we're allowed, so
		// trigger an error.
		error(compiler, "Cannot have more than %d locals in scope",
			MAX_LOCALS);
	}

	// Create the local
	int index = compiler->local_count;
	Local *local = &compiler->locals[index];
	compiler->local_count++;
	local->name = name;
	local->length = length;
	local->scope_depth = compiler->scope_depth;
	return index;
}


// Emits bytecode to push a local at a given index onto the stack.
void push_local_at_index(Compiler *compiler, int index) {
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_PUSH_VARIABLE);
	emit_arg_2(bytecode, index);
}


// Emits bytecode to push the local with the given name onto the
// stack.
void push_local(Compiler *compiler, char *name, int length) {
	int index = find_local(compiler, name, length);

	// Check for an undefined variable.
	if (index == -1) {
		error(compiler, "Undefined variable `%.*s`", length, name);
	}

	push_local_at_index(compiler, index);
}


// Emits bytecode to push a number onto the stack.
void push_number(Compiler *compiler, double number) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	uint64_t converted = number_to_value(number);
	emit(bytecode, CODE_PUSH_NUMBER);
	emit_arg_8(bytecode, converted);
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



//
//  Error Handling
//

// Triggers the given error on the compiler.
void error(Compiler *compiler, char *fmt, ...) {
	// Print the error
	va_list args;
	va_start(args, fmt);

	int line = compiler->vm->lexer.line;
	fprintf(stderr, RED BOLD "error " WHITE "line %d: ", line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, NORMAL "\n");

	va_end(args);

	// Halt the program
	exit(0);
}


// Consumes the next token, triggering an error with the given
// message if it isn't of the expected type.
//
// Returns the consumed token if successful, or NULL if the token
// was of an unexpected type.
Token expect(Compiler *compiler, TokenType expected, char *message) {
	Lexer *lexer = &compiler->vm->lexer;
	Token token = lexer_consume(lexer);

	// If the consumed token is of an unexpected type.
	if (token.type != expected) {
		// Trigger the error message.
		error(compiler, message);

		// Return a none token
		Token token;
		token.type = TOKEN_NONE;
		return token;
	} else {
		// The token was what we expected, so return it.
		return token;
	}
}
