
Writing a Language
==================


We're given a source code file (a string), and we need to execute it. There are a few steps involved:

* Lexing
* Compilation
* Execution


## Definitions

A few definitions first:

* **Identifier** - A sequence of letters (lower or upper case), numbers, or underscores. Must not start with a number.
* **Bytecode** - Like assembly code. Simple instructions that we can easily execute.


## Lexing

Lexing takes in the source code string, and outputs a stream of tokens. A token is the smallest unit of recognisable syntax. The tokens emitted by the lexer are:

Mathematical operators:

* Addition (`+`)
* Subtraction (`+`)
* Multiplication (`+`)
* Division (`+`)
* Modulo (`+`)

Boolean operators:

* Boolean and (`&&`)
* Boolean or (`||`)
* Boolean not (`!`)
* Equal (`==`)
* Not equal (`!=`)
* Less than (`<`)
* Less than or equal to (`<=`)
* Greater than (`>`)
* Greater than or equal to (`>=`)

Bitwise operators:

* Left shift (`<<`)
* Right shift (`>>`)
* Bitwise and (`&`)
* Bitwise or (`|`)
* Bitwise not (`~`)
* Bitwise xor (`^`)

Assignment:

* Assign (`=`)
* Addition assignment (`+=`)
* Subtraction assignment (`-=`)
* Multiplication assignment (`*=`)
* Division assignment (`/=`)
* Modulo assignment (`%=`)

Syntax:

* Open parenthesis (`(`)
* Close parenthesis (`)`)
* Open bracket (`[`)
* Close bracket (`]`)
* Open brace (`{`)
* Close brace (`}`)
* Dot (`.`)
* Comma (`,`)

Keywords:

* Let (`let`)
* If (`if`)
* Else (`else`)
* Else If (`else if`)
* While (`while`)
* Loop (`loop`)
* Break (`break`)
* For (`for`)
* In (`in`)
* Function (`fn`)
* True (`true`)
* False (`false`)
* Nil (`nil`)

Other:

* Identifiers (eg. `test` or `_variable_1`)
* Numbers (eg. `13` or `42.8`)
* String literals (eg. `"test"` or `'hello'`)
* Newlines (`\n` or `\r`)
* End of file

The lexer gives the source code to a parser. The parser helps the lexer sequentially extract bits of the source code. To do this, it has a cursor - a position within the source code string. The cursor position is incremented to iterate over characters. This allows easy extraction of numbers, identifiers, etc. It also provides simple functions like `current_char`, which returns the character under the cursor, `starts_with`, to test if the string at the current cursor position equals something, and `is_eof`, which returns true if the cursor has reached the end of the file.

For example, say we have the source code `let a = 3`. The parser starts with a cursor position of 0 (the first character in the source). We can look at the character at the current cursor position, and sees that it's a letter (the letter `l`). We could call the function `consume` to increment the current cursor position, or call `consume_identifier` to consume a whole identifier, etc. Take a look at `parser.h` to see all the available functions.

The lexer uses the parser and its functionality to extract tokens from the source. To parse a token, it first consumes all spaces and tabs. If the next character is a letter, then we have an identifier, so we consume it and emit an identifier token. If the next character is a number, then we use the parser to extract the number, and emit a number token. If it's a quote (`"` or `'`), then we have a string literal. If the character is `(`, for example, then we emit an open parenthesis token, etc.


## Compilation

The compiler uses a lexer, and interprets a sequence of tokens as a statement, which it generates bytecode for. There are 3 major constructs used to structure code:

* **Block** - Contain a sequence of statements, one after each other. Usually surrounded by `{` and `}`.
* **Statement** - A single language construct that can be treated independently, like a while loop or if/else block. This includes the block attached to any statement. Contains expressions.
* **Expression** - A sequence of operators and operands that will result in a value. For example, the expression `3 + 4 * 5` results in the value of 23.

We begin by treating the contents of the whole file as one big block, terminated by an end of file token.

To compile a block, we just keep compiling statements until we find the block's terminator token, such as a `}`.

To compile a statement, we determine what sort of statement it is (eg. is it a for loop, if statement, etc.), then handle each case separately.

To compile an expression, we use a Pratt parser, explained well by Bob Nystrom in his post [here](http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/). I won't bother explaining it - you can just read the article.

For example, compiling a while loop involves consuming the while keyword, then compiling an expression terminated by an open brace, then consuming the open brace, compiling a block terminated by a close brace, and then consuming the close brace.

### Variable Assignment

### If Statements

### While Loops

### Break Statements

### Function Definitions

### Return Statements

* Functions return by pushing their return value (or nil if the function doesn't have a return value) onto the stack
* When the virtual machine encounters a return instruction, it pops the

### Closures

* An upvalue is a value in the stack of an enclosing function used in a sub-function (usually an anonymous function)
	* The upvalue must be kept in scope even when the enclosing function's stack frame is popped
* Closures are functions which capture upvalues
* Closures are represented as a type of variable
	* Stored as an index into the VM's closure list

```
struct Upvalue {
	// True when we've closed the upvalue
	bool closed;

