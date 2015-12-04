
#
#  Hydrogen Build File
#

cmake_minimum_required(VERSION 2.8)
project(Hydrogen)

# Set the default build type to `release`.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -O3")



#
# Library
#

include_directories(include)
file(GLOB VM_SOURCES src/vm/*.c)
add_library(hydrogen STATIC ${VM_SOURCES})



#
#  Command Line Executable
#

file(GLOB CLI_SOURCES src/cli/*.c)
add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli hydrogen)



#
#  Tests
#

enable_testing()

macro(test name)
	include_directories(src/vm)
	add_executable(test_${name} tests/test_${name}.c)
	target_link_libraries(test_${name} hydrogen)
	add_test(test_${name} test_${name})
endmacro()

test(lexer)
test(bytecode)
test(expr)
