
//
//  Parser Test
//


#include "test.h"

#include "../../src/lexer.c"


START(consuming) {
	Parser parser;
	parser_new(&parser, "hello");

	ASSERT_EQ(consume_char(&parser), 'h');
	ASSERT_EQ(consume_char(&parser), 'e');
	ASSERT_EQ(consume_char(&parser), 'l');
	ASSERT_EQ(consume_char(&parser), 'l');
	ASSERT_EQ(consume_char(&parser), 'o');
	ASSERT_EQ(consume_char(&parser), '\0');
	ASSERT_EQ(consume_char(&parser), '\0');
}
END()


START(end_of_file) {
	Parser parser;
	parser_new(&parser, "he");

	ASSERT_EQ(is_eof(&parser), false);
	consume_char(&parser);
	ASSERT_EQ(is_eof(&parser), false);
	consume_char(&parser);
	ASSERT_EQ(is_eof(&parser), true);
	consume_char(&parser);
	ASSERT_EQ(is_eof(&parser), true);
}
END()


START(current_char) {
	Parser parser;
	parser_new(&parser, "hel");

	ASSERT_EQ(current_char(&parser), 'h');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'e');
	ASSERT_EQ(current_char(&parser), 'e');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'l');

	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), '\0');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), '\0');
}
END()


START(restore_state) {
	Parser parser;
	parser_new(&parser, "hello");

	ASSERT_EQ(current_char(&parser), 'h');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'e');
	char *saved = current(&parser);
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'l');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'l');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'o');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), '\0');

	restore_state(&parser, saved);
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'l');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'l');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), 'o');
	consume_char(&parser);
	ASSERT_EQ(current_char(&parser), '\0');
}
END()


START(move_cursor) {
	Parser parser;
	parser_new(&parser, "hello");

	ASSERT_EQ(current_char(&parser), 'h');
	move_cursor(&parser, 2);
	ASSERT_EQ(current_char(&parser), 'l');
	move_cursor(&parser, 1);
	ASSERT_EQ(current_char(&parser), 'l');
	move_cursor(&parser, 1);
	ASSERT_EQ(current_char(&parser), 'o');
	move_cursor(&parser, -3);
	ASSERT_EQ(current_char(&parser), 'e');
	move_cursor(&parser, -100);
	ASSERT_EQ(current_char(&parser), 'h');
	move_cursor(&parser, 200);
	ASSERT_EQ(current_char(&parser), '\0');

	move_cursor(&parser, -1);
	ASSERT_EQ(current_char(&parser), 'o');
}
END()


START(peek) {
	Parser parser;
	parser_new(&parser, "hello");

	ASSERT_EQ(peek_char(&parser, 0), 'h');
	ASSERT_EQ(peek_char(&parser, -1), 'h');
	ASSERT_EQ(peek_char(&parser, -100), 'h');
	ASSERT_EQ(peek_char(&parser, 1), 'e');
	ASSERT_EQ(peek_char(&parser, 2), 'l');
	ASSERT_EQ(peek_char(&parser, 3), 'l');
	ASSERT_EQ(peek_char(&parser, 4), 'o');
	consume_char(&parser);
	ASSERT_EQ(peek_char(&parser, 0), 'e');
	ASSERT_EQ(peek_char(&parser, 1), 'l');
	ASSERT_EQ(peek_char(&parser, -1), 'h');
	ASSERT_EQ(peek_char(&parser, -2), 'h');
	ASSERT_EQ(peek_char(&parser, -100), 'h');
	ASSERT_EQ(peek_char(&parser, 100), '\0');
}
END()


START(starts_with) {
	Parser parser;
	parser_new(&parser, "hello");

	ASSERT_EQ(starts_with(&parser, "he", 2), true);
	ASSERT_EQ(starts_with(&parser, "hello", 5), true);
	ASSERT_EQ(starts_with(&parser, "nothing", 7), false);
	ASSERT_EQ(starts_with(&parser, "hellotest", 9), false);
	ASSERT_EQ(starts_with(&parser, "hellotest", 5), true);
	ASSERT_EQ(starts_with(&parser, "hello again", 2), true);
	consume_char(&parser);
	ASSERT_EQ(starts_with(&parser, "el", 2), true);
	ASSERT_EQ(starts_with(&parser, "hel", 3), false);
	ASSERT_EQ(starts_with(&parser, "hello again", 11), false);
	move_cursor(&parser, 100);
	move_cursor(&parser, -1);
	ASSERT_EQ(starts_with(&parser, "o", 1), true);
	move_cursor(&parser, 1);
	ASSERT_EQ(starts_with(&parser, "\0", 1), true);
	ASSERT_EQ(starts_with(&parser, "something", 9), false);
}
END()


