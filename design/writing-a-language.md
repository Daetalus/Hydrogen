
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

## Garbage Collection

## Execution
