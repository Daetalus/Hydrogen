
//
//  Vector Tests
//

extern "C" {
#include <vec.h>
}

#include <gtest/gtest.h>


// Tests creating a new empty vector.
TEST(Vector, Empty) {
	Vec(char) vec;
	vec_new(vec, char, 4);
	ASSERT_EQ(vec_len(vec), 0u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	vec_free(vec);
}


// Tests appending elements to a vector.
TEST(Vector, Append) {
	Vec(char) vec;
	vec_new(vec, char, 4);
	ASSERT_EQ(vec_len(vec), 0u);
	ASSERT_EQ(vec_capacity(vec), 4u);

	vec_inc(vec);
	ASSERT_EQ(vec_len(vec), 1u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	vec_last(vec) = 'a';
	ASSERT_EQ(vec_last(vec), 'a');
	ASSERT_EQ(vec_at(vec, 0), 'a');

	vec_inc(vec);
	ASSERT_EQ(vec_len(vec), 2u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	vec_last(vec) = 'b';
	ASSERT_EQ(vec_last(vec), 'b');
	ASSERT_EQ(vec_at(vec, 0), 'a');
	ASSERT_EQ(vec_at(vec, 1), 'b');

	vec_inc(vec);
	vec_last(vec) = 'c';
	vec_inc(vec);
	vec_last(vec) = 'd';
	ASSERT_EQ(vec_len(vec), 4u);
	ASSERT_EQ(vec_capacity(vec), 4u);

	vec_inc(vec);
	vec_last(vec) = 'e';
	ASSERT_EQ(vec_len(vec), 5u);
	ASSERT_EQ(vec_capacity(vec), 8u);
	ASSERT_EQ(vec_at(vec, 0), 'a');
	ASSERT_EQ(vec_at(vec, 1), 'b');
	ASSERT_EQ(vec_at(vec, 2), 'c');
	ASSERT_EQ(vec_at(vec, 3), 'd');
	ASSERT_EQ(vec_at(vec, 4), 'e');

	vec_at(vec, 1) = 'f';
	ASSERT_EQ(vec_len(vec), 5u);
	ASSERT_EQ(vec_capacity(vec), 8u);
	ASSERT_EQ(vec_at(vec, 0), 'a');
	ASSERT_EQ(vec_at(vec, 1), 'f');
	ASSERT_EQ(vec_at(vec, 2), 'c');
	ASSERT_EQ(vec_at(vec, 3), 'd');
	ASSERT_EQ(vec_at(vec, 4), 'e');

	vec_last(vec) = 'g';
	ASSERT_EQ(vec_len(vec), 5u);
	ASSERT_EQ(vec_capacity(vec), 8u);
	ASSERT_EQ(vec_at(vec, 0), 'a');
	ASSERT_EQ(vec_at(vec, 1), 'f');
	ASSERT_EQ(vec_at(vec, 2), 'c');
	ASSERT_EQ(vec_at(vec, 3), 'd');
	ASSERT_EQ(vec_at(vec, 4), 'g');

	vec_free(vec);
}


// Tests inserting elements in a vector.
TEST(Vector, Insert) {
	Vec(char) vec;
	vec_new(vec, char, 4);

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'b';
	vec_inc(vec);
	vec_last(vec) = 'c';
	ASSERT_EQ(vec_len(vec), 3u);
	ASSERT_EQ(vec_capacity(vec), 4u);

	vec_insert(vec, 0, 'd');
	ASSERT_EQ(vec_len(vec), 4u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'd');
	ASSERT_EQ(vec_at(vec, 1), 'a');
	ASSERT_EQ(vec_at(vec, 2), 'b');
	ASSERT_EQ(vec_at(vec, 3), 'c');

	vec_insert(vec, 2, 'e');
	ASSERT_EQ(vec_len(vec), 5u);
	ASSERT_EQ(vec_capacity(vec), 8u);
	ASSERT_EQ(vec_at(vec, 0), 'd');
	ASSERT_EQ(vec_at(vec, 1), 'a');
	ASSERT_EQ(vec_at(vec, 2), 'e');
	ASSERT_EQ(vec_at(vec, 3), 'b');
	ASSERT_EQ(vec_at(vec, 4), 'c');

	vec_insert(vec, vec_len(vec) - 1, 'f');
	ASSERT_EQ(vec_len(vec), 6u);
	ASSERT_EQ(vec_capacity(vec), 8u);
	ASSERT_EQ(vec_at(vec, 0), 'd');
	ASSERT_EQ(vec_at(vec, 1), 'a');
	ASSERT_EQ(vec_at(vec, 2), 'e');
	ASSERT_EQ(vec_at(vec, 3), 'b');
	ASSERT_EQ(vec_at(vec, 4), 'c');
	ASSERT_EQ(vec_at(vec, 5), 'f');

	vec_free(vec);
}


// Tests removing elements from a vector.
TEST(Vector, Remove) {
	Vec(char) vec;
	vec_new(vec, char, 4);

	vec_inc(vec);
	vec_last(vec) = 'a';
	ASSERT_EQ(vec_len(vec), 1u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'a');

	vec_remove(vec, 0);
	ASSERT_EQ(vec_len(vec), 0u);
	ASSERT_EQ(vec_capacity(vec), 4u);

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'b';
	ASSERT_EQ(vec_len(vec), 2u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'a');
	ASSERT_EQ(vec_at(vec, 1), 'b');

	vec_remove(vec, 0);
	ASSERT_EQ(vec_len(vec), 1u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'b');

	vec_inc(vec);
	vec_last(vec) = 'a';
	ASSERT_EQ(vec_len(vec), 2u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'b');
	ASSERT_EQ(vec_at(vec, 1), 'a');

	vec_remove(vec, vec_len(vec) - 1);
	ASSERT_EQ(vec_len(vec), 1u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'b');

	vec_inc(vec);
	vec_last(vec) = 'a';
	vec_inc(vec);
	vec_last(vec) = 'c';
	ASSERT_EQ(vec_len(vec), 3u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'b');
	ASSERT_EQ(vec_at(vec, 1), 'a');
	ASSERT_EQ(vec_at(vec, 2), 'c');

	vec_remove(vec, 1);
	ASSERT_EQ(vec_len(vec), 2u);
	ASSERT_EQ(vec_capacity(vec), 4u);
	ASSERT_EQ(vec_at(vec, 0), 'b');
	ASSERT_EQ(vec_at(vec, 1), 'c');

	vec_free(vec);
}
