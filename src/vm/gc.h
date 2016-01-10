
//
//  Garbage Collection
//

#ifndef GC_H
#define GC_H

#include "value.h"


// Data required by the garbage collector.
typedef struct garbage_collector {
	// The first object in the linked list of all instantiated objects.
	Object *head;

	// The total amount of memory allocated in the current cycle.
	uint32_t allocated;

	// The threshold amount of memory that will trigger a collection.
	uint32_t threshold;
} GarbageCollector;


// Creates a new garbage collector.
GarbageCollector gc_new(void);

// Frees all objects the garbage collector is keeping track of.
void gc_free(GarbageCollector *gc);

// Collects garbage.
void gc_collect(VirtualMachine *vm);

// Checks if a GC run is necessary, and runs one if it is.
void gc_check(VirtualMachine *vm);

// Adds an object to a GC's object list.
static inline void gc_add(GarbageCollector *gc, struct object *obj) {
	obj->next = gc->head;
	gc->head = obj;
}

#endif
