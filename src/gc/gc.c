/*
** # Garbage Collection: The `gc` module.
**
** 
*/

#include "gc.h"
#include "../test/test.h"

#define STACK_FRAMES 0x8
#define STACK_MASK   0x7

struct _gc_t {
    gc_t master;
    void *black;
    void *grey;
    void *white;
    void *stacks[STACK_FRAMES];
    size_t stack_index;
    size_t num_objects;
    // TODO: add children element.
};

gc_t gc_init() {
   gc_t gc = {malloc(sizeof(_gc_t))};
   gc._data->master = {NULL};
   gc._data->black = NULL;
   gc._data->gray  = NULL;
   gc._data->white = NULL;
   for (int i; i < STACK_FRAMES; i++)
       gc._data->stacks[i] = NULL;
   gc._data->stack_index = 0;
   gc._data->num_objects = 0;
   // TODO: Initialize children element.
   return gc;
}

gc_t gc_fork(gc_t gc) {
   // TODO: implement, using children element.
   return gc_init();
}

void gc_free(gc_t gc) {
    // TODO: free children as well.
}

void gc_defaultFreeFn(gc_t gc, void* object);

void gc_defaultMarkFn(gc_t gc, void* object);

void gc_mark(gc_t, void* parent, void* child);

void gc_register(gc_t gc, void* object) {
    gc_header_t *header = object;
    header->
    LINK(gc->white, object);
}

void gc_enter(gc_t gc);

void gc_leave(gc_t gc);

void gc_return(gc_t gc, void* object);


