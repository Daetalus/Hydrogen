
//
//  Lexer Tests
//

#include "test.h"


// Creates a new lexer.
#define LEXER(code)              \
	const char *source = (code); \
	Lexer lexer = lexer_new(NULL, (char *) source);


// Reads the next token from the lexer and ensures it matches the given type.
#define ASSERT_TOKEN(required) \
	lexer_next(&lexer);        \
	ASSERT_EQ(lexer.token.type, required);


// Ensures the next token is an identifier with the given name.
#define ASSERT_IDENTIFIER(contents)                               \
	lexer_next(&lexer);                                           \
	ASSERT_EQ(lexer.token.type, TOKEN_IDENTIFIER);                \
	ASSERT_EQ(lexer.token.length, strlen(contents));              \
	ASSERT_STREQN(lexer.token.start, contents, strlen(contents)); \



// Ensures the next token matches the given string.
#define ASSERT_STRING(contents, parsed) {                             \
	lexer_next(&lexer);                                               \
	ASSERT_EQ(lexer.token.type, TOKEN_STRING);                        \
	ASSERT_EQ(lexer.token.length, strlen(contents) + 2);              \
	ASSERT_STREQN(lexer.token.start + 1, contents, strlen(contents)); \
	char *extracted = lexer_extract_string(&lexer.token);             \
	ASSERT_NE(extracted, (char *) NULL);                              \
	ASSERT_STREQ(extracted, parsed);                                  \
	free(extracted);                                                  \
}


// Ensures the next token matches the given number.
#define ASSERT_NUMBER(expected)                \
	lexer_next(&lexer);                        \
	ASSERT_EQ(lexer.token.type, TOKEN_NUMBER); \
	ASSERT_EQ(lexer.token.number, expected);   \


// Ensures the next token matches the given integer.
#define ASSERT_INTEGER(expected)                \
	lexer_next(&lexer);                         \
	ASSERT_EQ(lexer.token.type, TOKEN_INTEGER); \
	ASSERT_EQ(lexer.token.integer, expected);   \


// Tests all mathematical tokens.
TEST(Lexer, Math) {
	LEXER("+ - *\t \t  \n/ %");
	ASSERT_TOKEN(TOKEN_ADD);
	ASSERT_TOKEN(TOKEN_SUB);
	ASSERT_TOKEN(TOKEN_MUL);
	ASSERT_TOKEN(TOKEN_DIV);
	ASSERT_TOKEN(TOKEN_MOD);
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests all comparison tokens.
TEST(Lexer, Comparison) {
	LEXER("== != > < >= <=");
	ASSERT_TOKEN(TOKEN_EQ);
	ASSERT_TOKEN(TOKEN_NEQ);
	ASSERT_TOKEN(TOKEN_GT);
	ASSERT_TOKEN(TOKEN_LT);
	ASSERT_TOKEN(TOKEN_GE);
	ASSERT_TOKEN(TOKEN_LE);
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests all assignment tokens.
TEST(Lexer, Assignment) {
	LEXER("= += -= *= /=");
	ASSERT_TOKEN(TOKEN_ASSIGN);
	ASSERT_TOKEN(TOKEN_ADD_ASSIGN);
	ASSERT_TOKEN(TOKEN_SUB_ASSIGN);
	ASSERT_TOKEN(TOKEN_MUL_ASSIGN);
	ASSERT_TOKEN(TOKEN_DIV_ASSIGN);
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests all boolean operator tokens.
TEST(Lexer, Boolean) {
	LEXER("&& || !");
	ASSERT_TOKEN(TOKEN_AND);
	ASSERT_TOKEN(TOKEN_OR);
	ASSERT_TOKEN(TOKEN_NOT);
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests all bitwise operator tokens.
TEST(Lexer, Bitwise) {
	LEXER("& | ~ ^ << >>");
	ASSERT_TOKEN(TOKEN_BIT_AND);
	ASSERT_TOKEN(TOKEN_BIT_OR);
	ASSERT_TOKEN(TOKEN_BIT_NOT);
	ASSERT_TOKEN(TOKEN_BIT_XOR);
	ASSERT_TOKEN(TOKEN_LEFT_SHIFT);
	ASSERT_TOKEN(TOKEN_RIGHT_SHIFT);
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests all syntax tokens.
TEST(Lexer, Syntax) {
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


// Tests integer and decimal number parsing.
TEST(Lexer, Numbers) {
	LEXER("0 3 4 256 65589 3.1415926535 1.612 100.100 1.0");
	ASSERT_INTEGER(0);
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


// Tests string literal parsing.
TEST(Lexer, Strings) {
	LEXER(
		"'hello!' "
		"'this is a \\n\\r\\ttest \"\"str\\\"ing' '\\'' "
		"\"he''ll\\\"o\""
	);

	ASSERT_STRING("hello!", "hello!");
	ASSERT_STRING("this is a \\n\\r\\ttest \"\"str\\\"ing",
		"this is a \n\r\ttest \"\"str\"ing");
	ASSERT_STRING("\\'", "'");
	ASSERT_STRING("he''ll\\\"o", "he''ll\"o");
	ASSERT_TOKEN(TOKEN_EOF);
}


// Tests identifier parsing.
TEST(Lexer, Identifiers) {
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


// Tests keyword parsing.
TEST(Lexer, Keywords) {
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
