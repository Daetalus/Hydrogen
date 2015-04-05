
# --
# Hydrogen Build File
# --

cmake_minimum_required(VERSION 2.6)
project(Hydrogen)

set(HYDROGEN_VERSION_MAJOR 0)
set(HYDROGEN_VERSION_MINOR 1)



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
	add_executable(test_${name} tests/units/test_${name}.c)
	target_link_libraries(test_${name} vm)
	add_test(test_${name} test_${name})
endmacro()

test(parser)
test(lexer)
test(expression)



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
