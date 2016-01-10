
//
//  Garbage Collector
//

#include "gc.h"
#include "vm.h"


// The initial threshold for the garbage collector.
#define INITIAL_THRESHOLD (10 * 1024 * 1024)


// Creates a new garbage collector.
GarbageCollector gc_new(void) {
	GarbageCollector gc;
	gc.head = NULL;
	gc.allocated = 0;
	gc.threshold = INITIAL_THRESHOLD;
	return gc;
}


// Frees all objects the garbage collector is keeping track of.
void gc_free(GarbageCollector *gc) {
	Object *current = gc->head;
	while (current != NULL) {
		Object *next = current->next;
		free(current);
		current = next;
	}

	// Reset the GC
	gc->head = NULL;
	gc->allocated = 0;
	gc->threshold = INITIAL_THRESHOLD;
}


// Checks if a GC run is necessary, and runs one if it is.
void gc_check(VirtualMachine *vm) {
	if (vm->gc.allocated >= vm->gc.threshold) {
		hy_collect_garbage(vm);
	}
}


// Run the garbage collector.
void hy_collect_garbage(HyVM *vm) {
	// Roots:
	// * Stack
	// * Upvalues
	// * Package top level locals


}
