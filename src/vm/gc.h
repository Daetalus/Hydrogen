
//
//  Garbage Collection
//

#ifndef GC_H
#define GC_H

#include "value.h"


// The multiplier applied to the GC's threshold each time the GC is triggered.
#define GROWTH_RATE 1.5


// Forward declaration of VM.
struct vm;


// Data required by the garbage collector.
typedef struct garbage_collector {
	// The first object in the linked list of all instantiated objects.
	Object *head;

	// The total amount of memory allocated in the current cycle. Use a 64 bit
	// integer, because a 32 bit one could only allocate up to 4GB.
	uint64_t allocated;

	// The threshold amount of memory that will trigger a collection.
	uint64_t threshold;

	// The value of an object's mark bit which indicates it has been marked.
	uint8_t mark;
} GarbageCollector;


// Creates a new garbage collector.
GarbageCollector gc_new(void);

// Frees all objects the garbage collector is keeping track of.
void gc_free(GarbageCollector *gc);

// Run the garbage collector.
void gc_collect(GarbageCollector *gc, struct vm *vm, uint64_t *stack,
		uint32_t stack_size);


// Checks if a GC run is necessary, and runs one if it is.
static inline void gc_check(GarbageCollector *gc, struct vm *vm, uint64_t *stack,
		uint32_t stack_size) {
	if (gc->allocated >= gc->threshold) {
		gc_collect(gc, vm, stack, stack_size);

		// Grow threshold
		gc->threshold *= GROWTH_RATE;
	}
}


// Adds an object to a GC's object list. The size is needed so we can increment
// the GC's allocation count.
static inline void gc_add(GarbageCollector *gc, struct object *obj,
		size_t size) {
	obj->next = gc->head;
	gc->head = obj;
	gc->allocated += size;
}

#endif
