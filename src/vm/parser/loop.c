
//
//  Loops
//

#include "loop.h"
#include "expr.h"
#include "../bytecode.h"


// Parses an infinite loop.
void parse_loop(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `loop` token
	lexer_next(lexer);

	// Expect an opening brace
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after `loop`");
	lexer_next(lexer);

	// Add the loop to the parser's linked list
	Loop loop;
	loop.jump = -1;
	loop.outer = parser->loop;
	parser->loop = &loop;

	// Parse the inner block
	uint32_t start = parser->fn->bytecode_count;
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect the closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close body of infinite loop");
	lexer_next(lexer);

	// Remove the loop from the linked list
	parser->loop = loop.outer;

	// Insert a jump statement to return to the start of the loop
	uint32_t offset = parser->fn->bytecode_count - start;
	emit(parser->fn, instr_new(LOOP, offset, 0, 0));

	// Patch break statements to here
	if (loop.jump >= 0) {
		jmp_target_all(parser->fn, loop.jump, parser->fn->bytecode_count);
	}
}


// Parses a while loop.
void parse_while(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `while` token
	lexer_next(lexer);

	// Expect an expression
	// TODO: Check condition is a jump
	uint32_t start = parser->fn->bytecode_count;
	Operand condition = expr(parser, parser->locals_count);

	// Add a loop to the linked list
	Loop loop;
	loop.jump = -1;
	loop.outer = parser->loop;
	parser->loop = &loop;

	// Parse the block
	EXPECT(TOKEN_OPEN_BRACE, "Expected `{` after condition in while loop");
	lexer_next(lexer);
	parse_block(parser, TOKEN_CLOSE_BRACE);
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close while loop block");
	lexer_next(lexer);

	// Remove the loop from the linked list
	parser->loop = loop.outer;

	// Insert a jump statement to return to the start of the loop
	uint32_t offset = parser->fn->bytecode_count - start;
	emit(parser->fn, instr_new(LOOP, offset, 0, 0));

	// Point the condition's false case here
	uint32_t after = parser->fn->bytecode_count;
	expr_patch_false_case(parser, condition, after);

	// Point all break statements here
	if (loop.jump >= 0) {
		jmp_target_all(parser->fn, loop.jump, after);
	}
}


// Parses a break statement.
void parse_break(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `break` token
	lexer_next(lexer);

	// Ensure we're inside a loop
	if (parser->loop == NULL) {
		ERROR("`break` not inside a loop");
		return;
	}

	// Emit a jump instruction
	uint32_t jump = jmp_new(parser->fn);

	// Add it to the loop's jump list
	Loop *loop = parser->loop;
	if (loop->jump == -1) {
		loop->jump = jump;
	} else {
		uint32_t last = jmp_last(parser->fn, loop->jump);
		jmp_append(parser->fn, last, jump);
	}
}
