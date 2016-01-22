
//
//  Parser
//

#include <stdarg.h>
#include <stdlib.h>

#include "parser.h"
#include "fn.h"
#include "pkg.h"
#include "vm.h"
#include "err.h"


// Returns a pointer to the current function we're emitting bytecode values to.
static inline Function * parser_fn(Parser *parser) {
	return &vec_at(parser->state->functions, parser->scope->fn_index);
}


// Forward declaration.
static void parse_block(Parser *parser, TokenType terminator);



//
//  Error Handling
//

// Expects a token with type `type` to be the current token on the lexer,
// triggering an error if it is not found.
static void err_expect(Parser *parser, char *fmt, ...) {
	HyError *err = err_new();

	// Print format string
	va_list args;
	va_start(args, fmt);
	err_print_varargs(err, fmt, args);
	va_end(args);

	// Print found token
	err_print(err, ", found ");
	Token *token = &parser->lexer.token;
	err_print_token(err, token);

	// Attach token and trigger error
	err_token(parser->state, err, token);
	err_trigger(parser->state, err);
}



//
//  Function Scopes
//

// Create a new function scope (including the function on the interpreter).
static FunctionScope scope_new(Parser *parser) {
	FunctionScope scope;
	scope.parent = NULL;
	scope.fn_index = fn_new(parser->state);
	scope.is_method = false;
	scope.first_local = parser->locals_count;
	scope.loop = NULL;
	scope.block_depth = 0;

	Function *fn = &vec_at(parser->state->functions, scope.fn_index);
	fn->package = parser->package;
	fn->source = parser->source;
	fn->line = parser->lexer.line;
	return scope;
}


// Push a function scope on top of the parser's function scope stack.
static void scope_push(Parser *parser, FunctionScope *scope) {
	scope->parent = parser->scope;
	parser->scope = scope;
}


// Pop a function from the parser's function scope stack.
static void scope_pop(Parser *parser) {
	// All blocks and locals should have been freed here, so we're safe to pop
	// the function scope
	parser->scope = parser->scope->parent;
}



//
//  Locals
//

// Reserve space for a new local, returning its index.
static uint16_t local_reserve(Parser *parser) {
	uint16_t new_size = parser->locals_count++;

	// Increment the function's frame size
	uint16_t frame_size = new_size - parser->scope->first_local;
	Function *fn = parser_fn(parser);
	if (frame_size > fn->frame_size) {
		fn->frame_size = frame_size;
	}

	return new_size;
}


// Create a new, named local, returning its index.
static uint16_t local_new(Parser *parser) {
	vec_add(parser->locals);
	Local *local = &vec_last(parser->locals);
	local->name = NULL;
	local->length = 0;
	local->block = parser->scope->block_depth;
	return local_reserve(parser);
}


// Free the uppermost local.
static void local_free(Parser *parser) {
	parser->locals_count--;

	// Check if this was a named local
	if (parser->locals_count < vec_len(parser->locals)) {
		vec_len(parser->locals)--;
	}
}


// Create a new block scope for named locals.
static void block_new(Parser *parser) {
	// Increase the block depth
	parser->scope->block_depth++;
}


// Free a block and all variables defined within it.
static void block_free(Parser *parser) {
	// No temporary locals should be allocated here
	if (vec_len(parser->locals) > 0) {
		while (vec_last(parser->locals).block >= parser->scope->block_depth) {
			local_free(parser);
		}
	}
	parser->scope->block_depth--;
}



//
//  Variable Assignment
//

// Parses an initial variable declaration (using `let`).
static void parse_declaration(Parser *parser) {

}



//
//  Blocks and Statements
//

// Parses an unconditional block (a new scope for locals).
static void parse_unconditional_block(Parser *parser) {
	Lexer *lexer = &parser->lexer;

	// Skip the opening brace
	lexer_next(lexer);

	// Parse a block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect a closing brace
	err_expect(parser, "Expected `{` to close unconditional block");
	lexer_next(lexer);
}


// Parses a single statement, like an `if` or `while` construct.
static void parse_statement(Parser *parser) {
	switch (parser->lexer.token.type) {
		// Local declaration
	case TOKEN_LET:
		parse_declaration(parser);
		break;

		// Unconditional block
	case TOKEN_OPEN_BRACE:
		parse_unconditional_block(parser);
		break;

	default:
		break;
	}
}


// Parses a block of statements until we reach the terminating token or the end
// of the file.
static void parse_block(Parser *parser, TokenType terminator) {
	Lexer *lexer = &parser->lexer;

	// Allocate a new block for locals defined in this scope
	block_new(parser);

	// Continually parse statements until we reach the terminator or end of file
	while (lexer->token.type != TOKEN_EOF && lexer->token.type != terminator) {
		parse_statement(parser);
	}

	// Free our allocated block
	block_free(parser);
}



//
//  Parser
//

// Creates a new parser, which will append all functions, packages, etc it needs
// to define to the interpreter `state`.
Parser parser_new(HyState *state, Index pkg) {
	Parser parser;
	parser.state = state;
	parser.package = pkg;
	parser.source = NOT_FOUND;
	vec_new(parser.locals, Local, 8);
	parser.locals_count = 0;
	parser.scope = NULL;
	return parser;
}


// Releases resources allocated by a parser.
void parser_free(Parser *parser) {
	vec_free(parser->locals);
}


// Parses some source code, creating a function for the top level code in the
// source.
Index parser_parse(Parser *parser, Index source) {
	// Create a new lexer from the source code
	parser->source = source;
	parser->lexer = lexer_new(parser->state, parser->package, source);

	// Allocate a new function scope for the top level of the source code
	FunctionScope scope = scope_new(parser);
	scope_push(parser, &scope);

	// Parse the top level source
	parse_block(parser, TOKEN_EOF);

	// Emit a final return instruction
	fn_emit(parser_fn(parser), RET0, 0, 0, 0);

	// Free the scope we pushed
	scope_pop(parser);
	return scope.fn_index;
}
