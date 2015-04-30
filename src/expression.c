
//
//  Expression
//


#include <stdio.h>

#include "lib/operator.h"
#include "lexer.h"
#include "value.h"
#include "compiler.h"
#include "bytecode.h"
#include "expression.h"
#include "vm.h"
#include "error.h"


// The maximum number of items (ie. operands, operators, etc.)
// that can be used in a grammar rule.
#define MAX_RULE_ITEMS 2


// The precedence of an operator.
typedef enum {
	// No precedence
	PREC_NONE,
	// Boolean or
	PREC_BOOLEAN_OR,
	// Boolean and
	PREC_BOOLEAN_AND,
	// Bitwise or
	PREC_BITWISE_OR,
	// Bitwise xor
	PREC_BITWISE_XOR,
	// Bitwise and
	PREC_BITWISE_AND,
	// Equal, not equal
	PREC_EQUALITY,
	// Less than, less than or equal to, greater than, greater
	// than or equal to
	PREC_ORDERING,
	// Shift left, shift right
	PREC_BITWISE_SHIFT,
	// Addition, subtraction
	PREC_ADDITION,
	// Multiplication, division, modulo
	PREC_MULTIPLICATION,
	// Bitwise not, boolean not, negation
	PREC_NOT,
	// Field access
	PREC_FIELD_ACCESS,
} Precedence;


// The associativity of an operator.
typedef enum {
	// Left associative, like addition and multiplication.
	//
	// This means the expression:
	// 3 + 4 + 5
	// Becomes:
	// ((3 + 4) + 5)
	ASSOC_LEFT,

	// Right associative, like exponentiation.
	//
	// This means the expression:
	// 3 ^ 4 ^ 5
	// Becomes:
	// (3 ^ (4 ^ 5))
	ASSOC_RIGHT,
} Associativity;


// The callback function to compile an operand or postfix
// operator.
typedef void (*Callback)(Expression *expression);


// Information about a prefix operator.
typedef struct {
	Precedence precedence;
	NativeFunction fn;
} PrefixOperator;


// The type of an infix operator.
typedef enum {
	INFIX_OPERATOR_NATIVE,
	INFIX_OPERATOR_CUSTOM,
} InfixOperatorType;


// Information about an infix operator.
typedef struct {
	InfixOperatorType type;
	Precedence precedence;
	Associativity associativity;

	union {
		NativeFunction native;
		Callback custom;
	};
} InfixOperator;


// Information about a postfix operator.
typedef struct {
	Callback fn;
} PostfixOperator;


// Information about an operand.
typedef struct {
	Callback fn;
} Operand;


// The type of a rule.
typedef enum {
	RULE_PREFIX,
	RULE_INFIX,
	RULE_POSTFIX,
	RULE_OPERAND,
	RULE_UNUSED,
} RuleType;


// A rule item, defining one behaviour for a token.
typedef struct {
	RuleType type;
	union {
		PrefixOperator prefix;
		InfixOperator infix;
		PostfixOperator postfix;
		Operand operand;
	};
} RuleItem;


// A rule instructing the expression parser on what to do when
// it encounters an operator.
typedef struct {
	// The number of items defined for this rule. Will be 0 if
	// the rule is unused.
	int count;

	// An array of items defined for this rule. A token may
	// refer to more than one rule (eg. a prefix and infix
	// operator).
	RuleItem items[MAX_RULE_ITEMS];
} Rule;


// Operands.
void operand_number(Expression *expression);
void operand_identifier(Expression *expression);
void operand_string_literal(Expression *expression);
void sub_expression(Expression *expression);
void operand_true(Expression *expression);
void operand_false(Expression *expression);
void operand_nil(Expression *expression);
void operand_function(Expression *expression);
void operand_class(Expression *expression);

// Custom infix operators.
void infix_field_access(Expression *expression);

// Postfix operators.
void postfix_function_call(Expression *expression);


