
//
//  Struct Tests
//

#include "test.h"


// Asserts the field at index `field_index` of the struct at `struct_index` is
// named `name`.
#define ASSERT_FIELD(struct_index, field_index, name)        \
	ASSERT_STREQN(                                           \
		vm->structs[struct_index].fields[field_index].start, \
		name,                                                \
		vm->structs[struct_index].fields[field_index].length \
	);


// Tests defining a struct with zero, one, and more than one field.
TEST(Struct, Definition) {
	COMPILER(
		"struct Test\n"
		"struct Test2 {\n"
		"	field1\n"
		"}\n"
		"struct Test3 {\n"
		"	field1, field2, field3\n"
		"}\n"
	);

	// No actual instructions
	ASSERT_RET();

	ASSERT_EQ(vm->structs_count, 3u);

	ASSERT_STREQN(vm->structs[0].name, "Test", vm->structs[0].length);
	ASSERT_EQ(vm->structs[0].fields_count, 0u);

	ASSERT_STREQN(vm->structs[1].name, "Test2", vm->structs[1].length);
	ASSERT_EQ(vm->structs[1].fields_count, 1u);
	ASSERT_FIELD(1, 0, "field1");

	ASSERT_STREQN(vm->structs[2].name, "Test3", vm->structs[2].length);
	ASSERT_EQ(vm->structs[2].fields_count, 3u);
	ASSERT_FIELD(2, 0, "field1");
	ASSERT_FIELD(2, 1, "field2");
	ASSERT_FIELD(2, 2, "field3");

	COMPILER_FREE();
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

	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 1, 0, 0);
	ASSERT_RET();

	COMPILER_FREE();
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

	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 1, 0, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests setting a field on a struct.
TEST(Struct, SetField) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"{\n"
		"let a = new Test()\n"
		"a.field1 = 3\n"
		"a.field1.test.hello = 10\n"
		"}\n"
	);

	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_LI, 1, 3, 0);
	ASSERT_INSTR(STRUCT_SET, 0, 0, 1);
	ASSERT_INSTR(STRUCT_FIELD, 1, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 1, 1, 1);
	ASSERT_INSTR(MOV_LI, 2, 10, 0);
	ASSERT_INSTR(STRUCT_SET, 1, 2, 2);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LI, 1, 3, 0);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 1, 0, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LI, 1, 3, 0);
	ASSERT_RET();

	COMPILER_FREE();
}


// Tests the use of `self` within a struct's method.
TEST(Struct, UseSelf) {
	COMPILER(
		"struct Test {\n"
		"	field1\n"
		"}\n"
		"fn (Test) test() {\n"
		"	let a = self.field1\n"
		"}\n"
	);

	FN(0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(STRUCT_FIELD, 1, 0, 0);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);

	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 0, 0, 0);
	ASSERT_INSTR(MOV_LT, 1, 0, 0);
	ASSERT_CALL(CALL_L, 0, 1, 1, 0);
	ASSERT_INSTR(MOV_TL, 1, 0, 0);

	ASSERT_INSTR(MOV_LT, 0, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 0, 0, 0);
	ASSERT_INSTR(MOV_LT, 1, 0, 0);
	ASSERT_CALL(CALL_L, 0, 1, 1, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LI, 1, 3, 0);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_LF, 1, 2, 0);
	ASSERT_INSTR(UPVALUE_CLOSE, 0, 0, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(MOV_LI, 1, 3, 0);
	ASSERT_RET();

	FN(2);
	ASSERT_INSTR(MOV_LU, 0, 0, 0);
	ASSERT_INSTR(STRUCT_FIELD, 0, 0, 0);
	ASSERT_INSTR(MOV_LU, 1, 0, 0);
	ASSERT_CALL(CALL_L, 0, 1, 1, 0);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(STRUCT_SET, 0, 0, 1);
	ASSERT_RET();

	COMPILER_FREE();
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
	ASSERT_INSTR(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTR(MOV_LL, 1, 0, 0);
	ASSERT_INSTR(MOV_LI, 2, 3, 0);
	ASSERT_CALL(CALL_F, 1, 1, 2, 1);
	ASSERT_INSTR(MOV_TL, 0, 0, 0);
	ASSERT_RET();

	FN(1);
	ASSERT_INSTR(STRUCT_SET, 0, 0, 1);
	ASSERT_RET();

	COMPILER_FREE();
}
