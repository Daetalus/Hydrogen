
//
//  Expression Test
//


#include "test.h"

#include "../../src/vm.h"
#include "../../src/compiler.h"
#include "../../src/expression.h"
#include "../../src/lexer.h"
#include "../../src/debug.h"


// Create a compiler globally that we can reuse across tests.
Compiler *compiler;

void clear_bytecode() {
	compiler->fn->bytecode.count = 0;
}


START(single_operand);
	clear_bytecode();
	expression(compiler, TOKEN_END_OF_FILE);
	pretty_print_bytecode(&compiler->fn->bytecode);
END();


MAIN(expression);
	VirtualMachine vm = vm_new("3 + 4");

	Function fn;
	fn.name = "test";
	fn.name_length = 4;
	fn.argument_count = 0;
	bytecode_new(&fn.bytecode, 10);

	compiler->vm = &vm;
	compiler->local_count = 0;
	compiler->scope_depth = 0;
	compiler->has_error = false;
	compiler->fn = &fn;

	RUN(single_operand);
MAIN_END();