	// The absolute position of the upvalue in the VM's stack when the upvalue is open
	int stack_position;

	// The upvalue's value when it is closed, hoisted out of the stack and copied into here
	uint64_t value;

	// The name of the upvalue we're closing over. Used when we encounter an identifier and need to check if an upvalue has already been created for it. To avoid collisions with different upvalues named the same thing, this is NULLed when the original local goes out of scope.
	char *name;

	// The length of the name. Set to 0 when the original local goes out of scope.
	int length;
}
```

* Creating closures (in expressions)
	* Defined like any ordinary function, returning index and `Function` struct pointer
	* Compile function block like any other function with a compiler
	* Convert index into value and push onto stack
* Calling closures (separately in function calls or in expressions) in VM
	* Pop variable to call
	* Check if closure in VM
		* Trigger runtime error if not closure
	* Call bytecode by pushing function frame, like ordinary function
* Using (pushing or storing) variables from an outer function's stack (capturing an upvalue)
	* When encountering an identifier:
	* Check current compiler's local list
		* Not an upvalue, so just push the local
	* Iterate over all the VM's upvalues
		* Compare the name of the upvalue to the identifier (ENSURE WE CHECK FOR THE CASE WHERE THE NAME IS NULL)
		* If we get a match, then emit a push upvalue instruction with the upvalue's index
	* Else, we need to create a new upvalue
		* Recursively iterate over parent compiler's locals lists until we have a pointer to the original local
		* Create a new upvalue in the VM's list
		* Mark the local as an upvalue
		* Set the upvalue to open
		* Calculate and set the absolute stack position of the upvalue
* Persisting captured variables after function's stack is popped (closing an upvalue)
	* On scope pop, check if local is upvalue (via marked attribute)
	* If it is, push close upvalue instruction with index of upvalue in function's upvalues list
	* Close instructions must come before pop instructions
	* On function return, where locals aren't popped, emit close upvalue instructions before calculating the return expression
	* When closing upvalue, set the upvalue's name char pointer to NULL and its length to 0, to avoid collisions with future upvalues that are potentially named the same

	* Closing an upvalue during execution in the VM:
		* Shallow copy of value in stack into the value in the upvalue
		* Set closed to true
* Instead of `PUSH_LOCAL`, use `PUSH_UPVALUE`
	* If upvalue is open, push item at absolute stack location
	* If upvalue is closed, push value of upvalue
* Instead of `STORE_LOCAL` instruction, use `STORE_UPVALUE` instruction
	* If upvalue is open, modify value at absolute stack position
	* If upvalue is closed, modify value stored in upvalue itself


* Upvalues have a relative stack index inside their defining function, and a root stack index of their defining function
* Store pointers to all variable definitions in a function that are later used as upvalues
* When a function frame is pushed, iterate over all defined upvalues and set the stack index of their defining function to be the stack_start of the function
* When indexing/modifying/closing open upvalues, index the stack using `defining_function_stack_index` + `relative_stack_index`
* Remove `absolute_stack_position` stuff


* Places to change:
	* Variables as operands in expressions
	* Calling closures in function calls
	* Storing in variable assignment

### Classes

```
struct Field {
	char *name;
	int length;
}

struct ClassDefinition {
	char *name;
	int length;

	Field fields[MAX_FIELDS];
	int field_count;

	// A list of indices in the VM's function list of all the
	// class' constructor functions.
	int constructors[MAX_CONSTRUCTORS];
	int constructor_count;

	// List of indices in the VM's function list of the class'
	// defined methods.
	int methods[MAX_METHODS];
	int method_count;
}

