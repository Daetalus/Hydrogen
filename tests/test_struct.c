
//
//  Struct Tests
//

#include "test.h"


#define ASSERT_FIELD(struct_index, field_index, name)                  \
	EQ_STRN(vm->structs[struct_index].fields[field_index].start, name, \
		vm->structs[struct_index].fields[field_index].length);


TEST(definition) {
	COMPILER("struct Test\nstruct Test2 {\nfield1\n}\n"
		"struct Test3 {\nfield1, field2, field3\n}");

	ASSERT_RET();

	EQ(vm->structs_count, 3);

	EQ_STRN(vm->structs[0].name, "Test", vm->structs[0].length);
	EQ(vm->structs[0].fields_count, 0);

	EQ_STRN(vm->structs[1].name, "Test2", vm->structs[1].length);
	EQ(vm->structs[1].fields_count, 1);
	ASSERT_FIELD(1, 0, "field1");

	EQ_STRN(vm->structs[2].name, "Test3", vm->structs[2].length);
	EQ(vm->structs[2].fields_count, 3);
	ASSERT_FIELD(2, 0, "field1");
	ASSERT_FIELD(2, 1, "field2");
	ASSERT_FIELD(2, 2, "field3");
}


TEST(instantiation) {
	COMPILER("struct Test {\nfield1\n}\nlet a = new Test()\nlet b = new Test()");

	ASSERT_INSTRUCTION(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_NEW, 1, 0, 0);
	ASSERT_RET();
}


TEST(field_access) {
	COMPILER("struct Test {\nfield1\n}\nlet a = new Test()\nlet b = a.field1");

	ASSERT_INSTRUCTION(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 0, 0);
	ASSERT_RET();
}


TEST(field_set) {
	COMPILER("struct Test {\nfield1\n}\nlet a = new Test()\na.field1 = 3\n"
		"a.field1.test.hello = 10");

	ASSERT_INSTRUCTION(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_SET_I, 0, 0, 3);
	ASSERT_INSTRUCTION(MOV_LL, 1, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 1, 1);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 1, 2);
	ASSERT_INSTRUCTION(STRUCT_SET_I, 1, 3, 10);
	ASSERT_RET();
}


TEST(method_definition) {
	COMPILER("struct Test {\nfield1\n}\nfn (Test) test() {\nlet a = 3\n}\n");

	SELECT_FN(0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 1, 3, 0);
	ASSERT_RET();
}


TEST(method_access) {
	COMPILER("struct Test {\nfield1\n}\nfn (Test) test() {\nlet a = 3\n}\n"
		"let a = new Test()\nlet b = a.test");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 0, 0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 1, 3, 0);
	ASSERT_RET();
}


TEST(self_access) {
	COMPILER("struct Test {\nfield1\n}\nfn (Test) test() {\n"
		"let a = self.field1\n}");

	SELECT_FN(0);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 0, 0);
	ASSERT_RET();
}


TEST(method_call) {
	COMPILER("struct Test\nfn (Test) test() {\nlet a = 3\n}\n"
		"let a = new Test()\nlet b = a.test()");

	SELECT_FN(0);
	ASSERT_INSTRUCTION(STRUCT_NEW, 0, 0, 0);
	ASSERT_INSTRUCTION(STRUCT_FIELD, 1, 0, 0);
	ASSERT_INSTRUCTION(MOV_LL, 2, 0, 0);
	ASSERT_CALL(CALL_L, 1, 1, 2, 1);
	ASSERT_RET();

	SELECT_FN(1);
	ASSERT_INSTRUCTION(MOV_LI, 1, 3, 0);
	ASSERT_RET();
}


TEST(constructor_definition) {

}


TEST(constructor_call) {

}


MAIN() {
	RUN(definition);
	RUN(instantiation);
	RUN(field_access);
	RUN(field_set);
	RUN(method_definition);
	RUN(method_access);
	RUN(self_access);
	RUN(method_call);
	// RUN(constructor_definition);
	// RUN(constructor_call);
}
