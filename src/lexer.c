
//
//  Lexer
//


#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"



//
//  Lexer
//

// Returns the next token, ignoring the cache and
// `emit_newlines` flag.
Token lexer_next(Lexer *lexer);


// Create a new lexer with `source`.
Lexer lexer_new(char *source) {
	Lexer lexer;
	lexer.parser = parser_new(source);
	lexer.line = 1;
	lexer.cache_size = 0;
	lexer.emit_newlines = true;
	return lexer;
}


// Consumes a token, ignoring the lexer's `emit_newlines` flag.
Token lexer_cache_consume(Lexer *lexer) {
	if (lexer->cache_size == 0) {
		// Add an item to the cache
		lexer->cache[0] = lexer_next(lexer);
		lexer->cache_size++;
	}

	// Save the token we're consuming.
	Token token = lexer->cache[0];

	// Shift all items back 1 (to remove the first item).
	for (int i = 1; i < lexer->cache_size; i++) {
		lexer->cache[i - 1] = lexer->cache[i];
	}

	lexer->cache_size--;
	return token;
}


// Consumes a token, returning it.
Token lexer_consume(Lexer *lexer) {
	Token token = lexer_cache_consume(lexer);
	if (!lexer->emit_newlines && token.type == TOKEN_LINE) {
		token = lexer_cache_consume(lexer);
	}

	return token;
}


// Peek at a token `amount` ahead of the current token, ignoring
// the lexer's `emit_newlines` flag.
Token lexer_cache_peek(Lexer *lexer, int amount) {
	// If we're past the maximum cache amount, then don't
	// bother.
	if (amount >= MAX_TOKEN_CACHE_SIZE) {
		Token token;
		token.type = TOKEN_NONE;
		return token;
	}

	// Populate the cache with enough tokens to include
	// the one we're wanting to peek at.
	if (lexer->cache_size <= amount) {
		// Cache the maximum at the start of the loop, as we
		// modify the lexer's cache size in the loop, so adding
		// this directly into the loop's condition will cause it
		// to iterate forever (I wonder how I found that out...)
		int max = amount - lexer->cache_size;
		for (int i = 0; i <= max; i++) {
			Token token = lexer_next(lexer);
			lexer->cache[lexer->cache_size] = token;
			lexer->cache_size++;

			// Stop if we hit the end of the file.
			if (token.type == TOKEN_END_OF_FILE) {
				break;
			}
		}
	}

	// Return the token we're after.
	return lexer->cache[amount];
}


// Returns the token `amount` tokens in front of the current
// one.
Token lexer_peek(Lexer *lexer, int amount) {
	Token token = lexer_cache_peek(lexer, amount);

	// If we're not emitting newlines, then just get the token
	// one head of the current token.
	if (!lexer->emit_newlines && token.type == TOKEN_LINE) {
		token = lexer_cache_peek(lexer, amount + 1);
	}

	return token;
}


// Returns the current token without consuming anything.
Token lexer_current(Lexer *lexer) {
	return lexer_peek(lexer, 0);
}


// Returns true if the lexer starts with `token`.
bool lexer_match(Lexer *lexer, TokenType token) {
	return lexer_current(lexer).type == token;
}


// Returns true if the next two tokens are `one` and `two`.
bool lexer_match_two(Lexer *lexer, TokenType one, TokenType two) {
	return lexer_current(lexer).type == one &&
		lexer_peek(lexer, 1).type == two;
}


// Tells the lexer to not emit any newline tokens.
void lexer_disable_newlines(Lexer *lexer) {
	lexer->emit_newlines = false;
}


// Tells the lexer to emit newline tokens.
void lexer_enable_newlines(Lexer *lexer) {
	lexer->emit_newlines = true;
}



//
//  Parsing
//

// Consumes a character and sets `token` to `type`.
void single_token(Parser *parser, Token *token, TokenType type) {
	token->type = type;
	token->location = parser_ptr(parser);
	token->length = 1;
	parser_consume(parser);
}


// Consumes a character and if the next character matches `ch`,
// then sets `token` to `type_two`, otherwise sets `token` to
// `type_one`.
void double_token(Parser *parser, Token *token,
		TokenType type_one, char ch, TokenType type_two) {
	token->location = parser_ptr(parser);
	parser_consume(parser);
	if (parser_current(parser) == ch) {
		parser_consume(parser);
		token->type = type_two;
		token->length = 2;
	} else {
		token->type = type_one;
		token->length = 1;
	}
}