// Define a prefix operator.
#define PREFIX(precedence, fn) \
	{RULE_PREFIX, .prefix = {  \
		(precedence),          \
		&(fn),                 \
	}}


// Define an infix operator.
#define INFIX(precedence, fn)  \
	{RULE_INFIX, .infix = {    \
		INFIX_OPERATOR_NATIVE, \
		(precedence),          \
		ASSOC_LEFT,            \
		.native = &(fn),       \
	}}


// Define a custom infix operator.
#define CUSTOM_INFIX(precedence, fn) \
	{RULE_INFIX, .infix = {          \
		INFIX_OPERATOR_CUSTOM,       \
		(precedence),                \
		ASSOC_LEFT,                  \
		.custom = &(fn),             \
	}}


// Define a postfix operator.
#define POSTFIX(fn)             \
	{RULE_POSTFIX, .postfix = { \
		&(fn),                  \
	}}


// Define an operand.
#define OPERAND(fn)             \
	{RULE_OPERAND, .operand = { \
		&(fn),                  \
	}}


// Define an unused token.
#define UNUSED() \
	{0}


// Expression rules array. The entries in the array are in order
// of the tokens as defined in the lexer, so we can simply
// index it with a token to find the rule for any token in
// constant time.
//
// Note: Must be kept in order of the `Token` enum in `lexer.h`
Rule rules[] = {
	// Addition
	{1, {INFIX(PREC_ADDITION, operator_addition)}},
	// Subtraction and negation
	{2, {
		PREFIX(PREC_NOT, operator_negation),
		INFIX(PREC_ADDITION, operator_subtraction),
	}},
	// Multiplication
	{1, {INFIX(PREC_MULTIPLICATION, operator_multiplication)}},
	// Division
	{1, {INFIX(PREC_MULTIPLICATION, operator_division)}},
	// Modulo
	{1, {INFIX(PREC_MULTIPLICATION, operator_modulo)}},

	// Boolean and
	{1, {INFIX(PREC_BOOLEAN_AND, operator_boolean_and)}},
	// Boolean or
	{1, {INFIX(PREC_BOOLEAN_OR, operator_boolean_or)}},
	// Boolean not
	{1, {PREFIX(PREC_NOT, operator_boolean_not)}},
	// Equal
	{1, {INFIX(PREC_EQUALITY, operator_equal)}},
	// Not equal
	{1, {INFIX(PREC_EQUALITY, operator_not_equal)}},
	// Less than
	{1, {INFIX(PREC_ORDERING, operator_less_than)}},
	// Less than equal to
	{1, {INFIX(PREC_ORDERING, operator_less_than_equal_to)}},
	// Greater than
	{1, {INFIX(PREC_ORDERING, operator_greater_than)}},
	// Greater than equal to
	{1, {INFIX(PREC_ORDERING, operator_greater_than_equal_to)}},

	// Left shift
	{1, {INFIX(PREC_BITWISE_SHIFT, operator_left_shift)}},
	// Right shift
	{1, {INFIX(PREC_BITWISE_SHIFT, operator_right_shift)}},
	// Bitwise and
	{1, {INFIX(PREC_BITWISE_AND, operator_bitwise_and)}},
	// Bitwise or
	{1, {INFIX(PREC_BITWISE_OR, operator_bitwise_or)}},
	// Bitwise not
	{1, {PREFIX(PREC_NOT, operator_bitwise_not)}},
	// Bitwise xor
	{1, {INFIX(PREC_BITWISE_XOR, operator_bitwise_xor)}},

	// Assignment
	UNUSED(),
	// Addition assignment
	UNUSED(),
	// Subtraction assignment
	UNUSED(),
	// Multiplication assignment
	UNUSED(),
	// Division assignment
	UNUSED(),
	// Modulo assignment
	UNUSED(),

	// Open parenthesis
	{2, {
		OPERAND(sub_expression),
		POSTFIX(postfix_function_call),
	}},
	// Close parenthesis
	UNUSED(),
	// Open bracket
	UNUSED(),
	// Close bracket
	UNUSED(),
	// Open brace
	UNUSED(),
	// Close brace
	UNUSED(),
	// Dot
	{1, {CUSTOM_INFIX(PREC_FIELD_ACCESS, infix_field_access)}},
	// Comma
	UNUSED(),

	// Let
	UNUSED(),
	// If
	UNUSED(),
	// Else
	UNUSED(),
	// Else if
	UNUSED(),
	// While
	UNUSED(),
	// Loop
	UNUSED(),
	// Break
	UNUSED(),
	// For
	UNUSED(),
	// In
	UNUSED(),
	// Class
	UNUSED(),
	// New
	{1, {OPERAND(operand_class)}},
	// Function
	{1, {OPERAND(operand_function)}},
	// Return
	UNUSED(),

	// True
	{1, {OPERAND(operand_true)}},
	// False
	{1, {OPERAND(operand_false)}},
	// Nil
	{1, {OPERAND(operand_nil)}},

	// Identifier
	{1, {OPERAND(operand_identifier)}},
	// Number
	{1, {OPERAND(operand_number)}},
	// String
	{1, {OPERAND(operand_string_literal)}},
	// Line
	UNUSED(),
	// End of file
	UNUSED(),
	// None
	UNUSED(),
};


