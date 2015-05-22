
To Do
=====


## Refactoring

* Prefix functions with class name
* Add static in front of functions used in single file


## To Do

* More descriptive error messages, including source code snippets
* Associate line numbers with bytecode instructions for error handling
* At minimum 20 tests for all constructs
* Complete TODOs in code
* Write failing tests
* Compose writeup on internal workings
* Add unicode support
* Add \U unicode escape in string literals (specify a unicode character with hex)
* Try pointer casting instead of union conversion for numbers <-> values
	* Ie. `uint64_t value = *((uint64_t *) &(number))`, where `number` is a double.
	* Test performance for each
* Computed gotos
* Use a binary search instead of linear searches (eg. searching for a method on a class)
* Impose hard restrictions on length limits (ie. limits on variable name length, number of functions, etc.)
* Heap allocate functions list (remove potential for stack overflow and remove maximum limit on number of functions)
* Heap allocate runtime call stack to raise stack overflow limit
* Short circuiting boolean operators
* Replace call native with each of the binary operators as bytecode instructions
* Trigger compile time error for accessing self in functions which aren't defined as methods
* Trigger compile time error for accessing fields on self that don't exist
