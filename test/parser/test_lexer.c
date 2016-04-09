
//
//  Lexer Tests
//

#include <test.h>
#include <mock_lexer.h>


// Ensures the current token matches the given type, and reads the next token
void eq_token(Lexer *lexer, TokenType token) {
	eq_int(lexer->token.type, token);
	lexer_next(lexer);
}


// Ensures the next token is an identifier with the given name
void eq_ident(Lexer *lexer, char *identifier) {
	eq_int(lexer->token.type, TOKEN_IDENTIFIER);
	eq_int(lexer->token.length, strlen(identifier));
	eq_strn(lexer->token.start, identifier, lexer->token.length);
	lexer_next(lexer);
}


// Ensures the next token matches the given string
void eq_string(Lexer *lexer, char *exact, char *parsed) {
	eq_int(lexer->token.type, TOKEN_STRING);
	eq_int(lexer->token.length, strlen(exact) + 2);
	eq_strn(lexer->token.start + 1, exact, strlen(exact));

	char *extracted = malloc(lexer->token.length + 1);
	lexer_extract_string(lexer, &lexer->token, extracted);
	eq_str(extracted, parsed);
	free(extracted);

	lexer_next(lexer);
}


// Ensures the next token matches the given number
void eq_number(Lexer *lexer, double number) {
	eq_int(lexer->token.type, TOKEN_NUMBER);
	eq_num(lexer->token.number, number);
	lexer_next(lexer);
}


// Ensures the next token matches the given integer
void eq_integer(Lexer *lexer, int16_t integer) {
	eq_int(lexer->token.type, TOKEN_INTEGER);
	eq_int(lexer->token.integer, integer);
	lexer_next(lexer);
}