// Compiles an expression, stopping once we reach an operator
// with a higher precedence than the given precedence level.
void parse_precedence(Expression *expression, Precedence precedence);

// Compile the left hand side of an infix operator. This could
// be a number or variable name, or a prefix operator (like
// negation).
//
// Leaves the result on the top of the stack.
void left(Expression *expression);

// Compiles a prefix operator, leaving the result on the top of
// the stack.
void prefix(Expression *expression, PrefixOperator prefix);

// Compiles an infix operator, leaving the result on the top
// of the stack.
//
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Expression *expression, InfixOperator infix);

// Peeks at the next token, assuming its a binary operator.
//
// Returns true if the next token is an operator, populating the
// infix operator argument.
bool next_infix(Expression *expression, InfixOperator *infix);


// Create a new expression.
Expression expression_new(Compiler *compiler,
		ExpressionTerminator terminator) {
	Expression expression;
	expression.compiler = compiler;
	expression.terminator = terminator;
	expression.is_only_function_call = false;
	return expression;
}


// Generates bytecode to evaluate an expression parsed from the
// compiler's lexer. Leaves the result of the expression on the
// top of the stack. Stops parsing when `terminator` returns
// true.
//
// Triggers an error on the compiler if the expression fails to
// parse.
//
// The terminator token is not consumed.
void expression_compile(Expression *expression) {
	parse_precedence(expression, PREC_NONE);
}


// Compiles an expression, stopping once we reach an operator
// with a higher precedence than `precedence`.
void parse_precedence(Expression *expression, Precedence precedence) {
	InfixOperator operator;

	// Compile the left hand side of an infix operator
	left(expression);

	// Get the infix operator after the left argument
	if (!next_infix(expression, &operator)) {
		return;
	}

	expression->is_only_function_call = false;

	// Keep compiling operators until we reach the end of the
	// expression, or an operator of higher precedence than the
	// one we're allowed.
	while (precedence < operator.precedence) {
		// Compile an infix operator
		infix(expression, operator);
		expression->is_only_function_call = false;

		// Fetch the next operator
		if (!next_infix(expression, &operator)) {
			break;
		}
	}
}


// Searches the rules list for a rule matching a particular
// token and type, returning true and a rule item if one was
// found, else returns false.
bool find_rule(TokenType token, RuleType type, RuleItem **item) {
	Rule rule = rules[token];

	for (int i = 0; i < rule.count; i++) {
		if (rule.items[i].type == type) {
			// Found a matching rule
			if (item != NULL) {
				*item = &rule.items[i];
			}

			return true;
		}
	}

	return false;
}


