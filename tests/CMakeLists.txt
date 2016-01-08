
#
#  Tests Build File
#

# Enable testing using Google's testing framework
add_subdirectory(gtest/googletest)
enable_testing()

# Macro for adding a test program
macro(test name)
	# Add include directories
	include_directories(
		../src/vm
		${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}
	)

	# Create the testing executable
	add_executable(test_${name} test_${name}.cpp)

	# Link against Hydrogen, Google Test, and Boost libraries
	target_link_libraries(test_${name}
		hydrogen hystdlib
		gtest gtest_main
	)

	# Register the test
	add_test(test_${name} test_${name})
endmacro()

# Add tests
test(lexer)
test(bytecode)
test(jmp)
test(assign)
test(expr)
test(if)
test(loop)
test(while)
test(fn)
test(upvalue)
test(struct)
test(import_path)
test(runtime)
