
//
//  Virtual Machine
//

#ifndef VM_H
#define VM_H

#include <hydrogen.h>

#include "vec.h"
#include "pkg.h"
#include "fn.h"
#include "struct.h"
#include "parser.h"


// The interpreter state, used to execute Hydrogen source code. Variables,
// functions, etc. are preserved by the state across calls to `hy_run`.
struct hy_state {
	// We store all functions, native functions, struct definitions, and
	// upvalues in the interpreter state rather than in their respective
	// packages in order to simplify the bytecode (we don't have to specify
	// a package index in each instruction). The cost is that we can only define
	// 2^16 functions/structs/etc across all packages, rather than per package.
	Vec(Package) packages;
	Vec(Function) functions;
	Vec(NativeFunction) natives;
	Vec(StructDefinition) structs;
	Vec(Upvalue) upvalues;

	// We can't store 64 bit values like numbers (doubles) and strings
	// (pointers) directly in the bytecode (because each argument is only 16
	// bits), so we use an index into these arrays instead.
	//
	// The constants array holds all number literals and values defined using
	// `const`. Struct fields are stored as the hash of the field name.
	Vec(HyValue) constants;
	Vec(String *) strings;
	Vec(StringHash) fields;

	// We use longjmp/setjmp for errors, which requires a jump buffer, which we
	// store in the state (instead of as a global variable).
	jmp_buf error_jmp;

	// This is set to a heap allocated error object before longjmp is called, so
	// we can return it to the user calling the API function.
	HyError *error;
};

#endif
