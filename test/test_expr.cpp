
//
//  Expression Tests
//

extern "C" {
#include <hydrogen.h>
#include <vec.h>
#include <parser.h>
#include <pkg.h>
#include <vm.h>
}

#include <gtest/gtest.h>


// Creates a new compiler.
#define COMPILER(code) \
	char *source = (code);
	HyState *state = hy_new();
	Index pkg_index = pkg_new(state);
	Package *pkg = &vec_at(state->packages, pkg_index);
	Index src_index = pkg_add_string(pkg, source);
	pkg_compile()

