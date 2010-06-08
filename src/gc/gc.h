/*
** # Garbage Collection: The `gc` module.
**
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
struct gc_t { struct _gc_t *_data; };

/*
** ## Constructors, Destructors
**
** `gc_init` creates the master-level garbage collector, while `gc_fork`
** creates a thread-specific `gc_t`, that dumps its contents to the
** master, once it's full. This way, `gc_register` can be implemented
** without locks and without race conditions.
**
** Use one `gc_fork` per thread. If you only have one thread, `gc_init`
*** is all you need (since no race conditions will arise anyway).
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
**             doSomething(&gc);
**             gc_free(gc);
**             return EXIT_SUCCESS;
**         } else {
**             gc_t gc_master = gc_init();
**             gc_t *gc_slaves = malloc(nthreads * sizeof(gc_t));
**             thread_t *threads = malloc(nthreads * sizeof(gc_t));
**             for (int i = 0; i < nthreads; i++) {
**                 gc_slaves[i] = gc_fork(gc_master);
**                 threads[i] = thread_new(doSomething, &gc_slaves[i]);
**             }
**             for (int i = 0; i < nthreads; i++)
**                 thread_start(threads[i]);
**             for (int i = 0; i < nthreads; i++)
**                 thread_waitForExit(threads[i]);
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
**     void foo_freeFn(gc_t gc, void* _data) {
**         foo_t foo = {_data};
**         free(foo._data->str);
**         free(foo._data);
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
** - `parent` is the object that holds a 
** - `child` is the object that 
**
** Example:
** 
**    void foo_markFn(gc_t gc, void* _data) {
**       foo_t foo = {_data};
**       gc_mark(gc, foo._data, foo._data->bar._data);
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
**     struct foo_t { struct _foo_t* _data; }
**     struct _foo_t {
**         gc_header_t _gcHeader;
**         char* str;
**         bar_t bar;
**     }
**     
**     foo_t foo_new(gc_t gc) {
**         foo_t foo = {malloc(sizeof(_foo_t))};
**         foo._data->_gcHeader.freeFn = foo_freeFn;
**         foo._data->_gcHeader.markFn = foo_markFn;
**         
**         foo._data->str = strdup("John Smith");
**         foo._data->bar = bar_new();
**         gc_register(gc, foo._data); // Look below for docs.
**     }
*/
struct gc_header_t {
    gc_freeFn_t freeFn;
    gc_markFn_t markFn;
};

/*
**
*/
void gc_register(gc_t gc, void* object);

/*
** 
*/
void gc_enter(gc_t gc);
void gc_leave(gc_t gc);

/*
** 
*/
void gc_return(gc_t gc, void* object);

#endif

