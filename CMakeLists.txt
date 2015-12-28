
#
#  Hydrogen Build File
#

cmake_minimum_required(VERSION 2.8)
project(Hydrogen)

# Set the default build type to `release`.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Debug and release flags
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -O3")

# Library
include_directories(include)
file(GLOB VM_SOURCES src/vm/*.c)
add_library(hydrogen STATIC ${VM_SOURCES})

# Command line executable
file(GLOB CLI_SOURCES src/cli/*.c)
add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli hydrogen)

# Enable testing using Google's testing framework
add_subdirectory(gtest/googletest)
enable_testing()

# Macro for adding a test program
macro(test name)
	# Add required include directories
	include_directories(
		# Hydrogen
		include src/vm
		# Google test framework
		${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}
	)

	# Create the testing executable
	add_executable(test_${name} tests/test_${name}.cpp)

	# Link against hydrogen and google test libraries
	target_link_libraries(test_${name} hydrogen gtest gtest_main)

	# Tell CMake about the test
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
