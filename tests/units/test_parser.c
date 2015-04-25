
//
//  Parser Test
//


#include "test.h"

#include "../../src/parser.h"


START(consuming) {
	Parser parser = parser_new("hello");

	ASSERT_EQ(parser_current(&parser), 'h');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'e');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'o');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
	parser_consume(&parser);
}
END()


START(end_of_file) {
	Parser parser = parser_new("he");

	ASSERT_EQ(parser_is_eof(&parser), false);
	parser_consume(&parser);
	ASSERT_EQ(parser_is_eof(&parser), false);
	parser_consume(&parser);
	ASSERT_EQ(parser_is_eof(&parser), true);
	parser_consume(&parser);
	ASSERT_EQ(parser_is_eof(&parser), true);
}
END()


START(current) {
	Parser parser = parser_new("hel");

	ASSERT_EQ(parser_current(&parser), 'h');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'e');
	ASSERT_EQ(parser_current(&parser), 'e');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');

	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
}
END()


START(restore_state) {
	Parser parser = parser_new("hello");

	ASSERT_EQ(parser_current(&parser), 'h');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'e');
	parser_save(&parser);
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'o');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');

	parser_restore(&parser);
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), 'o');
	parser_consume(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
}
END()


START(move) {
	Parser parser = parser_new("hello");

	ASSERT_EQ(parser_current(&parser), 'h');
	parser_move(&parser, 2);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_move(&parser, 1);
	ASSERT_EQ(parser_current(&parser), 'l');
	parser_move(&parser, 1);
	ASSERT_EQ(parser_current(&parser), 'o');
	parser_move(&parser, -3);
	ASSERT_EQ(parser_current(&parser), 'e');
	parser_move(&parser, -1);
	ASSERT_EQ(parser_current(&parser), 'h');
	parser_move(&parser, 5);
	ASSERT_EQ(parser_current(&parser), '\0');

	parser_move(&parser, -1);
	ASSERT_EQ(parser_current(&parser), 'o');
}
END()


START(peek) {
	Parser parser = parser_new("hello");

	ASSERT_EQ(parser_peek(&parser, 0), 'h');
	ASSERT_EQ(parser_peek(&parser, 1), 'e');
	ASSERT_EQ(parser_peek(&parser, 2), 'l');
	ASSERT_EQ(parser_peek(&parser, 3), 'l');
	ASSERT_EQ(parser_peek(&parser, 4), 'o');
	parser_consume(&parser);
	ASSERT_EQ(parser_peek(&parser, 0), 'e');
	ASSERT_EQ(parser_peek(&parser, 1), 'l');
	ASSERT_EQ(parser_peek(&parser, 100), '\0');
}
END()


START(starts_with) {
	Parser parser = parser_new("hello");

	ASSERT_EQ(parser_starts_with(&parser, "he", 2), true);
	ASSERT_EQ(parser_starts_with(&parser, "hello", 5), true);
	ASSERT_EQ(parser_starts_with(&parser, "nothing", 7), false);
	ASSERT_EQ(parser_starts_with(&parser, "hellotest", 9), false);
	ASSERT_EQ(parser_starts_with(&parser, "hellotest", 5), true);
	ASSERT_EQ(parser_starts_with(&parser, "hello again", 2), true);
	parser_consume(&parser);
	ASSERT_EQ(parser_starts_with(&parser, "el", 2), true);
	ASSERT_EQ(parser_starts_with(&parser, "hel", 3), false);
	ASSERT_EQ(parser_starts_with(&parser, "hello again", 11), false);
	parser_move(&parser, 100);
	parser_move(&parser, -1);
	ASSERT_EQ(parser_starts_with(&parser, "o", 1), true);
	parser_move(&parser, 1);
	ASSERT_EQ(parser_starts_with(&parser, "\0", 1), true);
	ASSERT_EQ(parser_starts_with(&parser, "something", 9), false);
}
END()


