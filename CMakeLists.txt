
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
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings")
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

# Macro for adding a parser test program
macro(parser_test name)
	# Create the testing executable
	add_executable(test_${name} ${CMAKE_SOURCE_DIR}/test/parser/test_${name}.cpp)

	# Link against Hydrogen and Google Test
	target_link_libraries(test_${name}
		hydrogen hystdlib
		gtest gtest_main
	)

	# Register the test
	add_test(test_${name} test_${name})
endmacro()

# Add parser tests
parser_test(lexer)
parser_test(ins)
parser_test(jmp)
parser_test(expr)
parser_test(if)
parser_test(loop)
parser_test(while)
parser_test(fn)
parser_test(import)
# parser_test(upvalue)
# parser_test(struct)

# Add runtime tests
find_package(PythonInterp REQUIRED)
add_test(
	NAME test_runtime
	COMMAND ${PYTHON_EXECUTABLE}
	${CMAKE_SOURCE_DIR}/test/runtime/test_runtime.py
	${CMAKE_SOURCE_DIR}/test/runtime
	${CMAKE_BINARY_DIR}/cli
)