// Tests all mathematical tokens
void test_math(void) {
	Lexer lexer = mock_lexer("+ - *\t \t  \n/ %");
	eq_token(&lexer, TOKEN_ADD);
	eq_token(&lexer, TOKEN_SUB);
	eq_token(&lexer, TOKEN_MUL);
	eq_token(&lexer, TOKEN_DIV);
	eq_token(&lexer, TOKEN_MOD);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests all comparison tokens
void test_comparison(void) {
	Lexer lexer = mock_lexer("== != > < >= <=");
	eq_token(&lexer, TOKEN_EQ);
	eq_token(&lexer, TOKEN_NEQ);
	eq_token(&lexer, TOKEN_GT);
	eq_token(&lexer, TOKEN_LT);
	eq_token(&lexer, TOKEN_GE);
	eq_token(&lexer, TOKEN_LE);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests all assignment tokens
void test_assignment(void) {
	Lexer lexer = mock_lexer("= += -= *= /=");
	eq_token(&lexer, TOKEN_ASSIGN);
	eq_token(&lexer, TOKEN_ADD_ASSIGN);
	eq_token(&lexer, TOKEN_SUB_ASSIGN);
	eq_token(&lexer, TOKEN_MUL_ASSIGN);
	eq_token(&lexer, TOKEN_DIV_ASSIGN);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests all boolean operator tokens
void test_boolean(void) {
	Lexer lexer = mock_lexer("&& || !");
	eq_token(&lexer, TOKEN_AND);
	eq_token(&lexer, TOKEN_OR);
	eq_token(&lexer, TOKEN_NOT);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests all bitwise operator tokens
void test_bitwise(void) {
	Lexer lexer = mock_lexer("& | ~ ^ << >>");
	eq_token(&lexer, TOKEN_BIT_AND);
	eq_token(&lexer, TOKEN_BIT_OR);
	eq_token(&lexer, TOKEN_BIT_NOT);
	eq_token(&lexer, TOKEN_BIT_XOR);
	eq_token(&lexer, TOKEN_LSHIFT);
	eq_token(&lexer, TOKEN_RSHIFT);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests all syntax tokens
void test_syntax(void) {
	Lexer lexer = mock_lexer("() [] {} ,.");
	eq_token(&lexer, TOKEN_OPEN_PARENTHESIS);
	eq_token(&lexer, TOKEN_CLOSE_PARENTHESIS);
	eq_token(&lexer, TOKEN_OPEN_BRACKET);
	eq_token(&lexer, TOKEN_CLOSE_BRACKET);
	eq_token(&lexer, TOKEN_OPEN_BRACE);
	eq_token(&lexer, TOKEN_CLOSE_BRACE);
	eq_token(&lexer, TOKEN_COMMA);
	eq_token(&lexer, TOKEN_DOT);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests integer and decimal number parsing
void test_numbers(void) {
	Lexer lexer = mock_lexer(
		"0 3 4 256 65589 3.1415926535 1.612 100.100 1.0"
	);

	eq_integer(&lexer, 0);
	eq_integer(&lexer, 3);
	eq_integer(&lexer, 4);
	eq_integer(&lexer, 256);
	eq_number(&lexer, 65589.0);
	eq_number(&lexer, 3.1415926535);
	eq_number(&lexer, 1.612);
	eq_number(&lexer, 100.1);
	eq_number(&lexer, 1.0);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests string literal parsing
void test_strings(void) {
	Lexer lexer = mock_lexer(
		"'hello!' "
		"'this is a \\n\\r\\ttest \"\"str\\\"ing' '\\'' "
		"\"he''ll\\\"o\""
	);

	eq_string(&lexer, "hello!", "hello!");
	eq_string(&lexer,
		"this is a \\n\\r\\ttest \"\"str\\\"ing",
		"this is a \n\r\ttest \"\"str\"ing"
	);
	eq_string(&lexer, "\\'", "'");
	eq_string(&lexer, "he''ll\\\"o", "he''ll\"o");
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests identifier parsing
void test_identifiers(void) {
	Lexer lexer = mock_lexer(
		"this is a test _for identifiers _te231__wfes"
	);

	eq_ident(&lexer, "this");
	eq_ident(&lexer, "is");
	eq_ident(&lexer, "a");
	eq_ident(&lexer, "test");
	eq_ident(&lexer, "_for");
	eq_ident(&lexer, "identifiers");
	eq_ident(&lexer, "_te231__wfes");
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests keyword parsing
void test_keywords(void) {
	Lexer lexer = mock_lexer(
		"true false nil if else\n\t\r\n if else while for fn"
	);

	eq_token(&lexer, TOKEN_TRUE);
	eq_token(&lexer, TOKEN_FALSE);
	eq_token(&lexer, TOKEN_NIL);
	eq_token(&lexer, TOKEN_IF);
	eq_token(&lexer, TOKEN_ELSE_IF);
	eq_token(&lexer, TOKEN_ELSE);
	eq_token(&lexer, TOKEN_WHILE);
	eq_token(&lexer, TOKEN_FOR);
	eq_token(&lexer, TOKEN_FN);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests single line comments
void test_line_comments(void) {
	Lexer lexer = mock_lexer(
		"true // false this is a test\n+ - //\n//  \t\t  \n\rfn"
	);

	eq_token(&lexer, TOKEN_TRUE);
	eq_token(&lexer, TOKEN_ADD);
	eq_token(&lexer, TOKEN_SUB);
	eq_token(&lexer, TOKEN_FN);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


// Tests block comments
void test_block_comments(void) {
	Lexer lexer = mock_lexer(
		"/* this is a \n\n\r\n \t\r */ + /**/\n\r\n -/*\n*/ else \n if"
	);

	eq_token(&lexer, TOKEN_ADD);
	eq_token(&lexer, TOKEN_SUB);
	eq_token(&lexer, TOKEN_ELSE_IF);
	eq_token(&lexer, TOKEN_EOF);
	mock_lexer_free(&lexer);
}


int main(int argc, char *argv[]) {
	test_pass("Math tokens", test_math);
	test_pass("Comparison tokens", test_comparison);
	test_pass("Assignment tokens", test_assignment);
	test_pass("Boolean operation tokens", test_boolean);
	test_pass("Bitwise operation tokens", test_bitwise);
	test_pass("Syntax tokens", test_syntax);
	test_pass("Numbers", test_numbers);
	test_pass("Strings", test_strings);
	test_pass("Identifiers", test_identifiers);
	test_pass("Line comments", test_line_comments);
	test_pass("Block comments", test_block_comments);
	return test_run(argc, argv);
}
