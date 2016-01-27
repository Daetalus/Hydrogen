
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
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-write-strings")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

# Hydrogen include directory
include_directories(include)

# Library
file(GLOB_RECURSE VM_SOURCES src/vm/*)
add_library(hydrogen STATIC ${VM_SOURCES})

# Standard library
file(GLOB_RECURSE STDLIB_SOURCES src/std/*)
add_library(hystdlib STATIC ${STDLIB_SOURCES})

# Command line interface
file(GLOB_RECURSE CLI_SOURCES src/cli/*)
add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli hydrogen hystdlib)

# Enable testing using Google's testing framework
add_subdirectory(test/gtest/googletest)
enable_testing()

# Add include directories for tests
include_directories(
	src/vm
	${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}
)

# Macro for adding a test program
macro(test name)
	# Create the testing executable
	add_executable(test_${name} test/test_${name}.cpp)

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
test(ins)
test(jmp)
test(expr)
test(if)
# test(loop)
# test(while)
# test(fn)
# test(upvalue)
# test(struct)
# test(import_path)
# test(runtime)
