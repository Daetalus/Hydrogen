
//
//  Vector Tests
//

#include <test.h>
#include <vec.h>


// Tests creating a new empty vector
void test_empty(void) {
	Vec(char) vec;
	vec_new(vec, char, 4);
	eq_int(vec_len(vec), 0);
	eq_int(vec_capacity(vec), 4);
	vec_free(vec);
}


// Tests appending elements to a vector
void test_append(void) {
	Vec(char) vec;
	vec_new(vec, char, 4);
	eq_int(vec_len(vec), 0);
	eq_int(vec_capacity(vec), 4);

	vec_inc(vec);
	eq_int(vec_len(vec), 1);
	eq_int(vec_capacity(vec), 4);
	vec_last(vec) = 'a';
	eq_ch(vec_last(vec), 'a');
	eq_ch(vec_at(vec, 0), 'a');

	vec_inc(vec);
	eq_int(vec_len(vec), 2);
	eq_int(vec_capacity(vec), 4);
	vec_last(vec) = 'b';
	eq_ch(vec_last(vec), 'b');
	eq_ch(vec_at(vec, 0), 'a');
	eq_ch(vec_at(vec, 1), 'b');

	vec_inc(vec);
	vec_last(vec) = 'c';
	vec_inc(vec);
	vec_last(vec) = 'd';
	eq_int(vec_len(vec), 4);
	eq_int(vec_capacity(vec), 4);

	vec_inc(vec);
	vec_last(vec) = 'e';
	eq_int(vec_len(vec), 5);
	eq_int(vec_capacity(vec), 8);
	eq_ch(vec_at(vec, 0), 'a');
	eq_ch(vec_at(vec, 1), 'b');
	eq_ch(vec_at(vec, 2), 'c');
	eq_ch(vec_at(vec, 3), 'd');
	eq_ch(vec_at(vec, 4), 'e');

	vec_at(vec, 1) = 'f';
	eq_int(vec_len(vec), 5);
	eq_int(vec_capacity(vec), 8);
	eq_ch(vec_at(vec, 0), 'a');
	eq_ch(vec_at(vec, 1), 'f');
	eq_ch(vec_at(vec, 2), 'c');
	eq_ch(vec_at(vec, 3), 'd');
	eq_ch(vec_at(vec, 4), 'e');

	vec_last(vec) = 'g';
	eq_int(vec_len(vec), 5);
	eq_int(vec_capacity(vec), 8);
	eq_ch(vec_at(vec, 0), 'a');
	eq_ch(vec_at(vec, 1), 'f');
	eq_ch(vec_at(vec, 2), 'c');
	eq_ch(vec_at(vec, 3), 'd');
	eq_ch(vec_at(vec, 4), 'g');

	vec_free(vec);
}


// Tests inserting elements in a vector
void test_insert(void) {
	Vec(char) vec;
	vec_new(vec, char, 4);

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'b';
	vec_inc(vec);
	vec_last(vec) = 'c';
	eq_int(vec_len(vec), 3);
	eq_int(vec_capacity(vec), 4);

	vec_insert(vec, 0, 'd');
	eq_int(vec_len(vec), 4);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'd');
	eq_ch(vec_at(vec, 1), 'a');
	eq_ch(vec_at(vec, 2), 'b');
	eq_ch(vec_at(vec, 3), 'c');

	vec_insert(vec, 2, 'e');
	eq_int(vec_len(vec), 5);
	eq_int(vec_capacity(vec), 8);
	eq_ch(vec_at(vec, 0), 'd');
	eq_ch(vec_at(vec, 1), 'a');
	eq_ch(vec_at(vec, 2), 'e');
	eq_ch(vec_at(vec, 3), 'b');
	eq_ch(vec_at(vec, 4), 'c');

	vec_insert(vec, vec_len(vec) - 1, 'f');
	eq_int(vec_len(vec), 6);
	eq_int(vec_capacity(vec), 8);
	eq_ch(vec_at(vec, 0), 'd');
	eq_ch(vec_at(vec, 1), 'a');
	eq_ch(vec_at(vec, 2), 'e');
	eq_ch(vec_at(vec, 3), 'b');
	eq_ch(vec_at(vec, 4), 'c');
	eq_ch(vec_at(vec, 5), 'f');

	vec_free(vec);
}


// Tests removing elements from a vector
void test_remove(void) {
	Vec(char) vec;
	vec_new(vec, char, 4);

	vec_inc(vec);
	vec_last(vec) = 'a';
	eq_int(vec_len(vec), 1);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'a');

	vec_remove(vec, 0);
	eq_int(vec_len(vec), 0);
	eq_int(vec_capacity(vec), 4);

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'b';
	eq_int(vec_len(vec), 2);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'a');
	eq_ch(vec_at(vec, 1), 'b');

	vec_remove(vec, 0);
	eq_int(vec_len(vec), 1);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'b');

	vec_inc(vec);
	vec_last(vec) = 'a';
	eq_int(vec_len(vec), 2);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'b');
	eq_ch(vec_at(vec, 1), 'a');

	vec_remove(vec, vec_len(vec) - 1);
	eq_int(vec_len(vec), 1);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'b');

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'c';
	eq_int(vec_len(vec), 3);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'b');
	eq_ch(vec_at(vec, 1), 'a');
	eq_ch(vec_at(vec, 2), 'c');

	vec_remove(vec, 1);
	eq_int(vec_len(vec), 2);
	eq_int(vec_capacity(vec), 4);
	eq_ch(vec_at(vec, 0), 'b');
	eq_ch(vec_at(vec, 1), 'c');

	vec_free(vec);
}


int main(int argc, char *argv[]) {
	test_pass("Empty vector", test_empty);
	test_pass("Appending", test_append);
	test_pass("Insertion", test_insert);
	test_pass("Removal", test_remove);
	return test_run(argc, argv);
}