struct Class {
	// Pointer to the class' definition.
	ClassDefinition *definition;

	// Values of each of the class's fields
	uint64_t fields[0];
}
```

**Cases**

* Define class (compilation only)
* Define method (compilation only)
* Construct class (compilation/execution)
* Call method (compilation/execution)
	* Must be able to call result of an expression, ie. `(fn() {io.println(3)})()`
* Push field (compilation/execution)
	* Must be an operator (ie. `thing.transform.position.x`)
* Store field (compilation/execution)


**Steps**

* Treat functions as locals
	* 4 types of function variables: native function, function, closure, method
	* When encounter an identifier in an expression
		* Check locals
		* Check native functions
* Treat `(arg1, arg2, ...)` as a postfix operator in an expression
	* Emits bytecode to push the arguments
	* Emits bytecode to call the preceding expression with the arguments
		* Arguments to call instruction include number of arguments, so we can check at runtime whether the variable we're calling is a native/function/closure/method and takes the provided number of arguments
* Treat `.identifier` as a postfix operator
	* Emits bytecode to push the field named `identifier` of the expression before the operator onto the stack
	* No runtime tests yet, but unit tests for compiler
* Compile class definitions with fields
	* Optional braces defining field names
	* Trigger compile error if class already exists
* Compile class field access/storing
* Compile method definitions
	* Add methods to class definition
* Compile constructors

**Instructions**

* `CALL`
	* Only call instruction (remove all others)
	* Parses the number of arguments provided as an argument in the bytecode
	* Pops a variable off the stack
	* Triggers runtime error if not a native/function/closure/method
	* Triggers runtime error if number of arguments provided in bytecode does not match number of arguments required by method
	* Calls the function
	* Return value is left on the stack
* `PUSH_FIELD`
	* Pops a variable off the stack
	* Triggers runtime error if not a class
	* Converts class value into a pointer
	* Gets pointer to class definition in VM (from the pointer)
	* Gets the index of the requested field in the class definition's fields list
	* Triggers runtime error if field doesn't exist
	* Indexes the class' fields list with the retrieved index, pushing the value onto the stack
* `STORE_FIELD`
	* Fetches name of field to store to as 8 byte bytecode pointer and 2 byte length argument
	* Pops result of expression off the stack
	* Pops the variable's receiver off the stack
		* Ie. stack contains: ..., receiver, expression result




* Store list of class definitions in VM
* Encounter class definition
	* Create new class definition in VM
	* Populate fields list
* Encounter method definition
	* Compile method as a new function (telling the compiler that its a method so it knows to add the receiver as a variable on the stack)
	* Add to VM's function list
	* Add to class' method definitions list (or constructor list)
* Encounter new class in expression (during compilation)
	* Emit `NEW_CLASS` instruction with index in VM's class definitions list
	* Emit call to class's constructor
		* Assert the number of arguments are equal
		* Push receiver
		* Push constructor arguments
		* Emit normal function call
* Encounter method call (during compilation)
	* Push receiver (may or may not be an object with the requested method, doesn't matter - trigger an error at runtime rather than compile time)
	* Push arguments
	* Emit call function instruction (no special function call)
	* Compile function block, telling the sub-compiler that the function is a method (so it knows to push the receiver as a local) on a class (so it knows what class to check for defined fields)
* Encounter field access (ie. self.field or object.field)
	* Push receiver
	* Emit `PUSH_FIELD` instruction with field index in class


* Encounter `NEW_CLASS` instruction (during execution)
	* Create a new class on the heap, allocating enough space for all its fields as values
		* Add it to the list of all objects for garbage collection
	* !!!!!!!!!! Set new class' fields to `nil` !!!!!!!!!! **IMPORTANT**
* Encounter `PUSH_FIELD` instruction (during execution)
	* Get index of
	* Pop off top of stack (the receiver)
	*

* Method call and dot operator need to be recursive (to handle levels of indirection, ie. thing.position.x, and (fn(a, b) {io.println(a, b)})(1, 2))

* New instructions:
	* `NEW_CLASS`, 1 arg - index in VM's class definitions list as the prototype of the class to define
	* `PUSH_FIELD`, 1 arg - index in class' fields list
		* Pops the receiver off the stack (as the receiver)
		* Index's receiver's fields list, and pushes field at index onto the stack

* New expression operators:
	* `(` acts as an operand and as a operator

## Garbage Collection

## Execution
