
#
#  Hydrogen Build File
#

cmake_minimum_required(VERSION 2.8)
project(Hydrogen)

# Set the default build type to `Release`.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Set debug and release flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# Main include directory and common header files
include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/src/common
)

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



#
#  Testing
#

# Enable testing
enable_testing()

# Add include directories for tests
include_directories(
	${CMAKE_SOURCE_DIR}/src/core
	${CMAKE_SOURCE_DIR}/test
	${CMAKE_SOURCE_DIR}/test/mock
)

# Build testing framework
file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/test.c)
add_library(testing STATIC ${TEST_SOURCES})

# Build mock framework
file(GLOB_RECURSE MOCK_SOURCES ${CMAKE_SOURCE_DIR}/test/mock/*)
add_library(mock STATIC ${MOCK_SOURCES})
target_link_libraries(mock testing)

# Macro for adding a test program.
macro(test folder name)
	# Create the executable
	add_executable(
		test_${name}
		${CMAKE_SOURCE_DIR}/test/${folder}/test_${name}.c
	)

	# Link against Hydrogen and Google Test
	target_link_libraries(test_${name} hydrogen hylib testing mock)

	# Register the test
	add_test(test_${name} test_${name})
endmacro()

# Add parser tests
test(parser vec)
test(parser lexer)
test(parser ins)
test(parser jmp)
test(parser expr)
test(parser if)
test(parser loop)
test(parser while)
test(parser fn)
test(parser import)
#test(parser struct)
# test(parser upvalue)


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
