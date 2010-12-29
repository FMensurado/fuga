/**
*** # Garbage Collection
***
*** Header file: fuga/gc.h
*** Source file: fuga/gc.c
*** Prefix: FugaGC
**/

#ifndef FUGAGC_H
#define FUGAGC_H

#include "gclist.h"
#include <stdlib.h>

/**
*** `gc_t` is the main garbage collection structure. Pass a `gc_t` around
*** to any function that needs garbage collection. We unfortunately can't
*** keep a global `gc_t`, but this actually has its own set of advantages.
***
*** To get a copy of `gc_t`, look below at `gc_start` and at `gc_fork`.
***
*** (`struct _gc_t` is defined in `gc.c`)
**/
typedef struct FugaGC FugaGC;

/** 
*** ## Constructors, Destructors
*** ### FugaGC_start
***
*** `FugaGC_start` allocates and initializes the garbage collector. Don't
*** forget to use `FugaGC_end` when you're done!
***
*** - Parameters: (none)
*** - Returns: the garbage collector.
*** - See also: `FugaGC_end`.
**/
FugaGC* FugaGC_start();

/**
*** ### FugaGC_end
***
*** `FugaGC_end` stops and deallocates the garbage collector, freeing all of
*** the allocated objects in the process.
***
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector to deallocate.
*** - Returns: void
*** - See also: `FugaGC_start`
**/
void FugaGC_end(FugaGC* gc);

/**
*** ## Freeing Objects
*** ### FugaGCFreeFn
***
*** When the garbage collector finds an object that is undoubtedly
*** garbage, it needs to know how to delete it. For this reason, each
*** garbage-collected object carries around with it a pointer to a `freeFn`
*** -- a function that destructs the object safely.
***
*** The default behavior, as can be seen in FugaGC_free, is to simply
*** deallocate the object, without regard to whatever un-GCed pointers may
*** be contained in the object. This behavior should work for the vast
*** majority of objects.
***
*** Note that one does not need to (and should not) free pointer to other
*** objects that are being kept track by the garbage collector. In other
*** words, only free that which won't be freed already, lest ye risk a
*** segmentation fault.
***
*** DO NOT CALL free() DIRECTLY WITH THE DATA. Use FugaGC_free() instead.
***
*** Example of a FugaGCFreeFn:
***
***     void Foo_free(void* data) {
***         Foo* foo = data;
***         free(foo->str);   // foo->str is an un-gc'd member of foo
***         FugaGC_free(foo); // foo still needs to be freed at this point.
***     }
**/
typedef void (*FugaGCFreeFn)(void* object);

/**
*** ### FugaGC_free
***
*** Free a garbage collected object safely. This function is only meant to
*** be called from within various FugaGCFreeFns, not from anywhere else. This
*** functions is itself a FugaGCFreeFn, synechdochally.
***
*** If you accidentally use `free` instead of `FugaGC_free`, you'll get a
*** sinister error. In AMD64 Linux/glibc, produces this is a core dump. I
*** don't know for other platforms/libraries.
***
*** - Parameters:
***     - `void* object`: the garbage collected object to free
*** - Returns: void
*** - See also: `FugaGCFreeFn`
**/
void FugaGC_free(void* object);

/**
*** ## Marking
*** ### FugaGCMarkFn
***
*** For the garbage collector to determine whether or not an object is
*** garbage, it must ask every other object whether or not it needs this
*** object around. If no object needs an object, that object is garbage.
***
*** In order to do this, every object must carry around with it a `markFn`
*** -- a marking function. The `markFn` must call `FugaGC_mark` for every
*** object that is necessary.
***
*** The default markFn, doesn't do anything. This is suitable for objects
*** that contain no other GC'ed objects, but that's it. If your object
*** contains other GC'ed objects, you need to roll your own `markFn`.
***
**/
typedef void (*FugaGCMarkFn)(void*, FugaGC*);

/**
*** ### FugaGC_mark
***
*** Declare that there is a link between parent and child. This has to
*** be done inside `FugaGCMarkFn`s.
***
*** Unlike FugaGC_free (which is a FugaGCFreeFn), this function is not a
*** `FugaGCMarkFn`.
***
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector.
***     - `void* parent`: The object which holds a link to child.
***     - `void* child`: The object referenced in `parent`. If you pass in
***     NULL, this function does nothing.
*** - Returns: void
*** - See also: `FugaGCMarkFn`
**/
void FugaGC_mark(FugaGC* gc, void* parent, void* child);

/**
*** ### FugaGC_root
***
*** Declare an object to be a root object, so that it and its references
*** don't get garbage collected. In general, it's a better idea to have
*** more root objects than to have one root object that contains all other
*** objects. However, the difference is small, and expressivity is more
*** important anyway.
*** 
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector.
***     - `void* object`: The object to declare as a root object.
*** - Returns: void
*** - See also: `FugaGC_unroot`
**/
void FugaGC_root   (FugaGC* gc, void* object);

/**
*** ### FugaGC_unroot
***
*** Remove the 'root' status from an object. In other words, the object
*** can now be garbage collected normally.
***
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector.
***     - `void* object`: The object to put up for regular garbage collection.
*** - Returns: void
*** - See also: `FugaGC_root`
**/
void FugaGC_unroot (FugaGC* gc, void* object);

/**
*** ## Allocation
*** ### FugaGC_alloc
***
*** Allocate a new object. The data is not initialized to anything (like
*** `malloc`).
***
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector.
***     - `size_t size`: The size of the new object.
***     - `FugaGCFreeFn`: Function that tells us how to free the object.
***     If NULL is passed, the default FreeFn is used.
***     - `FugaGCMarkFn`: Function that tells us the object's references.
***     If NULL is passed, the default FreeFn is used.
*** - Returns: The new object.
*** - See also: `FugaGCFreeFn`, `FugaGCMarkFn`
**/
void* FugaGC_alloc  (FugaGC* gc, size_t size, FugaGCFreeFn, FugaGCMarkFn);

/**
*** ## Collection
*** ### FugaGC_collect
***
*** Trigger a collection step. Once this is called, unrooted objects with
*** no references from rooted object (directly or indirectly) will be
*** deallocated. In the future, this may happen over time instead.
***
*** - Parameters:
***     - `FugaGC* gc`: The garbage collector.
*** - Returns: void.
*** - See also: `FugaGC_root`, `FugaGC_unroot`
**/
void FugaGC_collect(FugaGC *gc);

#endif
