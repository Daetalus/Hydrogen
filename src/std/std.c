
//
//  Standard Library
//

#include <hystdlib.h>


// Register the entire standard library.
void hy_add_stdlib(HyState *state) {
	hy_add_io(state);
	hy_add_err(state);
}


// Register the IO library.
void hy_add_io(HyState *state) {

}


// Register the error library.
void hy_add_err(HyState *state) {

}
