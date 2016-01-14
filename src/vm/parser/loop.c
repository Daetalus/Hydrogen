
//
//  Loops
//

#include "loop.h"
#include "expr.h"
#include "local.h"
#include "jmp.h"
#include "../bytecode.h"


// Parses an infinite loop.
void parse_loop(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

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
	uint32_t start = fn->bytecode_count;
	parse_block(parser, TOKEN_CLOSE_BRACE);

	// Expect the closing brace
	EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close body of infinite loop");
	lexer_next(lexer);

	// Remove the loop from the linked list
	parser->loop = loop.outer;

	// Insert a jump statement to return to the start of the loop
	uint32_t offset = fn->bytecode_count - start;
	parser_emit(parser, LOOP, offset, 0, 0);

	// Patch break statements to here
	if (loop.jump >= 0) {
		jmp_target_all(fn, loop.jump, fn->bytecode_count);
	}
}


// Parses a while loop.
void parse_while(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Skip the `while` token
	lexer_next(lexer);

	// Expect an expression
	uint32_t start = fn->bytecode_count;

	uint16_t local;
	scope_new(parser);
	local_new(parser, &local);
	Operand condition = expr(parser, local);
	scope_free(parser);

	if (condition.type == OP_LOCAL) {
		condition = operand_to_jump(parser, condition);
	} else if (condition.type != OP_JUMP) {
		// TODO: Implement folding
		ERROR("While condition folding unimplemented");
		return;
	}

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
	uint32_t offset = fn->bytecode_count - start;
	parser_emit(parser, LOOP, offset, 0, 0);

	// Point the condition's false case here
	uint32_t after = fn->bytecode_count;
	expr_patch_false_case(parser, condition, after);

	// Point all break statements here
	if (loop.jump >= 0) {
		jmp_target_all(fn, loop.jump, after);
	}
}


// Parses a break statement.
void parse_break(Parser *parser) {
	Lexer *lexer = parser->lexer;
	Function *fn = &parser->vm->functions[parser->fn_index];

	// Skip the `break` token
	lexer_next(lexer);

	// Ensure we're inside a loop
	if (parser->loop == NULL) {
		ERROR("`break` not inside a loop");
		return;
	}

	// Emit a jump instruction
	uint32_t jump = parser_emit(parser, JMP, 0, 0, 0);

	// Add it to the loop's jump list
	Loop *loop = parser->loop;
	if (loop->jump == -1) {
		loop->jump = jump;
	} else {
		uint32_t last = jmp_last(fn, loop->jump);
		jmp_append(fn, last, jump);
	}
}