// Checks for a postfix operator, compiling it if one exists.
void postfix(Expression *expression) {
	Compiler *compiler = expression->compiler;
	Lexer *lexer = &compiler->vm->lexer;

	// Fetch the postfix operand we're attempting to compile
	RuleItem *rule;
	Token token = lexer_current(lexer);
	if (!find_rule(token.type, RULE_POSTFIX, &rule)) {
		return;
	}

	// Compile the postfix operator
	rule->postfix.fn(expression);
}


// Compile the left hand side of an infix operator. This could
// be a number or variable name, or a prefix operator (like
// negation).
//
// Leaves the result on the top of the stack.
void left(Expression *expression) {
	Compiler *compiler = expression->compiler;
	Lexer *lexer = &compiler->vm->lexer;

	// Fetch the token we're using as the operand or prefix
	// operator
	Token token = lexer_current(lexer);

	// Check for a continuation of the expression on the next
	// line
	if (token.type == TOKEN_LINE) {
		Token after = lexer_peek(lexer, 1);

		if (find_rule(after.type, RULE_PREFIX, NULL) ||
				find_rule(after.type, RULE_OPERAND, NULL)) {
			// Continued on the next line
			lexer_consume(lexer);
			token = after;
		}
	}

	RuleItem *rule;
	if (find_rule(token.type, RULE_OPERAND, &rule)) {
		// Compile the operand
		rule->operand.fn(expression);

		// Look for a potential postfix operator
		postfix(expression);
	} else if (find_rule(token.type, RULE_PREFIX, &rule)) {
		// A prefix operator
		prefix(expression, rule->prefix);
	} else {
		// Expected operand
		error(lexer->line, "Expected operand in expression, found `%.*s`",
			token.length, token.location);
	}
}


// Compiles a prefix operator, leaving the result on the top of
// the stack.
void prefix(Expression *expression, PrefixOperator prefix) {
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	// Consume the prefix operator token
	lexer_consume(lexer);

	// Compile the argument to the prefix operator
	parse_precedence(expression, 0);

	// Emit the native call
	emit_call_native(bytecode, prefix.fn);
}


// Compiles an infix operator, leaving the result on the top
// of the stack.
//
// Assumes the left side of the operator is already on the
// top of the stack.
void infix(Expression *expression, InfixOperator infix) {
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	// Determine precedence level
	Precedence precedence = infix.precedence;
	if (infix.associativity == ASSOC_RIGHT) {
		precedence--;
	}

	// Consume the operator token
	lexer_consume(lexer);

	// Evaluate the right hand side of the expression, leaving
	// the result on the top of the stack
	parse_precedence(expression, precedence);

	// Emit the native call for this operator
	if (infix.type == INFIX_OPERATOR_NATIVE) {
		emit_call_native(bytecode, infix.native);
	} else {
		infix.custom(expression);
	}
}


// Peeks at the next token, assuming its a binary operator.
//
// Returns true if the next token is an operator, populating the
// infix operator argument.
bool next_infix(Expression *expr, InfixOperator *infix) {
	Lexer *lexer = &expr->compiler->vm->lexer;

	// Fetch the next token
	Token token = lexer_current(lexer);
	RuleItem *rule;

	if (token.type == TOKEN_END_OF_FILE ||
			(expr->terminator != NULL && expr->terminator(token))) {
		// Reached the terminating token
		return false;
	} else if (token.type == TOKEN_LINE) {
		// The expression may continue over the newline
		token = lexer_peek(lexer, 1);
		if (!find_rule(token.type, RULE_INFIX, &rule)) {
			// No continuation, so terminate
			return false;
		}

		// Consume the newline token
		lexer_consume(lexer);
	} else {
		// We set the rule when handling newlines, but we
		// haven't done so for any other token yet
		if (!find_rule(token.type, RULE_INFIX, &rule)) {
			// Not an infix operator
			error(lexer->line, "Expected binary operator, found `%.*s`",
				token.length, token.location);
		}
	}

	*infix = rule->infix;
	return true;
}



