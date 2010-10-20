/*
** # gc/gc: Garbage Collection
**
** Header file: gc/gc.h
** Source file: gc/gc.c
** Prefix: gc
** 
*/

#ifndef GC_H
#define GC_H

/*
** `gc_t` is the main garbage collection structure. Pass a `gc_t` around
** to any function that needs garbage collection. We unfortunately can't
** keep a global `gc_t`, but this actually has its own set of advantages.
**
** To get a copy of `gc_t`, look below at `gc_init` and at `gc_fork`.
**
** (`struct _gc_t` is defined in `gc.c`)
*/
typedef struct _gc_t *gc_t;

/*
** ## Constructors, Destructors
**
** `gc_init` creates the master-level garbage collector, while `gc_fork`
** creates a thread-specific `gc_t`, that dumps its contents to the
** master, once it's full. This way, `gc_register` can be implemented
** without locks and without race conditions.
**
** Use one `gc_fork` per thread. If you only have one thread, `gc_init`
** is all you need (since no race conditions will arise anyway).
**
** Use `gc_free` to free a garbage collector and all associated data. If
** you use `gc_free` on a master `gc_t`, all of its slaves will be
** automatically freed.
**
** Example:
**
**     int main(int argc, char** argv) {
**         if (argc != 2) return EXIT_FAILURE;
**         int nthreads = atoi(argv[1]);
**         if (nthreads <= 0) return EXIT_FAILURE;
**         if (nthreads == 1) {
**             gc_t gc = gc_init();
**             doSomething(gc);
**             gc_free(gc);
**             return EXIT_SUCCESS;
**         } else {
**             gc_t gc_master = gc_init();
**             thread_t *threads = malloc(nthreads * sizeof(gc_t));
**             for (int i = 0; i < nthreads; i++) {
**                 gc_t gc_slave = gc_fork(gc_master);
**                 threads[i] = thread_new(doSomething, gc_slave);
**             }
**             for (int i = 0; i < nthreads; i++)
**                 thread_start(threads[i]);
**             for (int i = 0; i < nthreads; i++)
**                 thread_waitForExit(threads[i]);
**             free(threads);
**             gc_free(gc_master);
**             return EXIT_SUCCESS;
**         }
**     }
*/
gc_t gc_init();
gc_t gc_fork(gc_t);
void gc_free(gc_t);

/*
** ## Freeing Objects
**
** When the garbage collector finds an object that is undoubtedly
** garbage, it needs to know how to delete it. For this reason, each
** garbage-collected object carries around with it a pointer to a `freeFn`
** -- a function that destructs the object safely.
**
** The default behavior, as can be seen in gc_defaultFreeFn, is to simply
** deallocate the object pointer, without regard to whatever un-GCed
** pointers may be contained in the object. This behavior should work
** for the vast majority of objects.
**
** Note that one does not need to (and should not) free pointer to other
** objects that are being kept track by the garbage collector. In other
** words, only free that which won't be freed already, lest ye risk a
** segmentation fault.
**
** Example:
**
**     void foo_freeFn(gc_t gc, void* data) {
**         foo_T foo = data;
**         free(foo->str);
**         free(foo);
**     }
*/
typedef void gc_freeFn_t(gc_t gc, void* object);
void gc_defaultFreeFn(gc_t gc, void* object);

/*
** ## Marking
**
** For the garbage collector to determine whether or not an object is
** garbage, it must ask every other object whether or not it needs this
** object around. If no object needs an object, that object is garbage.
**
** In order to do this, every object must carry around with it a `markFn`
** -- a marking function. The `markFn` must call `gc_mark` for every
** object that is necessary.
**
** The default `markFn`, `gc_defaultMarkFn`, doesn't do anything. This is
** suitable for objects that contain no other GC'ed objects, but that's
** it. If your object contains other GC'ed objects, you need to roll your
** own `markFn`.
**
** `void gc_mark(void* parent, void* child)`:
**
** * declares that `parent` has a reference to `child`.
** * can be used outside of a markFn.
**
** Example:
** 
**    void foo_markFn(gc_t gc, void* data) {
**       foo_t foo = data;
**       gc_mark(gc, foo, foo->bar);
**    }
*/
typedef void gc_markFn_t(gc_t gc, void* object);
void gc_defaultMarkFn(gc_t gc, void* object);
void gc_mark(gc_t, void* parent, void* child);

/*
** ## The GC Header
** 
** As was said above, each object needs to carry around with it a
** `freeFn`, and a `markFn`. To do this, every GC'ed object must begin
** with a GC header (`gc_header_t`).
** 
** If your data type is relatively simple, you can use
** `gc_defaultHeader`, rather than rolling your own.
**
** Example:
**
**     typedef struct _foo_t *foo_t;
**     struct _foo_t {
**         gc_header_t gcHeader;
**         char* str;
**         bar_t bar;
**     }
**     
**     foo_t foo_new(gc_t gc) {
**         foo_t foo = malloc(sizeof *foo);
**         foo->gcHeader.freeFn = foo_freeFn;
**         foo->gcHeader.markFn = foo_markFn;
**         
**         foo->str = strdup("John Smith");
**         foo->bar = bar_new(gc);
**         gc_register(gc, foo); // Look below for docs.
**     }
*/
struct gc_header_t {
    gc_list_t _refs;
    struct gc_header_t *_parent, *_children;
    gc_freeFn_t freeFn;
    gc_markFn_t markFn;
};

/*
** Use `gc_register` to register an object with the garbage collector. An
** object register for the first time is assumed to be in the current
** stack frame (see below).
** 
** An object can be gc_register'ed again in order to flush its perceived
** state. What this means is that the gc keeps track of an object's
** references (through gc_mark, and through markFn), to speed things up.
** However, if some references are removed, gc_t has no way of knowing
** about it unless you do gc_register again.
*/
void gc_register(gc_t gc, void* object);

/*
** ## Stack Frames
**
** TODO: explain.
*/
void gc_enter(gc_t gc);
void gc_leave(gc_t gc);
void gc_return(gc_t gc, void* object);

#endif
