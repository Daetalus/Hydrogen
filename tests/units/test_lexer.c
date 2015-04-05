
//
//  Lexer Test
//


#include "test.h"

#include "../../src/lexer.c"


#define NEW_LEXER(source_string)    \
	char *source = (source_string); \
	Lexer lexer;                    \
	lexer_new(&lexer, source);

#define TEST_TOKEN(token_type, src_location, src_length) \
	printf("%d\n", token.type);\
	ASSERT_EQ(token.type, token_type);                   \
	ASSERT_EQ(token.location, src_location);             \
	ASSERT_EQ(token.length, src_length);

#define TEST_CONSUME_TOKEN(token_type, src_location, src_length) \
	token = consume(&lexer);                                     \
	TEST_TOKEN(token_type, src_location, src_length);

#define TEST_STRING(token_type, src_location, src_length, string) \
	ASSERT_EQ(token.type, token_type);                            \
	ASSERT_EQ(token.location, src_location);                      \
	ASSERT_EQ(token.length, src_length);                          \
	ASSERT_STRN_EQ(token.location, string, token.length);

#define TEST_CONSUME_STRING(token_type, src_location, src_length, string) \
	token = consume(&lexer);                                              \
	TEST_STRING(token_type, src_location, src_length, string);

#define TEST_DOUBLE(token_type, src_location, src_length, value) \
	ASSERT_EQ(token.type, token_type);                           \
	ASSERT_EQ(token.location, src_location);                     \
	ASSERT_EQ(token.length, src_length);                         \
	ASSERT_EQ(token.number, value);

#define TEST_CONSUME_DOUBLE(token_type, src_location, src_length, value) \
	token = consume(&lexer);                                             \
	TEST_DOUBLE(token_type, src_location, src_length, value);


START(operators) {
	NEW_LEXER("  + - \t += && \t << !=");

	Token token;
	TEST_CONSUME_TOKEN(TOKEN_ADDITION, source + 2, 1);
	TEST_CONSUME_TOKEN(TOKEN_SUBTRACTION, source + 4, 1);
	TEST_CONSUME_TOKEN(TOKEN_ADDITION_ASSIGNMENT, source + 8, 2);
	TEST_CONSUME_TOKEN(TOKEN_BOOLEAN_AND, source + 11, 2);
	TEST_CONSUME_TOKEN(TOKEN_LEFT_SHIFT, source + 16, 2);
	TEST_CONSUME_TOKEN(TOKEN_NOT_EQUAL, source + 19, 2);
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 21, 0);
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 21, 0);
}
END()


START(syntax) {
	NEW_LEXER(".({[}])");

	Token token;
	TEST_CONSUME_TOKEN(TOKEN_DOT, source, 1);
	TEST_CONSUME_TOKEN(TOKEN_OPEN_PARENTHESIS, source + 1, 1);
	TEST_CONSUME_TOKEN(TOKEN_OPEN_BRACE, source + 2, 1);
	TEST_CONSUME_TOKEN(TOKEN_OPEN_BRACKET, source + 3, 1);
	TEST_CONSUME_TOKEN(TOKEN_CLOSE_BRACE, source + 4, 1);
	TEST_CONSUME_TOKEN(TOKEN_CLOSE_BRACKET, source + 5, 1);
	TEST_CONSUME_TOKEN(TOKEN_CLOSE_PARENTHESIS, source + 6, 1);
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 7, 0);
}
END()


START(keywords) {
	NEW_LEXER("if \t\t for in\t\t fn \t else {\t else if } else\t\t\t  if");

	Token token;
	TEST_CONSUME_TOKEN(TOKEN_IF, source, 2);
	TEST_CONSUME_TOKEN(TOKEN_FOR, source + 6, 3);
	TEST_CONSUME_TOKEN(TOKEN_IN, source + 10, 2);
	TEST_CONSUME_TOKEN(TOKEN_FUNCTION, source + 15, 2);
	TEST_CONSUME_TOKEN(TOKEN_ELSE, source + 20, 4);
	TEST_CONSUME_TOKEN(TOKEN_OPEN_BRACE, source + 25, 1);
	TEST_CONSUME_TOKEN(TOKEN_ELSE_IF, source + 28, 7);
	TEST_CONSUME_TOKEN(TOKEN_CLOSE_BRACE, source + 36, 1);
	TEST_CONSUME_TOKEN(TOKEN_ELSE_IF, source + 38, 11);
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 49, 0);
}
END()


