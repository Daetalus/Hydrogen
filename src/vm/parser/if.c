
//
//  If Statement Parsing
//

#include "if.h"
#include "expr.h"
#include "../bytecode.h"


// Parses the condition and body of an if or else if statement.
void parse_if_body(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Parse the conditional expression
	Operand condition = expr(parser, parser->locals_count);
	// TODO: Check condition is a jump

	// Expect an opening brace
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after condition in if statement");
	lexer_next(lexer);

	// Parse the block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect a closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close block in if statement");
	lexer_next(lexer);

	// Get the location of the false case
	uint32_t false_case = parser->fn->bytecode_count;

	// If there's another if or else if after this
	if (lexer->token.type == TOKEN_ELSE_IF || lexer->token.type == TOKEN_ELSE) {
		// An extra jump is going to be inserted, so the false case is one more
		// instruction after what we think
		false_case++;
	}

	// Set the false case of the condition
	expr_patch_false_case(parser, condition, false_case);
}


// Parses an if statement.
void parse_if(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `if` token
	lexer_next(lexer);

	// Parse if statement
	parse_if_body(parser);

	// Save the first jump of the jump list
	int jump = -1;

	// Parse following else if statements
	while (lexer->token.type == TOKEN_ELSE_IF) {
		// Insert a jump at the end of the previous if body
		int new_jump = jmp_new(parser->fn);
		if (jump == -1) {
			jump = new_jump;
		} else {

			jmp_append(parser->fn, new_jump, jump);
			jump = new_jump;
		}

		// Skip the else if token
		lexer_next(lexer);

		// Parse the body
		parse_if_body(parser);
	}

	// Check for an else statement
	if (lexer->token.type == TOKEN_ELSE) {
		// Insert a jump at the end of the previous if body
		int new_jump = jmp_new(parser->fn);
		if (jump == -1) {
			jump = new_jump;
		} else {
			jmp_append(parser->fn, new_jump, jump);
			jump = new_jump;
		}

		// Skip `else` token
		lexer_next(lexer);

		// Parse block
		EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after `else`");
		lexer_next(lexer);
		parse_block(parser, TOKEN_CLOSE_BRACE);
		EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` after else statement block");
		lexer_next(lexer);
	}

	// Patch jumps to after if statement
	jmp_target_all(parser->fn, jump, parser->fn->bytecode_count);
}
