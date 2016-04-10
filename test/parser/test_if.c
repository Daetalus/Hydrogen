
//
//  If Tests
//

#include <mock_parser.h>
#include <test.h>


// Tests a single if statement
void test_if(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"if a == 3 {\n"
		"	a = 4\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, NEQ_LI, 0, 3, 0);
	jmp(&p, 2);
	ins(&p, MOV_TI, 0, 4, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests an if followed by an else
void test_if_else(void) {
	MockParser p = mock_parser(
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 4\n"
		"} else {\n"
		"	a = 5\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, NEQ_LI, 0, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_TI, 0, 4, 0);
	jmp(&p, 2);
	ins(&p, MOV_TI, 0, 5, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests an if followed by a single else if
void test_if_elseif(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, NEQ_LI, 0, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 5, 0);
	jmp(&p, 4);
	ins(&p, NEQ_LI, 0, 5, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 6, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests an if followed by multiple else ifs
void test_if_elseifs(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else if a == 7 {\n"
		"	a = 8\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, NEQ_LI, 0, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 5, 0);
	jmp(&p, 8);
	ins(&p, NEQ_LI, 0, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 6, 0);
	jmp(&p, 4);
	ins(&p, NEQ_LI, 0, 7, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 8, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests an if, followed by an else if, followed by an else
void test_if_elseif_else(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else {\n"
		"	a = 7\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, NEQ_LI, 0, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 5, 0);
	jmp(&p, 6);
	ins(&p, NEQ_LI, 0, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 6, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 7, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests an ifs, followed by multiple else ifs, followed by an else
void test_if_elseifs_else(void) {
	MockParser p = mock_parser(
		"{\n"
		"let a = 3\n"
		"if a == 4 {\n"
		"	a = 5\n"
		"} else if a == 5 {\n"
		"	a = 6\n"
		"} else if a == 6 {\n"
		"	a = 7\n"
		"} else {\n"
		"	a = 8\n"
		"}\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, NEQ_LI, 0, 4, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 5, 0);
	jmp(&p, 10);
	ins(&p, NEQ_LI, 0, 5, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 6, 0);
	jmp(&p, 6);
	ins(&p, NEQ_LI, 0, 6, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 7, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 8, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests folding an if
void test_fold_if(void) {
	MockParser p = mock_parser(
		"if true {\n"
		"	let a = 3\n"
		"}\n"
		"if false {\n"
		"	let b = 4\n"
		"}\n"
		"let c = 3\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_TI, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests folding an if with a subsequent else
void test_fold_if_else(void) {
	MockParser p = mock_parser(
		"if true {\n"
		"	let a = 3\n"
		"} else {\n"
		"	let a = 4\n"
		"}\n"
		"if false {\n"
		"	let a = 5\n"
		"} else {\n"
		"	let b = 6\n"
		"}\n"
	);

	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, MOV_LI, 0, 6, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests folding an if with a subsequent else if
void test_fold_if_elseif(void) {
	MockParser p = mock_parser(
		"let b = 10\n"
		"if true {\n"
		"	let a = 3\n"
		"} else if b == 10 {\n"
		"	let a = 4\n"
		"}\n"
		"if b == 10 {\n"
		"	let a = 3\n"
		"} else if false {\n"
		"	let a = 10\n"
		"} else if true {\n"
		"	let a = 4\n"
		"} else if false {\n"
		"	let a = 9\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 10, 0);
	ins(&p, MOV_LI, 0, 3, 0);

	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, NEQ_LI, 0, 10, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 3, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 4, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


// Tests folding an if followed by an else if, followed by an else
void test_fold_if_elseif_else(void) {
	MockParser p = mock_parser(
		"let b = 10\n"
		"if true {\n"
		"	let a = 3\n"
		"} else if b == 10 {\n"
		"	let a = 4\n"
		"} else {\n"
		"	let a = 5\n"
		"}\n"
		"if b == 10 {\n"
		"	let a = 3\n"
		"} else if false {\n"
		"	let a = 10\n"
		"} else if true {\n"
		"	let a = 4\n"
		"} else if false {\n"
		"	let a = 9\n"
		"} else {\n"
		"	let a = 5\n"
		"}\n"
	);

	ins(&p, MOV_TI, 0, 10, 0);
	ins(&p, MOV_LI, 0, 3, 0);

	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, NEQ_LI, 0, 10, 0);
	jmp(&p, 3);
	ins(&p, MOV_LI, 0, 3, 0);
	jmp(&p, 2);
	ins(&p, MOV_LI, 0, 4, 0);

	ins(&p, RET0, 0, 0, 0);
	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("If", test_if);
	test_pass("If, else", test_if_else);
	test_pass("If, else if", test_if_elseif);
	test_pass("If, else ifs", test_if_elseifs);
	test_pass("If, else if, else", test_if_elseif_else);
	test_pass("If, else ifs, else", test_if_elseifs_else);
	test_pass("Fold if", test_fold_if);
	test_pass("Fold if, else", test_fold_if_else);
	test_pass("Fold if, else if", test_fold_if_elseif);
	test_pass("Fold if, else if, else", test_fold_if_elseif_else);
	return test_run(argc, argv);
}
