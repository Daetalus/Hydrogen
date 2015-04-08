
To Do
=====


## Refactoring

* Separate parser out from lexer
* Hide struct interfaces in .c files
* Write proper interface functions
* Objectify compiler code
* Group files into subfolders
* Prefix functions with class name
* Add static infront of functions used in single file
* Refactor #defines into C functions
* Add operator table and parse expressions using it
* Rename `string_duplicate` to `string_copy`
* Add library function interface
* Remove unnecessary defines
* Refactor addition operator to remove duplicate code
* Refactor `patch_jump` to only accept conditional and forward jump, or to handle backwards jumps
* Refactor while loops to use bytecode interface when emitting backwards jump
* Completely re-write documentation
* Compose writeup on internal workings
* Disable lexer newlines by default
* Remove need to cache conditional patch locations in multiple else if statements
* Refactor location/length into own struct
* Refactor `operator_ptr` to use a dictionary (array), indexed by token
* Refactor errors into own module
* Reduce coupling between parts of code. If only needs lexer, only takes lexer as argument
* Remove compiler dependency on vm
* Typedef instruction as uint8_t
* Typedef value as uint64_t
* Remove useless comments


## To Do

* More descriptive error messages, including source code snippets
* Associate line numbers with bytecode instructions for error handling
* At minimum 20 tests for all constructs
* Write proper benchmark tester with averages over 30 trials
* Complete TODOs in code
