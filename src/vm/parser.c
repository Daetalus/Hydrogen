
//
//  Parser
//


#include "parser.h"


// A local variable.
typedef struct {
	// The name of the local.
	char *name;
	int length;

	// The scope depth the local was defined at.
	uint32_t scope_depth;
} Local;


// A parser, which converts lexed source code into
// bytecode.
typedef struct _parser {
	// The virtual machine we're parsing for.
	VirtualMachine *vm;

	// A pointer to the parent parser. NULL if this parser
	// is top level.
	struct _parser *parent;

	// The lexer.
	Lexer *lexer;

	// The function we're compiling the source code into.
	Function *fn;

	// The current scope depth.
	uint32_t scope_depth;

	// All defined locals.
	Local locals[MAX_LOCALS];
	uint32_t locals_count;
} Parser;



//
//  Error Handling
//

// Triggers a custom error.
#define ERROR(...)                                     \
	err_fatal(parser->vm, parser->lexer, __VA_ARGS__); \
	return;

// Triggers an unexpected token error and returns from the
// current function.
#define UNEXPECTED(...)                                     \
	err_unexpected(parser->vm, parser->lexer, __VA_ARGS__); \
	return;


// Triggers an error if the current token doesn't match the
// given one.
#define EXPECT(expected, ...)                                   \
	if (lexer->token != (expected)) {                           \
		err_unexpected(parser->vm, parser->lexer, __VA_ARGS__); \
		return;                                                 \
	}



//
//  Imports
//

// Imports a package with the given name.
void import(Parser *parser, char *name) {

}


// Parses a multi-import statement.
void parse_multi_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Ensure there's at least one string within the
	// parentheses
	EXPECT(TOKEN_STRING, "Expected package name after `(`");

	// Expect a comma separated list of strings
	while (lexer->token == TOKEN_STRING) {
		// Import the package
		char *name = lexer_extract_string(lexer->value.identifier);
		import(parser, name);

		// Consume the string
		lexer_next(lexer);

		// Expect a comma
		EXPECT(TOKEN_COMMA, "Expected `,` after package name");
		lexer_next(lexer);
	}

	// Expect a close parenthesis
	EXPECT(TOKEN_CLOSE_PARENTHESIS, "Expected `)` to close import list");
	lexer_next(lexer);
}


// Parses a single import statement.
void parse_import(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Parse a multi-import statement if the next token is
	// an open parenthesis
	if (lexer->token == TOKEN_OPEN_PARENTHESIS) {
		// Consume the open parenthesis
		lexer_next(lexer);

		// Multi-import statement
		parse_multi_import(parser);
	} else if (lexer->token == TOKEN_STRING) {
		// Single import statement
		char *name = lexer_extract_string(lexer->value.identifier);
		import(parser, name);

		// Consume the string token
		lexer_next(lexer);
	} else {
		// Unexpected token
		UNEXPECTED("Expected package name or `(` after `import`");
	}
}


// Parses a list of import statements at the top of a file.
void parse_imports(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Continually parse import statements
	while (lexer->token == TOKEN_IMPORT) {
		// Consume the `import`
		lexer_next(lexer);

		// Parse the rest of the import
		parse_import(parser);
	}
}



//
//  Expressions
//

// Possible operator precedences.
typedef enum {
	PREC_NONE,
	PREC_OR,
	PREC_AND,
	PREC_BIT_OR,
	PREC_BIT_XOR,
	PREC_BIT_AND,
	// Equal, not equal
	PREC_EQ,
	// Less than, less than equal, greater than, greater
	// than equal
	PREC_ORD,
	// Addition, subtraction
	PREC_ADD,
	// Multiplication, division, modulo
	PREC_MUL,
} Precedence;


// The type of an operand to an expression.
typedef enum {
	OP_LOCAL,
	OP_NUMBER,
	OP_INTEGER,
	OP_STRING,
	OP_PRIMITIVE,
	OP_CALL,
	OP_FUNCTION,
} OperandType;


