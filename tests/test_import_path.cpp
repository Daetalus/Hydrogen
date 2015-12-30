
//
//  Import Path Tests
//

#include "test.h"

// Define the function's we're testing
extern "C" {
	int last_path_component(char *path);
	bool import_path_is_valid(char *path);
	char * import_package_path(Package *importer, char *path);
	char * import_package_name(char *path);
}


// Tests finding the last path component of paths.
TEST(ImportPath, LastPathComponent) {
	ASSERT_EQ(last_path_component("hello/test"), 5);
	ASSERT_EQ(last_path_component("hello"), -1);
	ASSERT_EQ(last_path_component("this/is/a/test/with/slashes"), 19);
	ASSERT_EQ(last_path_component("/absolute"), 0);
	ASSERT_EQ(last_path_component(""), -1);
}


// Tests validating import paths.
TEST(ImportPath, Validation) {
	ASSERT_EQ(import_path_is_valid("hello"), true);
	ASSERT_EQ(import_path_is_valid("hello/test"), true);
	ASSERT_EQ(import_path_is_valid("this/is/a/test"), true);
	ASSERT_EQ(import_path_is_valid("/absolute"), true);
	ASSERT_EQ(import_path_is_valid("/an/absolute/path"), true);
	ASSERT_EQ(import_path_is_valid("../relative"), true);
	ASSERT_EQ(import_path_is_valid("../relative/path/with/components"), true);
	ASSERT_EQ(import_path_is_valid("some/more/../relative"), true);
	ASSERT_EQ(import_path_is_valid("some/../more/../../relative"), true);

	ASSERT_EQ(import_path_is_valid(""), false);
	ASSERT_EQ(import_path_is_valid("/this/is/a/test/"), false);
	ASSERT_EQ(import_path_is_valid("/thi.s/is/a/tes.t"), false);
	ASSERT_EQ(import_path_is_valid("/this/is/testin/./"), false);
	ASSERT_EQ(import_path_is_valid("/this/../.is/testin"), false);
	ASSERT_EQ(import_path_is_valid("./more"), false);
	ASSERT_EQ(import_path_is_valid("empty//path/component"), false);
	ASSERT_EQ(import_path_is_valid("empty////path//components"), false);
	ASSERT_EQ(import_path_is_valid("invalid/@#FJ($!@#F\n\t/characters"), false);
}


// Tests resolving import paths to their actual locations on the filesystem
// using the parent package.
TEST(ImportPath, PathResolution) {
	Package importer;

	importer.file = NULL;
	ASSERT_STREQ(import_package_path(&importer, "hello"), "hello");
	ASSERT_STREQ(import_package_path(&importer, "test/ing"), "test/ing");
	ASSERT_STREQ(import_package_path(&importer, "/abs/path"), "/abs/path");

	importer.file = "testing";
	ASSERT_STREQ(import_package_path(&importer, "hello"), "hello");
	ASSERT_STREQ(import_package_path(&importer, "test/ing"), "test/ing");
	ASSERT_STREQ(import_package_path(&importer, "/abs/path"), "/abs/path");

	importer.file = "test/testing";
	ASSERT_STREQ(import_package_path(&importer, "hello"), "test/hello");
	ASSERT_STREQ(import_package_path(&importer, "test/ing"), "test/test/ing");
	ASSERT_STREQ(import_package_path(&importer, "/abs/path"), "/abs/path");

	importer.file = "/test";
	ASSERT_STREQ(import_package_path(&importer, "hello"), "/hello");
	ASSERT_STREQ(import_package_path(&importer, "test/ing"), "/test/ing");
	ASSERT_STREQ(import_package_path(&importer, "/abs/path"), "/abs/path");
}


// Tests extracting the name of a package from its path.
TEST(ImportPath, PackageName) {
	ASSERT_STREQ(import_package_name("test"), "test");
	ASSERT_STREQ(import_package_name("a"), "a");
	ASSERT_STREQ(import_package_name("test/testing"), "testing");
	ASSERT_STREQ(import_package_name("../test/../testing"), "testing");
	ASSERT_STREQ(import_package_name("/absolute/path"), "path");
}