START(starts_with_identifier) {
	Parser parser = parser_new("test hello.something");

	ASSERT_EQ(parser_starts_with_identifier(&parser, "test", 4), true);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "test ", 5), false);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "t", 1), false);
	parser_move(&parser, 5);

	ASSERT_EQ(parser_starts_with_identifier(&parser, "hello", 5), true);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "hel", 3), false);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "nothing", 7), false);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "hello.", 6), false);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "hello.something", 15), true);
	parser_move(&parser, 6);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "something", 9), true);
	ASSERT_EQ(parser_starts_with_identifier(&parser, "somethin", 8), false);
}
END()


START(consume_whitespace) {
	Parser parser = parser_new("1 2   3 \t\n  8 \r\n\t  \r 9");

	ASSERT_EQ(parser_current(&parser), '1');
	parser_consume(&parser);
	parser_consume_whitespace(&parser);
	ASSERT_EQ(parser_current(&parser), '2');
	parser_consume(&parser);
	parser_consume_whitespace(&parser);
	ASSERT_EQ(parser_current(&parser), '3');
	parser_consume(&parser);
	parser_consume_whitespace(&parser);
	ASSERT_EQ(parser_current(&parser), '8');
	parser_consume(&parser);
	parser_consume_whitespace(&parser);
	ASSERT_EQ(parser_current(&parser), '9');
	parser_consume(&parser);
	parser_consume_whitespace(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
	parser_consume(&parser);
}
END()


START(consume_spaces_tabs) {
	Parser parser = parser_new("1    3 \t 2 \n 9  \t\r0");

	ASSERT_EQ(parser_current(&parser), '1');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '3');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '2');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '\n');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '9');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '\r');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '0');
	parser_consume(&parser);
	parser_consume_spaces_tabs(&parser);
	ASSERT_EQ(parser_current(&parser), '\0');
	parser_consume(&parser);
}
END()


#define TEST_CONSUME_IDENTIFIER(expected, expected_length) \
	length = 0;                                            \
	start = parser_consume_identifier(&parser, &length);   \
	ASSERT_EQ(length, expected_length);                    \
	ASSERT_STRN_EQ(start, expected, length);


START(consume_identifier) {
	Parser parser = parser_new("hello test_ing _h3ll0 another t_e_s_t");

	char *start;
	int length;
	TEST_CONSUME_IDENTIFIER("hello", 5);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("test_ing", 8);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("_h3ll0", 6);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("another", 7);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("t_e_s_t", 7);
}
END()


#define TEST_CONSUME_NUMBER(expected, expected_length) \
	length = 0;                                        \
	number = parser_consume_number(&parser, &length);  \
	ASSERT_EQ(length, expected_length);                \
	ASSERT_EQ(number, expected);


START(consume_number) {
	Parser parser = parser_new("123 32142 1 42.4 90.813 3.141592653");

	double number;
	int length;
	TEST_CONSUME_NUMBER(123.0, 3);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(32142.0, 5);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(1.0, 1);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(42.4, 4);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(90.813, 6);
	parser_consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(3.141592653, 11);
}
END()


#define TEST_STRING_LITERAL(expected, expected_length) \
	length = 0;                                        \
	start = parser_consume_literal(&parser, &length);  \
	ASSERT_EQ(length, expected_length);                \
	ASSERT_STRN_EQ(start, expected, length);


START(consume_string_literal) {
	Parser parser = parser_new(
		"'hello' 'another .123()}{.[];' '' '\\'' \"hello\" \"\" \"h\\\"e\"");

	char *start;
	int length;
	TEST_STRING_LITERAL("hello", 5);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("another .123()}{.[];", 20);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("", 0);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("\\'", 2);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("hello", 5);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("", 0);
	parser_consume_whitespace(&parser);
	TEST_STRING_LITERAL("h\\\"e", 4);
}
END()


START_MAIN(parser) {
	RUN(consuming)
	RUN(end_of_file)
	RUN(current)
	RUN(restore_state)
	RUN(move)
	RUN(peek)
	RUN(starts_with)
	RUN(starts_with_identifier)
	RUN(consume_whitespace)
	RUN(consume_spaces_tabs)
	RUN(consume_identifier)
	RUN(consume_number)
	RUN(consume_string_literal)
}
END_MAIN()
