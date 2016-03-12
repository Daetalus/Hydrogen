
//
//  Struct Tests
//

#include "test.h"


// Asserts the field at index `field_index` of the struct at `struct_index` is
// named `name`.
#define ASSERT_FIELD(struct_index, field_index, field_name) {      \
	StructDefinition *def = &vec_at(state->structs, struct_index); \
	Identifier *field = &vec_at(def->fields, field_index);         \
	ASSERT_STREQN(field->name, field_name, field->length);         \
}


// Tests defining a struct with zero, one, and more than one field.
TEST(Struct, Definition) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"struct Test2 {\n"
		"	field1, field2, field3\n"
		"}\n"
	);

	// No actual instructions
	INS(RET0, 0, 0, 0);

	ASSERT_EQ(vec_len(state->structs), 2u);

	ASSERT_STREQN(vec_at(state->structs, 0).name, "Test",
		vec_at(state->structs, 0).length);
	ASSERT_EQ(vec_len(vec_at(state->structs, 0).fields), 1u);
	ASSERT_FIELD(0, 0, "field1");

	ASSERT_STREQN(vec_at(state->structs, 1).name, "Test2",
		vec_at(state->structs, 1).length);
	ASSERT_EQ(vec_len(vec_at(state->structs, 1).fields), 3u);
	ASSERT_FIELD(1, 0, "field1");
	ASSERT_FIELD(1, 1, "field2");
	ASSERT_FIELD(1, 2, "field3");

	FREE();
}


// Tests instantiating a struct and storing it into a local.
TEST(Struct, Instantiation) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"let b = new Test()\n"
	);

	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 0, 0, 0);
	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests accessing a field on a struct.
TEST(Struct, GetField) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.field1\n"
	);

	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 0, 0, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(MOV_TL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests setting a field on a struct.
TEST(Struct, SetField) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"let a = new Test()\n"
		"a.field1 = 3\n"
		"a.field1.test.hello = 10\n"
	);

	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 0, 0, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_SET_I, 0, 3, 0);

	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 1);
	INS(STRUCT_SET_I, 2, 10, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests defining a method on a struct.
TEST(Struct, MethodDefinition) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
	);

	FN(0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 1, 3, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests getting a method on a struct.
TEST(Struct, GetMethod) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.test\n"
	);

	FN(0);
	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 0, 0, 0);
	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(MOV_TL, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 1, 3, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests the use of `self` within a struct's method.
TEST(Struct, Self) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = self.field1\n"
		"}\n"
	);

	FN(0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(STRUCT_FIELD, 1, 0, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests calling a method on a struct.
TEST(Struct, CallMethod) {
	COMPILER(
		"struct Test\n"
		"fn (Test) test() {\n"
		"	let a = 3\n"
		"}\n"
		"let a = new Test()\n"
		"let b = a.test()\n"
		"a.test()\n"
	);

	FN(0);
	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_TL, 0, 0, 0);

	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(MOV_LT, 1, 0, 0);
	INS(CALL, 0, 1, 0);
	INS(MOV_TL, 1, 0, 0);

	INS(MOV_LT, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(MOV_LT, 1, 0, 0);
	INS(CALL, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 1, 3, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests calling a method on a struct stored as an upvalue.
TEST(Struct, UpvalueCallMethod) {
	COMPILER(
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

	FN(0);
	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_LF, 1, 2, 0);
	INS(UPVALUE_CLOSE, 0, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(MOV_LI, 1, 3, 0);
	INS(RET0, 0, 0, 0);

	FN(2);
	INS(MOV_LU, 0, 0, 0);
	INS(STRUCT_FIELD, 0, 0, 0);
	INS(MOV_LU, 1, 0, 0);
	INS(CALL, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests defining a custom constructor on a struct.
TEST(Struct, CustomConstructor) {
	COMPILER(
		"struct Test\n"
		"fn (Test) new(arg) {\n"
		"	self.a = arg\n"
		"}\n"
	);

	FN(0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(STRUCT_SET_L, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}


// Tests calling a custom constructor upon instantiation.
TEST(Struct, CallCustomConstructor) {
	COMPILER(
		"struct Test\n"
		"fn (Test) new(arg) {\n"
		"	self.a = arg\n"
		"}\n"
		"let a = new Test(3)\n"
	);

	FN(0);
	INS(STRUCT_NEW, 0, 0, 0);
	INS(MOV_LF, 1, 1, 0);
	INS(MOV_LL, 2, 0, 0);
	INS(MOV_LI, 3, 3, 0);
	INS(CALL, 1, 2, 2);
	INS(MOV_TL, 0, 0, 0);
	INS(RET0, 0, 0, 0);

	FN(1);
	INS(STRUCT_SET_L, 0, 1, 0);
	INS(RET0, 0, 0, 0);

	FREE();
}
