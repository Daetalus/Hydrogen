
//
//  Parser
//

#include "parser.h"



//
//  Function Scopes
//

// Create a new function scope (including the function on the interpreter).
static FunctionScope scope_new(Parser *parser) {
	FunctionScope scope;
	scope.parent = NULL;
	scope.fn_index = fn_new(parser->state);
}


// Push a function scope on top of the parser's function scope stack.
static void scope_push(Parser *parser, FunctionScope *scope) {
	scope->parent = parser->scope;
	parser->scope = scope;
}


// Pop a function from the parser's function scope stack.
static void scope_pop(Parser *parser) {

}



//
//  Parser
//

// Creates a new parser, which will append all functions, packages, etc it needs
// to define to the interpreter `state`.
Parser parser_new(HyState *state) {
	Parser parser;
	parser.state = state;
	parser.locals = vec_new(Local, 8);
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
void parser_parse(Parser *parser, Source *source) {
	parser.lexer = lexer_new(source);
}
