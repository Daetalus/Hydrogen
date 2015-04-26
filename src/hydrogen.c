
//
//  Hydrogen
//


#include "vm.h"


// Runs a source string.
void hydrogen_run(char *source) {
	VirtualMachine vm = vm_new(source);
	vm_attach_standard_library(&vm);
	vm_compile(&vm);
	vm_run(&vm);
	vm_free(&vm);
}
