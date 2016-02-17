
#
#  Hydrogen Build File
#

cmake_minimum_required(VERSION 2.8)
project(Hydrogen)

# Set the default build type to `release`.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Set debug and release flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

# Main include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
file(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/core/*)
add_library(hydrogen STATIC ${CORE_SOURCES})

# Standard library
file(GLOB_RECURSE LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/lib/*)
add_library(hylib STATIC ${LIB_SOURCES})

# Command line interface
file(GLOB_RECURSE CLI_SOURCES ${CMAKE_SOURCE_DIR}/src/cli/*)
add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli hydrogen hylib)

# Enable testing using Google's testing framework
add_subdirectory(${CMAKE_SOURCE_DIR}/test/gtest/googletest)
enable_testing()

# Add include directories for tests
include_directories(
	${CMAKE_SOURCE_DIR}/src/core
	${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}
)


# Macro for adding a parser test program.
macro(parser_test name)
	# Create the executable
	add_executable(
		test_${name}
		${CMAKE_SOURCE_DIR}/test/parser/test_${name}.cpp
	)

	# Link against Hydrogen and Google Test
	target_link_libraries(test_${name} hydrogen hylib gtest gtest_main)

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


# Require Python for runtime tests
find_package(PythonInterp REQUIRED)

# Macro for adding a set of runtime tests.
macro(runtime_test section)
	# Add the test case
	add_test(
		NAME test_runtime_${section}
		COMMAND ${PYTHON_EXECUTABLE}
		${CMAKE_SOURCE_DIR}/test/runtime/test_runtime.py
		${CMAKE_SOURCE_DIR}/test/runtime/${section}
		${CMAKE_BINARY_DIR}/cli
	)
endmacro()

# Add runtime test sets
runtime_test(pass)
runtime_test(fail)