START(starts_with_identifier) {
	Parser parser;
	parser_new(&parser, "test hello.something");

	ASSERT_EQ(starts_with_identifier(&parser, "test", 4), true);
	ASSERT_EQ(starts_with_identifier(&parser, "test ", 5), false);
	ASSERT_EQ(starts_with_identifier(&parser, "t", 1), false);
	move_cursor(&parser, 5);

	ASSERT_EQ(starts_with_identifier(&parser, "hello", 5), true);
	ASSERT_EQ(starts_with_identifier(&parser, "hel", 3), false);
	ASSERT_EQ(starts_with_identifier(&parser, "nothing", 7), false);
	ASSERT_EQ(starts_with_identifier(&parser, "hello.", 6), false);
	ASSERT_EQ(starts_with_identifier(&parser, "hello.something", 15), true);
	move_cursor(&parser, 6);
	ASSERT_EQ(starts_with_identifier(&parser, "something", 9), true);
	ASSERT_EQ(starts_with_identifier(&parser, "somethin", 8), false);
}
END()


START(consume_whitespace) {
	Parser parser;
	parser_new(&parser, "1 2   3 \t\n  8 \r\n\t  \r 9");

	ASSERT_EQ(consume_char(&parser), '1');
	consume_whitespace(&parser);
	ASSERT_EQ(consume_char(&parser), '2');
	consume_whitespace(&parser);
	ASSERT_EQ(consume_char(&parser), '3');
	consume_whitespace(&parser);
	ASSERT_EQ(consume_char(&parser), '8');
	consume_whitespace(&parser);
	ASSERT_EQ(consume_char(&parser), '9');
	consume_whitespace(&parser);
	ASSERT_EQ(consume_char(&parser), '\0');
}
END()


START(consume_spaces_tabs) {
	Parser parser;
	parser_new(&parser, "1    3 \t 2 \n 9  \t\r0");

	ASSERT_EQ(consume_char(&parser), '1');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '3');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '2');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '\n');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '9');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '\r');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '0');
	consume_spaces_tabs(&parser);
	ASSERT_EQ(consume_char(&parser), '\0');
}
END()


#define TEST_CONSUME_IDENTIFIER(expected, expected_length) \
	length = 0;                                            \
	start = consume_identifier(&parser, &length);          \
	ASSERT_EQ(length, expected_length);                    \
	ASSERT_STRN_EQ(start, expected, length);


START(consume_identifier) {
	Parser parser;
	parser_new(&parser, "hello test_ing _h3ll0 another t_e_s_t");

	char *start;
	int length;
	TEST_CONSUME_IDENTIFIER("hello", 5);
	consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("test_ing", 8);
	consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("_h3ll0", 6);
	consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("another", 7);
	consume_whitespace(&parser);
	TEST_CONSUME_IDENTIFIER("t_e_s_t", 7);
}
END()


#define TEST_CONSUME_NUMBER(expected, expected_length) \
	number = 0.0;                                      \
	length = consume_number(&parser, &number);         \
	ASSERT_EQ(length, expected_length);                \
	ASSERT_EQ(number, expected);


START(consume_number) {
	Parser parser;
	parser_new(&parser, "123 32142 1 42.4 90.813 3.141592653");

	double number;
	int length;
	TEST_CONSUME_NUMBER(123.0, 3);
	consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(32142.0, 5);
	consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(1.0, 1);
	consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(42.4, 4);
	consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(90.813, 6);
	consume_whitespace(&parser);
	TEST_CONSUME_NUMBER(3.141592653, 11);
}
END()


#define TEST_STRING_LITERAL(expected, expected_length) \
	length = 0;                                        \
	start = consume_string_literal(&parser, &length);  \
	ASSERT_EQ(length, expected_length);                \
	ASSERT_STRN_EQ(start, expected, length);


START(consume_string_literal) {
	Parser parser;
	parser_new(&parser, "'hello' 'another .123()}{.[];' '' '\\'' \"hello\" \"\" \"h\\\"e\"");

	char *start;
	int length;
	TEST_STRING_LITERAL("hello", 5);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("another .123()}{.[];", 20);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("", 0);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("\\'", 2);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("hello", 5);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("", 0);
	consume_whitespace(&parser);
	TEST_STRING_LITERAL("h\\\"e", 4);
}
END()


MAIN(parser) {
	RUN(consuming)
	RUN(end_of_file)
	RUN(current_char)
	RUN(restore_state)
	RUN(move_cursor)
	RUN(peek)
	RUN(starts_with)
	RUN(starts_with_identifier)
	RUN(consume_whitespace)
	RUN(consume_spaces_tabs)
	RUN(consume_identifier)
	RUN(consume_number)
	RUN(consume_string_literal)
}
MAIN_END()
