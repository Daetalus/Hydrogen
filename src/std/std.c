
//
//  Standard Library
//

#include <hystdlib.h>


// Register the entire standard library.
void hy_add_stdlib(HyVM *vm) {
	hy_add_io(vm);
}