// Consumes a character and
// * If the next character matches `ch_two`, then sets `token`
//   to `type_two`.
// * If the next character matches `ch_three`, then sets `token`
//   to `type_three`.
// * Else sets the token to `type_one`.
void triple_token(Parser *parser, Token *token,
		TokenType type_one,
		char ch_two, TokenType type_two,
		char ch_three, TokenType type_three) {
	token->location = parser_ptr(parser);
	parser_consume(parser);
	if (parser_current(parser) == ch_two) {
		token->type = type_two;
		token->length = 2;
		parser_consume(parser);
	} else if (parser_current(parser) == ch_three) {
		token->type = type_three;
		token->length = 2;
		parser_consume(parser);
	} else {
		token->type = type_one;
		token->length = 1;
	}
}


// Consumes a newline, and collapses all subsequent newlines
// into this one token.
void newlines(Lexer *lexer, Token *token) {
	Parser *parser = &lexer->parser;

	// Increment the lexer's line count.
	lexer->line++;

	// Set the token
	token->type = TOKEN_LINE;
	token->location = parser_ptr(parser);
	token->length = 1;

	// Consume the newline token.
	parser_consume(parser);

	// Consume subsequent newlines.
	parser_consume_spaces_tabs(parser);
	while (is_newline(parser_current(parser))) {
		lexer->line++;
		parser_consume(parser);
		parser_consume_spaces_tabs(parser);
	}
}


// Consumes a keyword and sets the token, returning true if
// successfully matched.
bool keyword(Parser *parser, Token *token, char *keyword, TokenType type) {
	int length = strlen(keyword);
	if (parser_starts_with_identifier(parser, keyword, length)) {
		token->type = type;
		token->location = parser_ptr(parser);
		token->length = length;
		parser_move(parser, length);
		return true;
	}

	return false;
}


// Potentially consumes a keyword consisting of two, whitespace
// separated words.
bool keyword_two(Parser *parser, Token *token,
		char *keyword_one, char *keyword_two, TokenType type) {
	int length_one = strlen(keyword_one);

	// If the first keyword matches
	if (parser_starts_with_identifier(parser, keyword_one, length_one)) {
		int length_two = strlen(keyword_two);
		char *start = parser_ptr(parser);

		// Save the cursor, in case we don't match the second
		// keyword.
		parser_save(parser);

		// Move to the start of the next keyword
		parser_move(parser, length_one);
		int whitespace = parser_consume_whitespace(parser);

		if (parser_starts_with_identifier(parser, keyword_two, length_two)) {
			// Move past the second keyword
			parser_move(parser, length_two);

			// Set the token
			token->type = type;
			token->location = start;
			token->length = length_one + whitespace + length_two;
			return true;
		} else {
			// Failed to match the second keyword, so restore
			// the parser.
			parser_restore(parser);
		}
	}

	return false;
}


// Potentially consumes a number, returning true if one was.
bool parse_number(Parser *parser, Token *token) {
	if (is_digit(parser_current(parser))) {
		token->type = TOKEN_NUMBER;

		// Save the starting location
		token->location = parser_ptr(parser);

		// Fetch the number
		token->number = parser_consume_number(parser, &token->length);

		// If the number was valid, then return
		if (token->length > 0) {
			return true;
		}
	}

	return false;
}


// Potentially consumes a string literal, returning true if one
// was.
bool parse_string_literal(Parser *parser, Token *token) {
	if (is_quotation_mark(parser_current(parser))) {
		token->type = TOKEN_STRING;
		token->location = parser_consume_literal(parser, &token->length);
		if (token->location != NULL) {
			return true;
		}
		// TODO: NULL is returned when there's no closing quote
		// Handle this as an error
	}

	return false;
}


// Potentially consumes an identifier, returning true if one
// was.
bool parse_identifier(Parser *parser, Token *token) {
	if (is_identifier_start(parser_current(parser))) {
		token->type = TOKEN_IDENTIFIER;
		token->location = parser_consume_identifier(parser, &token->length);
		return true;
	}

	return false;
}


// Shorthand call for function `keyword`.
#define KEYWORD(word, type)                         \
	if (keyword(parser, &result, (word), (type))) { \
		break;                                      \
	}


// Shorthand call for function `keyword_two`.
#define KEYWORD_TWO(word_one, word_two, type)                           \
	if (keyword_two(parser, &result, (word_one), (word_two), (type))) { \
		break;                                                          \
	}


