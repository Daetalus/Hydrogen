
//
//  Garbage Collector
//

#include "gc.h"
#include "vm.h"


// * A traditional mark and sweep garbage collector
// * Keeps track of how much memory is allocated
// * Triggers the GC once this allocation count hits a threshold (which grows
//   every time it's hit)
// * Iterates over all accessible values (called roots), which can come from the
//   stack, upvalues, or top level locals in packages
// * Marks each of these values as "in use"
// * Then iterates over all objects currently allocated (stored in a linked
//   list), freeing any of them that aren't marked


// The initial threshold for the garbage collector.
#define INITIAL_THRESHOLD (10 * 1024 * 1024)


// Creates a new garbage collector.
GarbageCollector gc_new(void) {
	GarbageCollector gc;
	gc.head = NULL;
	gc.allocated = 0;
	gc.threshold = INITIAL_THRESHOLD;
	gc.mark = 0;
	return gc;
}


// Frees all objects the garbage collector is keeping track of.
void gc_free(GarbageCollector *gc) {
	Object *current = gc->head;
	while (current != NULL) {
		// Save the next object so we don't access freed memory after we release
		// the current object
		Object *next = current->next;
		free(current);
		current = next;
	}

	// Reset the GC
	gc->head = NULL;
	gc->allocated = 0;
	gc->threshold = INITIAL_THRESHOLD;
}


// Mark a value (checking to make sure it's an object).
static void gc_mark_object(uint64_t value) {
	// Only mark objects, ignore the value otherwise
	if (IS_PTR_VALUE(value)) {
		((Object *) val_to_ptr(value))->mark ^= 0x01;
	}
}


// Mark the stack.
static void gc_mark_stack(uint64_t *stack, uint32_t size) {
	for (uint32_t i = 0; i < size; i++) {
		gc_mark_object(stack[i]);
	}
}


// Mark all upvalues.
static void gc_mark_upvalues(VirtualMachine *vm, uint64_t *stack) {
	for (uint32_t i = 0; i < vm->upvalues_count; i++) {
		Upvalue *upvalue = &vm->upvalues[i];

		// The value of the upvalue depends on whether it's open or not
		uint64_t value;
		if (upvalue->open) {
			// Fetch it from the stack
			value = stack[upvalue->fn_stack_start + upvalue->slot];
		} else {
			// Stored in the upvalue itself
			value = upvalue->value;
		}

		gc_mark_object(value);
	}
}


// Mark all top level variables.
static void gc_mark_top_level(VirtualMachine *vm) {
	// Iterate over all locals in all packages
	for (uint32_t i = 0; i < vm->packages_count; i++) {
		Package *package = &vm->packages[i];
		for (uint32_t j = 0; j < package->locals_count; j++) {
			gc_mark_object(package->values[j]);
		}
	}
}


// Frees an object. The previous object in the linked list is required in order
// to delete it from the linked list.
static void gc_free_object(GarbageCollector *gc, Object *obj, Object *previous) {
	// Unmarked object
	// Unlink it from the linked list
	if (previous == NULL) {
		gc->head = obj->next;
	} else {
		previous->next = obj->next;
	}

	// Deduct the size of the object from the GC's allocation count
	if (obj->type == OBJ_STRING) {
		gc->allocated -= sizeof(String) + ((String *) obj)->length + 1;
	} else {
		gc->allocated -= sizeof(Struct) + sizeof(uint64_t) *
			((Struct *) obj)->definition->fields_count;
	}

	// Free it
	free(obj);
}


// Free all unmarked objects.
static void gc_sweep(GarbageCollector *gc) {
	// We need both the previous and current element in the linked list in
	// order to delete something from it
	Object *previous = NULL;
	Object *current = gc->head;

	while (current != NULL) {
		if (current->mark != gc->mark) {
			// Save the next element in the linked list so we don't access freed
			// memory after releasing the current object
			Object *next = current->next;

			// Free it
			gc_free_object(gc, current, previous);

			// Keep the previous object the same so we can remove consecutive
			// elements from the list
			current = next;
		} else {
			previous = current;
			current = previous->next;
		}
	}
}


// Run the garbage collector.
void gc_collect(GarbageCollector *gc, VirtualMachine *vm, uint64_t *stack,
		uint32_t stack_size) {
	// 3 places GC roots can come from: stack, upvalues, top level locals
	gc_mark_stack(stack, stack_size);
	gc_mark_upvalues(vm, stack);
	gc_mark_top_level(vm);
	gc_sweep(gc);

	// Swap the meaning of the mark bit
	gc->mark ^= 0x01;
}
