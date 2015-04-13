
To Do
=====


## Refactoring

Incomplete

* Objectify compiler code
* Prefix functions with class name
* Add static in front of functions used in single file
* Refactor #defines into C functions
* Add library function interface
* Remove unnecessary defines
* Refactor addition operator to remove duplicate code
* Completely re-write documentation
* Remove need to cache conditional patch locations in multiple else if statements
* Reduce coupling between parts of code. If only needs lexer, only takes lexer as argument
* Remove compiler dependency on vm
* Remove useless comments
* Convert everything to use size_t
* Move print statements in operator.c to lib/io.c

Complete

* Group files into subfolders
* Separate parser out from lexer
* Reorder tokens
* Rename `string_duplicate` to `string_copy`
* Move String into own library files
* Refactor expression to use an expression table
* Refactor `patch_jump` to only accept conditional and forward jump, or to handle backwards jumps
* Refactor `operator_ptr` to use a dictionary (array), indexed by token
* Refactor errors into own module
* Refactor while loops to use bytecode interface when emitting backwards jump


## To Do

* More descriptive error messages, including source code snippets
* Associate line numbers with bytecode instructions for error handling
* At minimum 20 tests for all constructs
* Write proper benchmark tester with averages over 30 trials
* Complete TODOs in code
* Write failing tests
* Compose writeup on internal workings
* Add unicode support
