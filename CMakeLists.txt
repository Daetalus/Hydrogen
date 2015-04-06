
# --
# Hydrogen Build File
# --

cmake_minimum_required(VERSION 2.6)
project(Hydrogen)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")



# --
# Library
# --

add_library(
	vm
	src/bytecode.c
	src/compiler.c
	src/debug.c
	src/expression.c
	src/hydrogen.c
	src/lexer.c
	src/operators.c
	src/value.c
	src/vm.c
)



# --
# Tests
# --

enable_testing()

macro(test name)
	add_executable(test_${name} tests/test_${name}.c)
	target_link_libraries(test_${name} vm)
	add_test(test_${name} test_${name})
endmacro()

test(value)
test(parser)
test(lexer)
test(expression)
test(compiler)



# --
# Command Line Interpreter
# --

# add_executable(
	# hydrogen
	# src/main.c
# )
#
# target_link_libraries(hydrogen vm)
#
# install(TARGETS hydrogen DESTINATION bin)