//
//  Custom Infix Operators
//

// Compile a field access operator (a dot).
void infix_field_access(Expression *expression) {

}



//
//  Operands
//

// Compile a number.
void operand_number(Expression *expression) {
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	Token number = lexer_consume(lexer);
	emit_push_number(bytecode, number.number);
}


// Compile an identifier.
void operand_identifier(Expression *expression) {
	VirtualMachine *vm = expression->compiler->vm;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;
	Lexer *lexer = &expression->compiler->vm->lexer;

	Token name = lexer_consume(lexer);

	// Check for a local variable
	Variable var = capture_variable(expression->compiler, name.location,
		name.length);
	if (var.type != VARIABLE_UNDEFINED) {
		emit_push_variable(bytecode, &var);
		return;
	}

	// Check for a native function with the same name
	int native = vm_find_native(vm, name.location, name.length);
	if (native != -1) {
		emit_push_native(bytecode, native);
		return;
	}

	// The variable is undefined
	error(lexer->line, "Undefined variable `%.*s`", name.length,
		name.location);
}


// Compile a string literal.
void operand_string_literal(Expression *expression) {
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	Token literal = lexer_consume(lexer);
	String **string;
	char *invalid_sequence;
	int index = vm_new_string_literal(expression->compiler->vm, &string);
	*string = parser_extract_literal(literal.location, literal.length,
		&invalid_sequence);

	if (invalid_sequence != NULL) {
		// Invalid escape sequence in string
		error(lexer->line,
			"Invalid escape sequence `%.*s` in string", 2, invalid_sequence);
	} else {
		emit(bytecode, CODE_PUSH_STRING);
		emit_arg_2(bytecode, index);
	}
}


// Returns true when a sub-expression should be terminated (at a
// close parenthesis).
bool should_terminate_sub_expression(Token token) {
	return token.type == TOKEN_CLOSE_PARENTHESIS;
}


// Compile a sub-expression (surrounded by parentheses).
void sub_expression(Expression *expression) {
	Lexer *lexer = &expression->compiler->vm->lexer;
	lexer_consume(lexer);

	Expression sub_expression = expression_new(expression->compiler,
		&should_terminate_sub_expression);
	expression_compile(&sub_expression);

	expect(lexer, TOKEN_CLOSE_PARENTHESIS,
		"Expected `)` to close `(` in expression");
}


// Compile a true constant.
void operand_true(Expression *expression) {
	Bytecode *bytecode = &expression->compiler->fn->bytecode;
	Lexer *lexer = &expression->compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_TRUE);
}


// Compile a false constant.
void operand_false(Expression *expression) {
	Bytecode *bytecode = &expression->compiler->fn->bytecode;
	Lexer *lexer = &expression->compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_FALSE);
}


// Compile a nil constant.
void operand_nil(Expression *expression) {
	Bytecode *bytecode = &expression->compiler->fn->bytecode;
	Lexer *lexer = &expression->compiler->vm->lexer;
	lexer_consume(lexer);
	emit(bytecode, CODE_PUSH_NIL);
}


// Compile a function operand.
void operand_function(Expression *expression) {
	VirtualMachine *vm = expression->compiler->vm;
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	// Consume the function keyword
	lexer_consume(lexer);

	// Define a function
	Function *fn;
	int index = vm_new_function(vm, &fn);

	// Consume the function's arguments list
	function_definition_arguments(expression->compiler, fn);

	// Expect an opening brace for the function's block
	lexer_disable_newlines(lexer);
	expect(lexer, TOKEN_OPEN_BRACE,
		"Expected `{` after arguments list in anonymous function");

	// Compile the function's block
	fn->bytecode = bytecode_new(DEFAULT_INSTRUCTIONS_CAPACITY);
	lexer_enable_newlines(lexer);
	compile(vm, expression->compiler, fn, TOKEN_CLOSE_BRACE);

	// Expect a closing brace after the function's block
	expect(lexer, TOKEN_CLOSE_BRACE,
		"Expected `}` to close anonymous function block");

	// Push the function index
	emit(bytecode, CODE_PUSH_FUNCTION);
	emit_arg_2(bytecode, index);
}