START(identifiers) {
	NEW_LEXER("hello\twhat is up\t\t testing");

	Token token;
	TEST_CONSUME_STRING(TOKEN_IDENTIFIER, source, 5, "hello");
	TEST_CONSUME_STRING(TOKEN_IDENTIFIER, source + 6, 4, "what");
	TEST_CONSUME_STRING(TOKEN_IDENTIFIER, source + 11, 2, "is");
	TEST_CONSUME_STRING(TOKEN_IDENTIFIER, source + 14, 2, "up");
	TEST_CONSUME_STRING(TOKEN_IDENTIFIER, source + 19, 7, "testing");
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 26, 0);
}
END()


START(numbers) {
	NEW_LEXER("13 23.4 42.24 3.14159265");

	Token token;
	TEST_CONSUME_DOUBLE(TOKEN_NUMBER, source, 2, 13.0);
	TEST_CONSUME_DOUBLE(TOKEN_NUMBER, source + 3, 4, 23.4);
	TEST_CONSUME_DOUBLE(TOKEN_NUMBER, source + 8, 5, 42.24);
	TEST_CONSUME_DOUBLE(TOKEN_NUMBER, source + 14, 10, 3.14159265);
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 24, 0);
}
END()


START(string_literals) {
	NEW_LEXER("'hello' \"again\", '\\''\t\t { \"\\\"\" \t''");

	Token token;
	TEST_CONSUME_STRING(TOKEN_STRING, source + 1, 5, "hello");
	TEST_CONSUME_STRING(TOKEN_STRING, source + 9, 5, "again");
	TEST_CONSUME_TOKEN(TOKEN_COMMA, source + 15, 1);
	TEST_CONSUME_STRING(TOKEN_STRING, source + 18, 2, "\\'");
	TEST_CONSUME_TOKEN(TOKEN_OPEN_BRACE, source + 24, 1);
	TEST_CONSUME_STRING(TOKEN_STRING, source + 27, 2, "\\\"");
	TEST_CONSUME_STRING(TOKEN_STRING, source + 33, 0, "");
	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 34, 0);
}
END()


START(peek) {
	NEW_LEXER("+ - * / %");

	Token token = peek(&lexer, 0);
	TEST_TOKEN(TOKEN_ADDITION, source, 1);
	token = peek(&lexer, 1);
	TEST_TOKEN(TOKEN_SUBTRACTION, source + 2, 1);
	token = peek(&lexer, 2);
	TEST_TOKEN(TOKEN_MULTIPLICATION, source + 4, 1);
	token = peek(&lexer, 100);
	TEST_TOKEN(TOKEN_END_OF_FILE, source + 9, 0);

	TEST_CONSUME_TOKEN(TOKEN_ADDITION, source, 1);

	token = peek(&lexer, 0);
	TEST_TOKEN(TOKEN_SUBTRACTION, source + 2, 1);
	token = peek(&lexer, 1);
	TEST_TOKEN(TOKEN_MULTIPLICATION, source + 4, 1);
	token = peek(&lexer, 100);
	TEST_TOKEN(TOKEN_END_OF_FILE, source + 9, 0);

	TEST_CONSUME_TOKEN(TOKEN_SUBTRACTION, source + 2, 1);
	TEST_CONSUME_TOKEN(TOKEN_MULTIPLICATION, source + 4, 1);

	token = peek(&lexer, 0);
	TEST_TOKEN(TOKEN_DIVISION, source + 6, 1);
	token = peek(&lexer, 1);
	TEST_TOKEN(TOKEN_MODULO, source + 8, 1);

	TEST_CONSUME_TOKEN(TOKEN_END_OF_FILE, source + 9, 0);
}
END()


START(extract_string_literal) {
	Parser parser;
	parser_new(&parser, "'test' 'test\t\ntesting' 'test\\t \rtest' '\\'\\\"'");
}
END()


MAIN(lexer) {
	RUN(operators)
	RUN(syntax)
	RUN(keywords)
	RUN(identifiers)
	RUN(numbers)
	RUN(string_literals)
	RUN(peek)
	// RUN(extract_string_literal)
}
MAIN_END()
