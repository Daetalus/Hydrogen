
//
//  Struct Tests
//

#include <mock_parser.h>
#include <test.h>
#include <struct.h>
#include <vm.h>


// Asserts the struct at index `struct_index` has the name `struct_name`, and
// has `fields_count` number of fields.
#define ASSERT_STRUCT(struct_index, struct_name, fields_count) {   \
	StructDefinition *def = &vec_at(state->structs, struct_index); \
	eq_int(def->length, strlen(struct_name));                      \
	eq_strn(def->name, struct_name, def->length);                  \
	eq_int(vec_len(def->fields), fields_count);                    \
}


// Asserts the field at index `field_index` of the struct at `struct_index` is
// named `name`.
#define ASSERT_FIELD(struct_index, field_index, field_name) {      \
	StructDefinition *def = &vec_at(state->structs, struct_index); \
	Identifier *field = &vec_at(def->fields, field_index);         \
	eq_int(field->length, strlen(field_name));                     \
	eq_strn(field->name, field_name, field->length);               \
}


// Tests defining a struct with zero, one, and more than one field.
void test_definition(void) {
	MockParser p = mock_parser(
		"struct Test\n"
		"struct Test2 {\n"
		"	field1\n"
		"}\n"
		"struct Test3 {\n"
		"	field1, field2, field3\n"
		"}\n"
	);

	// No actual instructions
	ins(&p, RET0, 0, 0, 0);

	// Number of defined structs
	HyState *state = p.state;
	eq_int(vec_len(state->structs), 3);

	// Test
	ASSERT_STRUCT(0, "Test", 0);

	// Test2
	ASSERT_STRUCT(1, "Test2", 1);
	ASSERT_FIELD(1, 0, "field1");

	// Test3
	ASSERT_STRUCT(2, "Test3", 3);
	ASSERT_FIELD(2, 0, "field1");
	ASSERT_FIELD(2, 1, "field2");
	ASSERT_FIELD(2, 2, "field3");

	mock_parser_free(&p);
}


// Tests instantiating a struct and storing it into a local.
void test_instantiation(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"let b = new Test()\n"
	);

	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests accessing a field on a struct.
void test_get_field(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.field1\n"
	);

	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, MOV_TL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests setting a field on a struct.
void test_set_field(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"a.field1 = 3\n"
		"a.field1.test.hello = 10\n"
	);

	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_SET_I, 0, 3, 0);

	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 1);
	ins(&p, STRUCT_SET_I, 2, 10, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests defining a method on a struct.
void test_method_definition(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests getting a method on a struct.
void test_get_method(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.test\n"
	);

	switch_fn(&p, 0);
	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, MOV_TL, 1, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests the use of `self` within a struct's method.
void test_self(void) {
	MockParser p = mock_parser(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = self.field1\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_SELF, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests calling a method on a struct.
void test_method_call(void) {
	MockParser p = mock_parser(
		"struct Test\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.test()\n"
		"a.test()\n"
	);

	switch_fn(&p, 0);
	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_TL, 0, 0, 0);

	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, CALL, 0, 0, 0);
	ins(&p, MOV_TL, 1, 0, 0);

	ins(&p, MOV_LT, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, CALL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests calling a method on a struct stored as an upvalue.
void test_upvalue_method_call(void) {
	MockParser p = mock_parser(
		"struct Test\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
		"{\n"
		"let a = new Test()\n"
		"fn test() {\n"
		"	let c = a.test()\n"
		"}\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_LF, 0, 2, 0);
	ins(&p, UPVALUE_CLOSE, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, MOV_LI, 0, 3, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 2);
	ins(&p, MOV_LU, 0, 0, 0);
	ins(&p, STRUCT_FIELD, 0, 0, 0);
	ins(&p, MOV_LU, 1, 0, 0);
	ins(&p, CALL, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests defining a custom constructor on a struct.
void test_custom_constructor(void) {
	MockParser p = mock_parser(
		"struct Test\n"
		"fn (Test) new(arg) {\n"
		"	self.a = arg\n"
		"}\n"
	);

	switch_fn(&p, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, STRUCT_SET_L, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


// Tests calling a custom constructor upon instantiation.
void test_call_custom_constructor(void) {
	MockParser p = mock_parser(
		"struct Test\n"
		"fn (Test) new(arg) {\n"
		"	self.a = arg\n"
		"}\n"
		"let a = new Test(3)\n"
	);

	switch_fn(&p, 0);
	ins(&p, STRUCT_NEW, 0, 0, 0);
	ins(&p, MOV_LF, 1, 1, 0);
	ins(&p, MOV_LL, 2, 0, 0);
	ins(&p, MOV_LI, 3, 3, 0);
	ins(&p, CALL, 1, 2, 2);
	ins(&p, MOV_TL, 0, 0, 0);
	ins(&p, RET0, 0, 0, 0);

	switch_fn(&p, 1);
	ins(&p, STRUCT_SET_L, 0, 1, 0);
	ins(&p, RET0, 0, 0, 0);

	mock_parser_free(&p);
}


int main(int argc, char *argv[]) {
	test_pass("Definition", test_definition);
	test_pass("Instantiation", test_instantiation);
	test_pass("Get field", test_get_field);
	test_pass("Set field", test_set_field);
	test_pass("Method definition", test_method_definition);
	test_pass("Get method", test_get_method);
	test_pass("Self", test_self);
	test_pass("Method call", test_method_call);
	test_pass("Method call on upvalue", test_upvalue_method_call);
	test_pass("Custom constructor", test_custom_constructor);
	test_pass("Call custom constructor", test_call_custom_constructor);
	return test_run(argc, argv);
}
