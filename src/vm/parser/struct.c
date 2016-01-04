
//
//  Struct Parsing
//

#include "struct.h"
#include "local.h"
#include "fn.h"
#include "../bytecode.h"
#include "../value.h"


// Parses a struct definition.
void parse_struct_definition(Parser *parser) {
	Lexer *lexer = parser->lexer;

	// Skip the `struct` token
	lexer_next(lexer);

	// Expect the name of the struct
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `struct`");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Check the struct doesn't already exist
	if (struct_find(parser->vm, name, length) >= 0) {
		ERROR("Struct `%.*s` is already defined", length, name);
		return;
	}

	// Create the struct definition
	StructDefinition *def = struct_new(parser->vm, parser->fn->package);
	def->name = name;
	def->length = length;

	// Check for an optional brace
	if (lexer->token.type == TOKEN_OPEN_BRACE) {
		// Skip the opening brace
		lexer_next(lexer);

		// Parse struct fields
		while (lexer->token.type == TOKEN_IDENTIFIER) {
			int index = struct_new_field(def);
			def->fields[index].start = lexer->token.start;
			def->fields[index].length = lexer->token.length;
			def->values[index] = NIL_VALUE;
			lexer_next(lexer);

			// Expect a comma or closing brace
			if (lexer->token.type == TOKEN_COMMA) {
				lexer_next(lexer);
			} else if (lexer->token.type != TOKEN_CLOSE_BRACE) {
				UNEXPECTED("Expected `}` to close struct fields list");
			}
		}

		// Expect a closing brace
		EXPECT(TOKEN_CLOSE_BRACE, "Expected `}` to close struct fields list");
		lexer_next(lexer);
	}
}


// Parses a struct instantiation, storing the resulting struct into `slot`.
void parse_struct_instantiation(Parser *parser, uint16_t slot) {
	Lexer *lexer = parser->lexer;

	// Skip the `new` token
	lexer_next(lexer);

	// Expect the name of a struct
	EXPECT(TOKEN_IDENTIFIER, "Expected identifier after `new`");
	char *name = lexer->token.start;
	size_t length = lexer->token.length;
	lexer_next(lexer);

	// Find a struct with the given name
	int index = struct_find(parser->vm, name, length);
	if (index < 0) {
		// Struct is undefined
		ERROR("Undefined struct `%.*s` in instantiation", length, name);
		return;
	}
	StructDefinition *def = &parser->vm->structs[index];

	// Emit bytecode to create the struct
	emit(parser->fn, instr_new(STRUCT_NEW, slot, index, 0));

	// Call the constructor, if it exists
	if (def->constructor != -1) {
		// Create a new temporary slot for the return value
		uint16_t return_slot;
		scope_new(parser);
		local_new(parser, &return_slot);
		scope_free(parser);

		// Call the constructor
		OperandSelf self;
		self.type = SELF_LOCAL;
		self.slot = slot;
		self.is_method = true;
		parse_fn_call_self(parser, CALL_F, def->constructor, return_slot, &self);
	} else {
		// Expect an opening and closing parenthesis
		EXPECT(TOKEN_OPEN_PARENTHESIS,
			"Expected `(` after struct name in instantiation");
		lexer_next(lexer);
		EXPECT(TOKEN_CLOSE_PARENTHESIS,
			"Expected no arguments to struct instantiation, as struct has no"
			"constructor");
		lexer_next(lexer);
	}
}
