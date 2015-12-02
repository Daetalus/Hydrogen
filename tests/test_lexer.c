
//
//  Lexer Tests
//

#include "test.h"


// Creates a new lexer.
#define LEXER(source) Lexer lexer = lexer_new((source));


// Reads the next token from the lexer and ensures it
// matches the given type.
#define ASSERT_TOKEN(type) \
	lexer_next(&lexer);    \
	EQ(lexer.token, type);


// Ensures the next token matches the given identifier.
#define ASSERT_IDENTIFIER(contents) {                                  \
	lexer_next(&lexer);                                                \
	EQ(lexer.token, TOKEN_IDENTIFIER);                                 \
	EQ(lexer.value.identifier.length, strlen(contents));               \
	EQ_STRN(lexer.value.identifier.start, contents, strlen(contents)); \
}


// Ensures the next token matches the given string.
#define ASSERT_STRING(contents, parsed) {                              \
	lexer_next(&lexer);                                                \
	EQ(lexer.token, TOKEN_STRING);                                     \
	EQ(lexer.value.identifier.length, strlen(contents));               \
	EQ_STRN(lexer.value.identifier.start, contents, strlen(contents)); \
	char *extracted = lexer_extract_string(lexer.value.identifier);    \
	NEQ(extracted, NULL);                                              \
	EQ_STR(extracted, parsed);                                         \
	free(extracted);                                                   \
}


// Ensures the next token matches the given number.
#define ASSERT_NUMBER(expected) {     \
	lexer_next(&lexer);               \
	EQ(lexer.token, TOKEN_NUMBER);    \
	EQ(lexer.value.number, expected); \
}


// Ensures the next token matches the given integer.
#define ASSERT_INTEGER(expected) {     \
	lexer_next(&lexer);                \
	EQ(lexer.token, TOKEN_INTEGER);    \
	EQ(lexer.value.integer, expected); \
}


TEST(mathematical) {
	LEXER("+ - *\t \t  \n/ %");
	ASSERT_TOKEN(TOKEN_ADD);
	ASSERT_TOKEN(TOKEN_SUB);
	ASSERT_TOKEN(TOKEN_MUL);
	ASSERT_TOKEN(TOKEN_DIV);
	ASSERT_TOKEN(TOKEN_MOD);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(comparison) {
	LEXER("== != > < >= <=");
	ASSERT_TOKEN(TOKEN_EQ);
	ASSERT_TOKEN(TOKEN_NEQ);
	ASSERT_TOKEN(TOKEN_GT);
	ASSERT_TOKEN(TOKEN_LT);
	ASSERT_TOKEN(TOKEN_GE);
	ASSERT_TOKEN(TOKEN_LE);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(assignment) {
	LEXER("= += -= *= /= %=");
	ASSERT_TOKEN(TOKEN_ASSIGN);
	ASSERT_TOKEN(TOKEN_ADD_ASSIGN);
	ASSERT_TOKEN(TOKEN_SUB_ASSIGN);
	ASSERT_TOKEN(TOKEN_MUL_ASSIGN);
	ASSERT_TOKEN(TOKEN_DIV_ASSIGN);
	ASSERT_TOKEN(TOKEN_MOD_ASSIGN);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(boolean) {
	LEXER("&& || !");
	ASSERT_TOKEN(TOKEN_AND);
	ASSERT_TOKEN(TOKEN_OR);
	ASSERT_TOKEN(TOKEN_NOT);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(bitwise) {
	LEXER("& | ~ ^");
	ASSERT_TOKEN(TOKEN_BIT_AND);
	ASSERT_TOKEN(TOKEN_BIT_OR);
	ASSERT_TOKEN(TOKEN_BIT_NOT);
	ASSERT_TOKEN(TOKEN_BIT_XOR);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(syntax) {
	LEXER("() [] {} ,.");
	ASSERT_TOKEN(TOKEN_OPEN_PARENTHESIS);
	ASSERT_TOKEN(TOKEN_CLOSE_PARENTHESIS);
	ASSERT_TOKEN(TOKEN_OPEN_BRACKET);
	ASSERT_TOKEN(TOKEN_CLOSE_BRACKET);
	ASSERT_TOKEN(TOKEN_OPEN_BRACE);
	ASSERT_TOKEN(TOKEN_CLOSE_BRACE);
	ASSERT_TOKEN(TOKEN_COMMA);
	ASSERT_TOKEN(TOKEN_DOT);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(numbers) {
	LEXER("3 4 256 65589 3.1415926535 1.612 100.100 1.0");
	ASSERT_INTEGER(3);
	ASSERT_INTEGER(4);
	ASSERT_INTEGER(256);
	ASSERT_NUMBER(65589.0);
	ASSERT_NUMBER(3.1415926535);
	ASSERT_NUMBER(1.612);
	ASSERT_NUMBER(100.1);
	ASSERT_NUMBER(1.0);
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(strings) {
	LEXER("'hello!' 'this is a \\n\\r\\ttest \"\"str\\\"ing' '\\'' \"he''ll\\\"o\"");
	ASSERT_STRING("hello!", "hello!");
	ASSERT_STRING("this is a \\n\\r\\ttest \"\"str\\\"ing",
		"this is a \n\r\ttest \"\"str\"ing");
	ASSERT_STRING("\\'", "'");
	ASSERT_STRING("he''ll\\\"o", "he''ll\"o");
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(identifiers) {
	LEXER("this is a test _for identifiers _te231__wfes");
	ASSERT_IDENTIFIER("this");
	ASSERT_IDENTIFIER("is");
	ASSERT_IDENTIFIER("a");
	ASSERT_IDENTIFIER("test");
	ASSERT_IDENTIFIER("_for");
	ASSERT_IDENTIFIER("identifiers");
	ASSERT_IDENTIFIER("_te231__wfes");
	ASSERT_TOKEN(TOKEN_EOF);
}


TEST(keywords) {
	LEXER("true false nil if else\n\t\r\n if else while for fn");
	ASSERT_TOKEN(TOKEN_TRUE);
	ASSERT_TOKEN(TOKEN_FALSE);
	ASSERT_TOKEN(TOKEN_NIL);
	ASSERT_TOKEN(TOKEN_IF);
	ASSERT_TOKEN(TOKEN_ELSE_IF);
	ASSERT_TOKEN(TOKEN_ELSE);
	ASSERT_TOKEN(TOKEN_WHILE);
	ASSERT_TOKEN(TOKEN_FOR);
	ASSERT_TOKEN(TOKEN_FN);
	ASSERT_TOKEN(TOKEN_EOF);
}


MAIN() {
	RUN(mathematical);
	RUN(comparison);
	RUN(assignment);
	RUN(boolean);
	RUN(bitwise);
	RUN(syntax);
	RUN(numbers);
	RUN(strings);
	RUN(identifiers);
	RUN(keywords);
}
