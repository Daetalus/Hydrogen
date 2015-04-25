
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
	src/hydrogen.c
	src/bytecode.c
	src/compiler.c
	src/error.c
	src/debug.c
	src/expression.c
	src/lexer.c
	src/parser.c
	src/value.c
	src/vm.c

	src/lib/operator.c
	src/lib/str.c
	src/lib/io.c
)



# --
# Tests
# --

enable_testing()

macro(unit_test name)
	add_executable(test_${name} tests/units/test_${name}.c)
	target_link_libraries(test_${name} vm)
	add_test(test_${name} test_${name})
endmacro()

macro(test name)
	add_executable(test_${name} tests/${name}.c)
	target_link_libraries(test_${name} vm)
	add_test(test_${name} test_${name})
endmacro()

unit_test(value)
unit_test(parser)
unit_test(lexer)
unit_test(expression)
unit_test(assignment)
unit_test(if)
unit_test(while)
unit_test(function_calls)
unit_test(function_definitions)
unit_test(anonymous_functions)
unit_test(closures)
test(runtime)



# --
# Benchmarks
# --

add_executable(interpreter benchmarks/interpreter.c)
target_link_libraries(interpreter vm)