// An operand in an expression.
typedef struct {
	// The type of the operand.
	OperandType type;

	// The value of the operand. Numbers and strings are
	// stored as indices into the VM's number/string list.
	union {
		int16_t integer;
		uint16_t number;
		uint16_t string;
		uint16_t primitive;
		uint16_t local;
	};
} Operand;


// Returns the precedence of a binary operator.
Precedence operator_precedence(Token operator) {
	switch (operator) {
	case TOKEN_ADD:
	case TOKEN_SUB:
		return PREC_ADD;
	case TOKEN_MUL:
	case TOKEN_DIV:
	case TOKEN_MOD:
		return PREC_MUL;
	case TOKEN_EQ:
	case TOKEN_NEQ:
		return PREC_EQ;
	case TOKEN_LT:
	case TOKEN_LE:
	case TOKEN_GT:
	case TOKEN_GE:
		return PREC_ORD;
	case TOKEN_AND:
		return PREC_AND;
	case TOKEN_OR:
		return PREC_OR;
	case TOKEN_BIT_AND:
		return PREC_BIT_AND;
	case TOKEN_BIT_OR:
		return PREC_BIT_OR;
	case TOKEN_BIT_XOR:
		return PREC_BIT_XOR;
	default:
		return PREC_NONE;
	}
}


// Places an operand in the next available local slot,
// returning the slot.
uint16_t expr_discharge(Operand *operand) {

}


// Parses an expression, stopping when we reach a binary
// operator of lower precedence than the given precedence.
Operand expr_precedence(Parser *parser, Precedence precedence) {
	Lexer *lexer = lexer->next;

	// Expect a left hand side operand
	Operand left = expr_left(parser);

	// Parse a binary operator
	Token operator = lexer->token;
	while (operator_precedence(operator) > precedence) {

	}
}


// Parses an expression, returning the final result.
Operand expr(Parser *parser) {
	return expr_precedence(parser, PREC_NONE);
}


// Parses an expression, placing results into consecutive
// local slots. Returns the location of the first slot
// used.
uint16_t expr_emit(Parser *parser) {
	Operand operand = expr(parser);
	expr_discharge(&operand);
}



//
//  Variable Assignment
//

// Parses an assignment to a new variable (using a `let`
// token).
void parse_initial_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Consume the `let` token
	lexer_next(lexer);
}



//
//  Statements
//

// Parses a single statement.
void parse_statement(Parser *parser) {
	Lexer *lexer = parser->lexer;

	switch (lexer->token) {
		// Trigger a special error for misplaced imports
	case TOKEN_IMPORT:
		ERROR("Imports must be placed at the top of the file");

	case TOKEN_LET:
		parse_initial_assignment(parser);
		break;

	default:
		// Could be a function call or an assignment
		if (call_or_assignment(parser)) {
			break;
		}

		// Unrecognised statement
		UNEXPECTED("Expected statement (eg. `if`, `while`)");
		break;
	}
}


// Parses a block of statements, terminated by the given
// character.
void parse_block(Parser *parser, Token terminator) {
	Lexer *lexer = parser->lexer;

	// Continually parse statements
	while (lexer->token != TOKEN_EOF && lexer->token != terminator) {
		// Parse a single statement
		parse_statement(parser);
	}
}




//
//  Parser
//

// Parses a package into bytecode. Sets the main function
// index property on the package.
void parse_package(VirtualMachine *vm, Package *package) {
	// Create the lexer used for all parent and child parsers
	Lexer lexer = lexer_new(package->source);
	lexer_next(&lexer);

	// Create a new parser
	Parser parser;
	parser.parent = NULL;
	parser.lexer = &lexer;
	parser.fn = fn_new(vm, package, &package->main_fn);
	parser.scope_depth = 0;
	parser.locals_count = 0;

	// Parse the import statements at the top of the file
	parse_imports(&parser);

	// Parse the rest of the file
	parse_block(&parser, TOKEN_EOF);
}
