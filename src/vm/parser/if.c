
//
//  If Statement Parsing
//

#include "if.h"
#include "expr.h"
#include "local.h"
#include "jmp.h"
#include "../bytecode.h"


// Parses the condition and body of an if or else if statement.
void parse_if_body(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Parse the conditional expression
	uint16_t local;
	scope_new(parser);
	local_new(parser, &local);
	Operand condition = expr(parser, local);
	scope_free(parser);

	if (condition.type == OP_LOCAL) {
		condition = operand_to_jump(parser, condition);
	} else if (condition.type != OP_JUMP) {
		// TODO: Implement folding
		ERROR("If condition folding unimplemented");
		return;
	}

	// Expect an opening brace
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after condition in if statement");
	lexer_next(lexer);

	// Parse the block
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect a closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close block in if statement");
	lexer_next(lexer);

	// Get the location of the false case
	uint32_t false_case = fn->bytecode_count;

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
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Skip the `if` token
	lexer_next(lexer);

	// Parse if statement
	parse_if_body(parser);

	// Save the first jump of the jump list
	int jump = -1;

	// Parse following else if statements
	while (lexer->token.type == TOKEN_ELSE_IF) {
		// Insert a jump at the end of the previous if body
		int new_jump = jmp_new(parser);
		if (jump == -1) {
			jump = new_jump;
		} else {
			jmp_append(fn, new_jump, jump);
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
		int new_jump = jmp_new(parser);
		if (jump == -1) {
			jump = new_jump;
		} else {
			jmp_append(fn, new_jump, jump);
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
	jmp_target_all(fn, jump, fn->bytecode_count);
}
