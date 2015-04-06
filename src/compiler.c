
//
//  Compiler
//


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "compiler.h"
#include "value.h"
#include "operators.h"
#include "expression.h"


// The maximum number of else if statements that are allowed to
// follow an if statement.
#define MAX_ELSE_IF_STATEMENTS 256


// Forward definitions.
void block(Compiler *compiler, TokenType terminator);
void statement(Compiler *compiler);
void variable_assignment(Compiler *compiler);
void if_statement(Compiler *compiler);

bool match_variable_assignment(Compiler *compiler);
bool match_if_statement(Compiler *compiler);

void push_scope(Compiler *compiler);
void pop_scope(Compiler *compiler);

int index_of_local(Compiler *compiler, char *name, int length);
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
void compile(VirtualMachine *vm, Function *fn, TokenType terminator) {
	// Create a compiler for this function.
	Compiler compiler;
	compiler.vm = vm;
	compiler.fn = fn;
	compiler.local_count = 0;
	compiler.scope_depth = -1;
	compiler.string_literal_count = 0;

	// Treat the source code as a top level block, stopping when
	// we reach the terminator character.
	block(&compiler, terminator);
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
	while (!match(lexer, terminator)) {
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

	if (match(lexer, TOKEN_LINE)) {
		// Ignore empty lines
		consume(lexer);
	} else if (match_variable_assignment(compiler)) {
		variable_assignment(compiler);
	} else if (match_if_statement(compiler)) {
		if_statement(compiler);
	}
}


// Compile a variable assignment.
void variable_assignment(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Indicates whether the variable we're assigning to has been
	// defined before, or whether we're defining it for the first
	// time here.
	bool is_new_var = false;

	// Ignore newlines until the expression
	ignore_newlines(lexer);

	// Check for the let keyword.
	if (match(lexer, TOKEN_LET)) {
		is_new_var = true;

		// Consume the let keyword.
		consume(lexer);
	}

	// Expect an identifier (the variable's name).
	Token name = expect(compiler, TOKEN_IDENTIFIER,
		"Expected variable name after `let`.");
	if (name.type == TOKEN_NONE) {
		return;
	}

	// Check to see if the variable already exists.
	int index = index_of_local(compiler, name.location, name.length);
	if (is_new_var && index != -1) {
		// We're trying to create a new variable using a variable
		// name that's already taken.
		error(compiler, "Variable name `%.*s` already taken in assignment.",
			name.length, name.location);
		return;
	} else if (!is_new_var && index == -1) {
		// We're trying to assign a new value to a variable that
		// doesn't exist.
		error(compiler, "Variable `%.*s` doesn't exist in assignment.",
			name.length, name.location);
		return;
	}

	// Expect an equals sign.
	// TODO: support other types of assignment
	expect(compiler, TOKEN_ASSIGNMENT,
		"Expected `=` after variable name in assignment.");

	// Compile the expression after this. This will push bytecode
	// that will leave the resulting expression on top of the
	// stack.
	obey_newlines(lexer);
	expression(compiler, TOKEN_LINE);

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


// Compile the part of an if or else if statement where we have a
// conditional expression followed by a block. Emits bytecode for
// the expression, a conditional jump and code for the block.
//
// Expects the lexer to start on the first token of the
// expression.
void if_condition(Compiler *compiler) {
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
	int jump = emit_jump(bytecode, CODE_CONDITIONAL_JUMP);

	// Consume the opening brace of the if statement's block.
	expect(compiler, TOKEN_OPEN_BRACE,
		"Expected `{` after conditional expression in if statement.");

	// Compile the block.
	block(compiler, TOKEN_CLOSE_BRACE);

	// Consume the closing brace.
	expect(compiler, TOKEN_CLOSE_BRACE,
		"Expected `}` to close if statement block.");

	// Patch the jump instruction to point to the instruction
	// after the if statement's block.
	patch_jump(bytecode, jump);
}


// Compile an if statement.
void if_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Consume the if keyword.
	consume(lexer);

	// Compile the conditional expression and block.
	if_condition(compiler);

	// Store all the unpatched jump statements at the end of if
	// or else if blocks so we can patch all of them once we've
	// compiled the entire statement.
	int unpatched_jumps[MAX_ELSE_IF_STATEMENTS];
	int jump_count = 0;

	// Check for multiple else if statements after the if
	// statement.
	while (match(lexer, TOKEN_ELSE_IF)) {
		// Firstly append another instruction to the previous if
		// or else if statement's block. The instruction jumps
		// from the end of that block to after the entire
		// if/elseif/else statement.
		//
		// Because we need to compile all of the else if and else
		// blocks before we can patch these jump instructions,
		// store them in an array.
		unpatched_jumps[jump_count] = emit_jump(bytecode,
			CODE_CONDITIONAL_JUMP);
		jump_count++;

		// Consume the else if token.
		consume(lexer);

		// Compile the conditional expression and block.
		if_condition(compiler);
	}

	// If there's an else block to follow.
	if (match(lexer, TOKEN_ELSE)) {
		// Consume the else token.
		consume(lexer);

		// Emit an unpatched jump instruction for the if/elseif
		// statement that preceded this else statement.
		unpatched_jumps[jump_count] = emit_jump(bytecode,
			CODE_CONDITIONAL_JUMP);
		jump_count++;

		// Compile the else statement's block.
		expect(compiler, TOKEN_OPEN_BRACE,
			"Expected `{` after `else`.");
		block(compiler, TOKEN_CLOSE_BRACE);
		expect(compiler, TOKEN_CLOSE_BRACE,
			"Expected `}` to close else statement block.");
	}

	// We've compiled the entire statement now, so patch all the
	// unpatched jump statements to point here.
	for (int i = 0; i < jump_count; i++) {
		patch_jump(bytecode, unpatched_jumps[i]);
	}
}



//
//  Matching
//

// Returns true if the current sequence of tokens represent a
// variable assignment.
bool match_variable_assignment(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Assigning to a new variable looks like:
	// let name = expression
	//
	// Assigning to an already created variable looks like:
	// name = expression
	return match(lexer, TOKEN_LET) ||
		match_double(lexer, TOKEN_IDENTIFIER, TOKEN_ASSIGNMENT);
}


// Returns true if the current sequence of tokens represents an
// if statement.
bool match_if_statement(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// An if statement starts with the if token.
	return match(lexer, TOKEN_IF);
}



//
//  Function Calls
//

// Emits bytecode to call the native function for the given
// operator.
// Assumes the arguments to the call are on the stack already.
void emit_native_operator_call(Compiler *compiler, TokenType operator) {
	// A pointer to the operator call function.
	void *ptr = NULL;

	#define OPERATOR_CASE(token, function) \
		case token:                        \
			ptr = &function;               \
			break;

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
			return;
	}

	// Emit the call native bytecode.
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_CALL_NATIVE);
	emit_arg_8(bytecode, (uint64_t) ptr);
}