// Returns the next token, ignoring the cache and
// `emit_newlines` flag.
Token lexer_next(Lexer *lexer) {
	Parser *parser = &lexer->parser;
	Token result;

	// Check for end of file first.
	if (parser_is_eof(parser)) {
		result.type = TOKEN_END_OF_FILE;
		result.location = parser_ptr(parser);
		result.length = 0;
		return result;
	}

	// Consume whitespace before the token.
	parser_consume_spaces_tabs(parser);

	// Match the current character of the parser.
	switch(parser_current(parser)) {
		// Mathematical operators
	case '+':
		double_token(parser, &result, TOKEN_ADDITION,
			'=', TOKEN_ADDITION_ASSIGNMENT);
		break;
	case '-':
		double_token(parser, &result, TOKEN_SUBTRACTION,
			'=', TOKEN_SUBTRACTION_ASSIGNMENT);
		break;
	case '*':
		double_token(parser, &result, TOKEN_MULTIPLICATION,
			'=', TOKEN_MULTIPLICATION_ASSIGNMENT);
		break;
	case '/':
		double_token(parser, &result, TOKEN_DIVISION,
			'=', TOKEN_DIVISION_ASSIGNMENT);
		break;
	case '%':
		double_token(parser, &result, TOKEN_MODULO,
			'=', TOKEN_MODULO_ASSIGNMENT);
		break;

		// Boolean and bitwise operators
	case '~':
		single_token(parser, &result, TOKEN_BITWISE_NOT);
		break;
	case '^':
		single_token(parser, &result, TOKEN_BITWISE_XOR);
		break;
	case '&':
		double_token(parser, &result, TOKEN_BITWISE_AND,
			'&', TOKEN_BOOLEAN_AND);
		break;
	case '|':
		double_token(parser, &result, TOKEN_BITWISE_OR,
			'|', TOKEN_BOOLEAN_OR);
		break;
	case '!':
		double_token(parser, &result, TOKEN_BOOLEAN_NOT,
			'=', TOKEN_NOT_EQUAL);
		break;
	case '=':
		double_token(parser, &result, TOKEN_ASSIGNMENT,
			'=', TOKEN_EQUAL);
		break;
	case '<':
		triple_token(parser, &result, TOKEN_LESS_THAN,
			'=', TOKEN_LESS_THAN_EQUAL_TO,
			'<', TOKEN_LEFT_SHIFT);
		break;
	case '>':
		triple_token(parser, &result, TOKEN_GREATER_THAN,
			'=', TOKEN_GREATER_THAN_EQUAL_TO,
			'>', TOKEN_RIGHT_SHIFT);
		break;

		// Syntax
	case '(':
		single_token(parser, &result, TOKEN_OPEN_PARENTHESIS);
		break;
	case ')':
		single_token(parser, &result, TOKEN_CLOSE_PARENTHESIS);
		break;
	case '[':
		single_token(parser, &result, TOKEN_OPEN_BRACKET);
		break;
	case ']':
		single_token(parser, &result, TOKEN_CLOSE_BRACKET);
		break;
	case '{':
		single_token(parser, &result, TOKEN_OPEN_BRACE);
		break;
	case '}':
		single_token(parser, &result, TOKEN_CLOSE_BRACE);
		break;
	case '.':
		single_token(parser, &result, TOKEN_DOT);
		break;
	case ',':
		single_token(parser, &result, TOKEN_COMMA);
		break;

		// Newlines
	case '\n':
	case '\r':
		newlines(lexer, &result);
		break;

	default:
		// Keywords
		KEYWORD("let", TOKEN_LET);
		KEYWORD("fn", TOKEN_FUNCTION);
		KEYWORD("return", TOKEN_RETURN);

		KEYWORD("for", TOKEN_FOR);
		KEYWORD("in", TOKEN_IN);
		KEYWORD("while", TOKEN_WHILE);
		KEYWORD("loop", TOKEN_LOOP);
		KEYWORD("break", TOKEN_BREAK);

		KEYWORD("class", TOKEN_CLASS);
		KEYWORD("new", TOKEN_NEW);

		KEYWORD("true", TOKEN_TRUE);
		KEYWORD("false", TOKEN_FALSE);
		KEYWORD("nil", TOKEN_NIL);

		KEYWORD("if", TOKEN_IF);
		KEYWORD_TWO("else", "if", TOKEN_ELSE_IF);
		KEYWORD("else", TOKEN_ELSE);

		if (parse_number(parser, &result)) {
			break;
		}

		if (parse_string_literal(parser, &result)) {
			break;
		}

		if (parse_identifier(parser, &result)) {
			break;
		}

		// Unrecognised token
		result.type = TOKEN_NONE;
		result.location = NULL;
		result.length = 0;
	}

	return result;
}
