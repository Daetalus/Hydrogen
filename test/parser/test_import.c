
//
//  Import Tests
//

extern "C" {
#include <hydrogen.h>
#include <import.h>
#include <vec.h>
#include <pkg.h>
}

#include <gtest/gtest.h>


// Tests validating import paths
TEST(ImportPath, Validation) {
	ASSERT_EQ(import_is_valid("hello"), true);
	ASSERT_EQ(import_is_valid("hello/test"), true);
	ASSERT_EQ(import_is_valid("this/is/a/test"), true);
	ASSERT_EQ(import_is_valid("/absolute"), true);
	ASSERT_EQ(import_is_valid("/an/absolute/path"), true);
	ASSERT_EQ(import_is_valid("../relative"), true);
	ASSERT_EQ(import_is_valid("../relative/path/with/components"), true);
	ASSERT_EQ(import_is_valid("some/more/../relative"), true);
	ASSERT_EQ(import_is_valid("some/../more/../../relative"), true);

	ASSERT_EQ(import_is_valid(""), false);
	ASSERT_EQ(import_is_valid("/this/is/a/test/"), false);
	ASSERT_EQ(import_is_valid("/thi.s/is/a/tes.t"), false);
	ASSERT_EQ(import_is_valid("/this/is/testin/./"), false);
	ASSERT_EQ(import_is_valid("/this/../.is/testin"), false);
	ASSERT_EQ(import_is_valid("./more"), false);
	ASSERT_EQ(import_is_valid("empty//path/component"), false);
	ASSERT_EQ(import_is_valid("empty////path//components"), false);
	ASSERT_EQ(import_is_valid("invalid/@#FJ($!@#F\n\t/characters"), false);
}


// Asserts a package path is equal to its expected value
#define ASSERT_PACKAGE_PATH(parent, package, expected) {       \
	char *package_heap = (char *) malloc(strlen(package) + 1); \
	strcpy(package_heap, package);                             \
	char *result = import_pkg_path(parent, package_heap);      \
	ASSERT_STREQ(result, expected);                            \
	free(result);                                              \
	if (result != package_heap) {                              \
		free(package_heap);                                    \
	}                                                          \
}


// Tests resolving import paths to their actual locations on the filesystem
// using the parent package
TEST(ImportPath, PathResolution) {
	ASSERT_PACKAGE_PATH(NULL, "hello", "hello");
	ASSERT_PACKAGE_PATH(NULL, "test/ing", "test/ing");
	ASSERT_PACKAGE_PATH(NULL, "/abs/path", "/abs/path");

	ASSERT_PACKAGE_PATH("testing", "hello", "hello");
	ASSERT_PACKAGE_PATH("testing", "test/ing", "test/ing");
	ASSERT_PACKAGE_PATH("testing", "/abs/path", "/abs/path");

	ASSERT_PACKAGE_PATH("test/testing", "hello", "test/hello");
	ASSERT_PACKAGE_PATH("test/testing", "test/ing", "test/test/ing");
	ASSERT_PACKAGE_PATH("test/testing", "/abs/path", "/abs/path");

	ASSERT_PACKAGE_PATH("/test", "hello", "/hello");
	ASSERT_PACKAGE_PATH("/test", "test/ing", "/test/ing");
	ASSERT_PACKAGE_PATH("/test", "/abs/path", "/abs/path");
}


// Asserts a package name matches its expected value
#define ASSERT_PACKAGE_NAME(path, expected) { \
	char *name = hy_pkg_name(path);           \
	ASSERT_STREQ(name, expected);             \
	free(name);                               \
}


// Tests extracting the name of a package from its path
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