// Compiles a class constructor (the `new` keyword).
void operand_class(Expression *expression) {
	VirtualMachine *vm = expression->compiler->vm;
	Lexer *lexer = &expression->compiler->vm->lexer;
	Bytecode *bytecode = &expression->compiler->fn->bytecode;

	// Consume the new keyword
	lexer_disable_newlines(lexer);
	lexer_consume(lexer);

	// Expect the name of the class we're creating an instance
	// of (an identifier)
	Token name = expect(lexer, TOKEN_IDENTIFIER,
		"Expected class name after `new`");
	lexer_enable_newlines(lexer);

	// Get the index of the class we're instantiating
	int index = vm_find_class(vm, name.location, name.length);
	if (index == -1) {
		error(lexer->line, "Class `%.*s` is undefined", name.length,
			name.location);
	}

	// Emit an instantiation instruction
	emit(bytecode, CODE_INSTANTIATE_CLASS);
	emit_arg_2(bytecode, index);

	// Expect an opening and closing parenthesis, where the
	// arguments to the constructor call would normally go.
	//
	// This must go on the same line as the class name like all
	// other function calls.
	expect(lexer, TOKEN_OPEN_PARENTHESIS, "Expected `()` after class name");
	expect(lexer, TOKEN_CLOSE_PARENTHESIS, "Expected `()` after class name");

	// TODO emit call to constructor
}



//
//  Postfix Operators
//

// Returns true if the token should terminate a function call
// argument.
bool should_terminate_function_call(Token token) {
	return token.type == TOKEN_COMMA || token.type == TOKEN_CLOSE_PARENTHESIS;
}


// Compiles a set of function call arguments as expressions
// separated by commas. Expects the compiler to start on an
// opening parenthesis, and consumes a closing parenthesis after
// the arguments list.
//
// Returns the number of arguments compiled.
int function_call_arguments(Compiler *compiler) {
	Lexer *lexer = &compiler->vm->lexer;

	// Consume the opening parenthesis
	lexer_disable_newlines(lexer);
	expect(lexer, TOKEN_OPEN_PARENTHESIS,
		"Expected `(` to begin function call arguments");

	// Consume expressions separated by commas
	int arity = 0;
	while (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
		// Compile an expression
		lexer_enable_newlines(lexer);
		Expression expression = expression_new(compiler,
			&should_terminate_function_call);
		expression_compile(&expression);
		lexer_disable_newlines(lexer);
		arity++;

		if (lexer_match(lexer, TOKEN_COMMA)) {
			// Another argument
			lexer_consume(lexer);
		} else if (!lexer_match(lexer, TOKEN_CLOSE_PARENTHESIS)) {
			// Unrecognised operator
			Token token = lexer_current(lexer);
			error(lexer->line,
				"Unexpected `%.*s` in arguments to function call",
				token.length, token.location);
		}
	}

	// Consume the close parenthesis and return
	lexer_consume(lexer);
	lexer_enable_newlines(lexer);
	return arity;
}


// Compile a postfix function call.
void postfix_function_call(Expression *expression) {
	// Push the function call arguments onto the stack
	int arity = function_call_arguments(expression->compiler);

	// Push a call to the function
	Bytecode *bytecode = &expression->compiler->fn->bytecode;
	emit_call(bytecode, arity);

	expression->is_only_function_call = true;
}
