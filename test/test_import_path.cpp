
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


// Asserts a package path is equal to its expected value.
#define ASSERT_PACKAGE_PATH(package, expected) {                 \
	char *package_heap = (char *) malloc(strlen(package) + 1);   \
	strcpy(package_heap, package);                               \
	char *result = import_package_path(&importer, package_heap); \
	ASSERT_STREQ(result, expected);                              \
	free(result);                                                \
	if (result != package_heap) {                                \
		free(package_heap);                                      \
	}                                                            \
}


// Tests resolving import paths to their actual locations on the filesystem
// using the parent package.
TEST(ImportPath, PathResolution) {
	Package importer;

	importer.file = NULL;
	ASSERT_PACKAGE_PATH("hello", "hello");
	ASSERT_PACKAGE_PATH("test/ing", "test/ing");
	ASSERT_PACKAGE_PATH("/abs/path", "/abs/path");

	importer.file = "testing";
	ASSERT_PACKAGE_PATH("hello", "hello");
	ASSERT_PACKAGE_PATH("test/ing", "test/ing");
	ASSERT_PACKAGE_PATH("/abs/path", "/abs/path");

	importer.file = "test/testing";
	ASSERT_PACKAGE_PATH("hello", "test/hello");
	ASSERT_PACKAGE_PATH("test/ing", "test/test/ing");
	ASSERT_PACKAGE_PATH("/abs/path", "/abs/path");

	importer.file = "/test";
	ASSERT_PACKAGE_PATH("hello", "/hello");
	ASSERT_PACKAGE_PATH("test/ing", "/test/ing");
	ASSERT_PACKAGE_PATH("/abs/path", "/abs/path");
}


// Asserts a package name matches its expected value.
#define ASSERT_PACKAGE_NAME(path, expected) { \
	char *name = import_package_name(path);   \
	ASSERT_STREQ(name, expected);             \
	free(name);                               \
}


// Tests extracting the name of a package from its path.
TEST(ImportPath, PackageName) {
	ASSERT_PACKAGE_NAME("test", "test");
	ASSERT_PACKAGE_NAME("a", "a");
	ASSERT_PACKAGE_NAME("test/testing", "testing");
	ASSERT_PACKAGE_NAME("../test/../testing", "testing");
	ASSERT_PACKAGE_NAME("/absolute/path", "path");
	ASSERT_PACKAGE_NAME("test.hy", "test");

	ASSERT_PACKAGE_NAME("thing/test.hy", "test");
	ASSERT_PACKAGE_NAME("../test.hy", "test");
	ASSERT_PACKAGE_NAME("another/../test.hy", "test");
	ASSERT_PACKAGE_NAME("../another/../test.hy", "test");
}