// Emits bytecode to call the function with the given name.
void emit_function_call(Compiler *compiler, char *name, int length) {
	VirtualMachine *vm = compiler->vm;

	// Get the index of the function in the VM's function list.
	int index = vm_index_of_function(vm, name, length);

	// If the function wasn't found.
	if (index == -1) {
		// Trigger an error.
		error(compiler, "Function `%.*s` is not defined.", length, name);
		return;
	}

	// Emit the function call.
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

	int i = compiler->local_count;
	Local *local = &compiler->locals[i];
	Bytecode *bytecode = &compiler->fn->bytecode;

	// Loop over all the variables in the compiler's locals list.
	//
	// The locals are sorted from lowest to highest scope depth,
	// so just loop over from the end of the list until we find
	// a variable that's in scope.
	while(i > 0 && local->scope_depth > compiler->scope_depth) {
		// Emit a pop instruction.
		emit(bytecode, CODE_POP);

		// Decrement the counter
		i--;
		local = &compiler->locals[i];
	}
}



//
//  Locals
//

// Returns the index of a variable in the locals list, or -1
// if the variable doesn't exist.
int index_of_local(Compiler *compiler, char *name, int length) {
	// Starting from the end of the locals list is going to be
	// (hopefully) faster, as people are more likely to use
	// variables they've defined recently.
	//
	// This isn't based on any scientific fact! It's just
	// speculation.
	for (int i = compiler->local_count; i >= 0; i--) {
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
		error(compiler, "Cannot have more than %d locals in scope.",
			MAX_LOCALS);
		return -1;
	}

	// Create the local
	compiler->local_count++;

	// Return the index as the last item in the locals list.
	return compiler->local_count - 1;
}


// Emits bytecode to push the local with the given name onto the
// stack.
void push_local(Compiler *compiler, char *name, int length) {
	int index = index_of_local(compiler, name, length);

	// Check for an undefined variable.
	if (index == -1) {
		error(compiler, "Undefined variable `%.*s`.", length, name);
		return;
	}

	// Emit the bytecode.
	Bytecode *bytecode = &compiler->fn->bytecode;
	emit(bytecode, CODE_PUSH_VARIABLE);
	emit_arg_2(bytecode, index);
}


// Emits bytecode to push a number onto the stack.
void push_number(Compiler *compiler, double number) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	uint64_t converted = as_value(number);
	emit(bytecode, CODE_PUSH_NUMBER);
	emit_arg_8(bytecode, converted);
}


// Pushes a string onto the stack.
// Returns a pointer to an unallocated string,
// so the string that will be pushed can be modified.
String * push_string(Compiler *compiler) {
	Bytecode *bytecode = &compiler->fn->bytecode;

	int index = compiler->string_literal_count;
	compiler->string_literal_count++;

	emit(bytecode, CODE_PUSH_STRING);
	emit_arg_2(bytecode, index);
	return &compiler->string_literals[index];
}



//
//  Error Handling
//

// Color codes
#define NORMAL  "\x1B[0m"
#define BOLD    "\x1B[1m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"


// Triggers the given error on the compiler.
void error(Compiler *compiler, char *fmt, ...) {
	// Start the variable arguments list.
	va_list args;
	va_start(args, fmt);

	// Print the error.
	int line = compiler->vm->lexer.line;
	fprintf(stderr, RED BOLD "error " WHITE "line %d: ", line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, NORMAL "\n");

	// Stop the variable arguments list.
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
	Token token = consume(lexer);

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
