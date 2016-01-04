
//
//  Parser
//

#include "parser.h"
#include "expr.h"
#include "local.h"
#include "import.h"
#include "assign.h"
#include "if.h"
#include "loop.h"
#include "fn.h"
#include "struct.h"

// The maximum number of elements in a path.
#define MAX_PATH_DEPTH 64


// Creates a new parser. Does not create a new function for the parser.
Parser parser_new(Parser *parent) {
	Parser parser;
	parser.parent = parent;
	parser.scope_depth = 0;
	parser.loop = NULL;
	parser.fn = NULL;
	ARRAY_INIT(parser.locals, Local, 64);
	ARRAY_INIT(parser.imports, Package *, 4);

	if (parent != NULL) {
		parser.lexer = parent->lexer;
		parser.vm = parent->vm;
	} else {
		parser.lexer = NULL;
		parser.vm = NULL;
	}

	return parser;
}


// Frees resources allocated by a parser.
void parser_free(Parser *parser) {
	free(parser->locals);
	free(parser->imports);
}


// Returns true if a parser is currently parsing the top level of a file.
bool parser_is_top_level(Parser *parser) {
	return parser->parent == NULL && parser->scope_depth == 1;
}


// Creates a new function on `vm`, used as `package`'s main function, and
// populates the function's bytecode based on `package`'s source code.
void parse_package(VirtualMachine *vm, Package *package) {
	// Create a lexer on the stack for all child parsers
	Lexer lexer = lexer_new(vm, package->file, package->name, package->source);
	lexer_next(&lexer);

	// Create a new parser
	Parser parser = parser_new(NULL);
	parser.vm = vm;
	parser.lexer = &lexer;
	parser.fn = fn_new(vm, package, &package->main_fn);

	// Parse import statements at the top of the file
	parse_imports(&parser);

	// Parse the rest of the file
	parse_block(&parser, TOKEN_EOF);

	// Append a return instruction
	emit(parser.fn, instr_new(RET, 0, 0, 0));

	// Free the parser we allocated
	parser_free(&parser);
}


// Parses a path (a sequence of identifiers separated by dots). Returns the
// number of elements in the path, putting the identifiers into the given list.
int parse_path(Parser *parser, Identifier *path) {
	Lexer *lexer = parser->lexer;

	// Parse a sequence of identifiers
	int count = 0;
	while (lexer->token.type == TOKEN_IDENTIFIER) {
		path[count++].start = lexer->token.start;
		path[count - 1].length = lexer->token.length;
		lexer_next(lexer);

		// Expect a dot
		if (lexer->token.type == TOKEN_DOT) {
			lexer_next(lexer);
		} else {
			break;
		}
	}

	return count;
}


// Parses an assignment or function call. Returns false if neither could be
// parsed.
bool parse_call_or_assignment(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Check for an identifier
	if (lexer->token.type != TOKEN_IDENTIFIER) {
		return false;
	}

	// Parse an identifier list
	Identifier path[MAX_PATH_DEPTH];
	int count = parse_path(parser, path);
	if (count <= 0) {
		return false;
	}

	// Check the next character
	if (lexer->token.type == TOKEN_OPEN_PARENTHESIS) {
		// Function call
		parse_fn_call(parser, path, count);
		return true;
	} else if (lexer->token.type >= TOKEN_ASSIGN &&
			lexer->token.type <= TOKEN_DIV_ASSIGN) {
		// Assignment
		parse_assignment(parser, path, count);
		return true;
	}

	return false;
}


// Parses a single statement.
void parse_statement(Parser *parser) {
	Lexer *lexer = parser->lexer;
	switch (lexer->token.type) {
		// Trigger a special error for misplaced imports
	case TOKEN_IMPORT:
		ERROR("Imports must be placed at the top of the file");
		break;

	case TOKEN_LET:
		parse_initial_assignment(parser);
		break;

	case TOKEN_IF:
		parse_if(parser);
		break;

	case TOKEN_LOOP:
		parse_loop(parser);
		break;

	case TOKEN_WHILE:
		parse_while(parser);
		break;

	case TOKEN_BREAK:
		parse_break(parser);
		break;

	case TOKEN_FN:
		parse_fn_definition(parser);
		break;

	case TOKEN_RETURN:
		parse_return(parser);
		break;

	case TOKEN_STRUCT:
		parse_struct_definition(parser);
		break;

	case TOKEN_OPEN_BRACE:
		lexer_next(lexer);
		parse_block(parser, TOKEN_CLOSE_BRACE);
		EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close block");
		lexer_next(lexer);
		break;

	default:
		// Could be a function call or an assignment
		if (parse_call_or_assignment(parser)) {
			break;
		}

		// Unrecognised statement
		UNEXPECTED("Expected statement (eg. `if`, `while`)");
		break;
	}
}


// Parses a block of statements, terminated by `terminator`.
void parse_block(Parser *parser, TokenType terminator) {
	Lexer *lexer = parser->lexer;

	// Since variables can only be accessed in the block they were created in,
	// define a new variable scope and free it once we've parsed the block
	scope_new(parser);

	// Continually parse statements until we reach the end of the file, or we
	// reach the terminating token
	while (lexer->token.type != TOKEN_EOF && lexer->token.type != terminator) {
		parse_statement(parser);
	}

	// Destroy the scope we created
	scope_free(parser);
}
