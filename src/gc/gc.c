/*
** # Garbage Collection: The `gc` module.
**
** 
*/

#include "gc.h"
#include "../test/test.h"

#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif 

struct _gc_t {
    gc_list_t root;
    gc_list_t black;
    gc_list_t gray;
    gc_list_t white;
    size_t num_objects;
    size_t size;
    size_t pass;
};

typedef struct gc_header_t {
    gc_list_t list;
    gc_freeFn_t freeFn;
    gc_markFn_t markFn;
    unsigned char pass;
    unsigned char root;
    size_t size;
    char data[];
} gc_header_t;

#define _GC_HEADER(obj) ((gc_header_t*)((char*)(obj)-sizeof(gc_header_t)))

gc_t gc_start() {
    gc_t gc = malloc(sizeof *gc);
    ALWAYS(gc);
    gc_list_init(&gc->root);
    gc_list_init(&gc->black);
    gc_list_init(&gc->gray);
    gc_list_init(&gc->white);
    gc->num_objects = 0;
    gc->size = sizeof *gc;
    gc->pass = 0;
    return gc;
} TESTSUITE(gc_start) {
    gc_t gc = gc_start();

    TEST(gc_list_empty(&gc->root), "gc_start should initialize root list");
    TEST(gc_list_empty(&gc->black),"gc_start should initialize black list");
    TEST(gc_list_empty(&gc->root), "gc_start should initialize gray list");
    TEST(gc_list_empty(&gc->white),"gc_start should initialize white list");

    TEST(gc->num_objects == 0, "gc should start without any objects");
    TEST(gc->size == sizeof(struct _gc_t),
        "gc->size should start out as the size of the gc structure");
    TEST(gc->pass == 0, "gc->pass should be initialized");

    gc_end(gc);
}

void _gc_countFreeFn(void* data) {
    // for testing purposes
    (**(int**)data)++;
    gc_defaultFreeFn(data);
}

void gc_end(gc_t gc) {
    ALWAYS(gc);
    gc_list_t *link;

    gc_list_appendFront(&gc->white, &gc->root);
    gc_list_appendFront(&gc->white, &gc->black);
    gc_list_appendFront(&gc->white, &gc->gray);

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        gc_list_unlink(link);
        ((gc_header_t*)link)->freeFn(((gc_header_t*)link)->data);
    }

    free(gc);
} TESTSUITE(gc_end) {
    int freed = 0;

    gc_t gc = gc_start();
    *((int**)gc_alloc(gc, sizeof(int**), _gc_countFreeFn, NULL)) = &freed;
    *((int**)gc_alloc(gc, sizeof(int**), _gc_countFreeFn, NULL)) = &freed;
    *((int**)gc_alloc(gc, sizeof(int**), _gc_countFreeFn, NULL)) = &freed;
    gc_end(gc);
    TEST(freed == 3, "I've freed 3 objects so far... I hope.");

    gc = gc_start();
    *((int**)gc_alloc(gc, sizeof(int**), _gc_countFreeFn, NULL)) = &freed;
    *((int**)gc_alloc(gc, sizeof(int**), _gc_countFreeFn, NULL)) = &freed;
    gc_end(gc);
    TEST(freed == 5, "I've freed 5 objects so far... I hope.");
}



void gc_defaultFreeFn(void* object) {
    ALWAYS(object);
    free(_GC_HEADER(object));
} TESTSUITE(gc_default_freeFn) {
    gc_t gc = gc_start();
    gc_alloc(gc, 16, NULL, NULL);
    gc_end(gc);
}

void gc_defaultMarkFn(void* parent, gc_t gc) {
    ALWAYS(gc);
    ALWAYS(parent);
    // Not doing anything is the right default action.
}


void gc_mark(gc_t gc, void* parent, void* child) {
    ALWAYS(gc);
    ALWAYS(parent);
    if (!child) return;

    gc_header_t *childh = (void*)((char*)child - sizeof *childh);
    if (childh->pass != gc->pass) {
        childh->pass = gc->pass;
        if (!childh->root) {
            gc_list_unlink(&childh->list);
            gc_list_pushFront(&gc->gray, &childh->list);
        }
    }
} TESTSUITE(gc_mark) {

    gc_t gc = gc_start();
    void* root = gc_alloc(gc, 16, NULL, NULL);
    void* item = gc_alloc(gc, 16, NULL, NULL);

    gc_root(gc, root);

    TEST(gc_list_contains(&gc->root, _GC_HEADER(root)),
        "gc_root not working correctly");
    TEST(gc_list_contains(&gc->white, _GC_HEADER(item)),
        "gc_alloc puts object in the wrong gc list");
    
    gc_mark(gc, root, root);
    gc_mark(gc, root, item);

    TEST(gc_list_contains(&gc->root, _GC_HEADER(root)),
       "gc->pass should have prevented change, and root should stay root");
    TEST(gc_list_contains(&gc->white, _GC_HEADER(item)),
        "gc->pass should have prevented change in gc_lists");

    gc->pass += 1;
    gc_mark(gc, root, root);
    gc_mark(gc, root, item);

    TEST(gc_list_contains(&gc->root, _GC_HEADER(root)),
        "root should stay root");
    TEST(gc_list_contains(&gc->gray, _GC_HEADER(item)),
        "after marking, object should be in gray list");

    // Try marking NULL -- should handle it gracefully, not crash.
    gc_mark(gc, root, NULL);

    gc_end(gc);
}

void* gc_alloc(
    gc_t        gc,
    size_t      size,
    gc_freeFn_t freeFn,
    gc_markFn_t markFn
) {
    ALWAYS(gc);
    ALWAYS(size > 0);
    gc_header_t *header = malloc(sizeof *header + size);
    gc_list_pushFront(&gc->white, &header->list);
    header->freeFn = freeFn ? freeFn : gc_defaultFreeFn;
    header->markFn = markFn ? markFn : gc_defaultMarkFn;
    header->size   = size;
    header->pass   = gc->pass;
    gc->num_objects++;
    gc->size += sizeof *header + size;
    return header->data;
} TESTSUITE(gc_alloc) {
    gc_t gc = gc_start();
    size_t size = gc->size;
    
    gc_alloc(gc, 16, NULL, NULL);
    TEST(gc->size == size + sizeof(gc_header_t) + 16,
         "gc->size calculation if faulty");
    size = gc->size;

    gc_alloc(gc, 32, NULL, NULL);
    TEST(gc->size == size + sizeof(gc_header_t) + 32,
         "gc->size calculation if faulty");
    size = gc->size;

    gc_end(gc);
}


void gc_root(gc_t gc, void* data) {
    ALWAYS(gc);
    ALWAYS(data);
    gc_header_t *header = (void*)((char*)data - sizeof *header);
    header->root = TRUE;
    gc_list_unlink(&header->list);
    gc_list_pushBack(&gc->root, &header->list);
} TESTSUITE(gc_root) {
    gc_t gc = gc_start();

    void* item = gc_alloc(gc, 8, NULL, NULL);

    TEST(!gc_list_contains(&gc->root, _GC_HEADER(item)),
        "gc_alloc shouldn't put objects in root list.");
    gc_root(gc, item);
    TEST(gc_list_contains(&gc->root, _GC_HEADER(item)),
        "gc_root failed to move object to root list.");

    gc_end(gc);
}

void gc_unroot(gc_t gc, void* data) {
    ALWAYS(gc);
    ALWAYS(data);
    gc_header_t *header = (void*)((char*)data - sizeof *header);
    header->root = FALSE;
    gc_list_unlink(&header->list);
    gc_list_pushBack(&gc->white, &header->list);
} TESTSUITE(gc_unroot) {
    gc_t gc = gc_start();

    void* item = gc_alloc(gc, 8, NULL, NULL);
    gc_root(gc, item);
    gc_unroot(gc, item);
    TEST(!gc_list_contains(&gc->root, _GC_HEADER(item)),
        "gc_unroot failed to move object back from root list.");

    gc_end(gc);
}

// For testing purposes.
typedef struct __gc_dummy_t {
    void* other;
    size_t* freecount;
    size_t  markcount;
} *_gc_dummy_t;
void _gc_dummyFreeFn(void* data) {
    _gc_dummy_t dummy = data;
    (*dummy->freecount)++;
    gc_defaultFreeFn(data);
}
void _gc_dummyMarkFn(void* data, gc_t gc) {
    _gc_dummy_t dummy = data;
    dummy->markcount++;
    gc_mark(gc, data, dummy->other);
}
_gc_dummy_t _gc_mkDummy(gc_t gc, size_t* freecount) {
    _gc_dummy_t dummy = gc_alloc(
        gc,
        sizeof*dummy,
        _gc_dummyFreeFn,
        _gc_dummyMarkFn
    );
    dummy->other     = NULL;
    dummy->markcount = 0;
    dummy->freecount = freecount;
    return dummy;
}

void gc_collect(gc_t gc) {
    ALWAYS(gc);
    gc_list_t *link = gc->root.next;
    gc->pass++;

    gc_list_appendFront(&gc->white, &gc->black);
    gc_list_appendFront(&gc->white, &gc->gray);

    for(link = gc->root.next; link != &gc->root; link = link->next) {
        ((gc_header_t*)link)->pass = gc->pass;
        ((gc_header_t*)link)->markFn(((gc_header_t*)link)->data, gc);
    }

    for(link = gc->gray.next; link != &gc->gray; link = gc->gray.next) {
        ((gc_header_t*)link)->pass = gc->pass;
        gc_list_unlink(link);
        gc_list_pushBack(&gc->black, link);
        ((gc_header_t*)link)->markFn(((gc_header_t*)link)->data, gc);
    }

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        // update statistics
        gc->num_objects -= 1;
        gc->size -= ((gc_header_t*)link)->size + sizeof(gc_header_t);
        // remove object
        gc_list_unlink(link);
        ((gc_header_t*)link)->freeFn(((gc_header_t*)link)->data);
    }
} TESTSUITE(gc_collect) {
    gc_t gc = gc_start();

    size_t freecount;
    _gc_dummy_t dummy1, dummy2, dummy3, dummy4;

    freecount = 0;
    dummy1 = _gc_mkDummy(gc, &freecount);
    dummy2 = _gc_mkDummy(gc, &freecount);
    dummy3 = _gc_mkDummy(gc, &freecount);
    TEST(gc->num_objects == 3, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(gc_header_t) + sizeof *dummy1) * 3,
        "failed at keeping track of object size statistics");
    gc_collect(gc);
    TEST(freecount == 3, "expected all 3 dummies to be freed"); 

    freecount = 0;
    dummy1 = _gc_mkDummy(gc, &freecount);
    dummy2 = _gc_mkDummy(gc, &freecount);
    dummy3 = _gc_mkDummy(gc, &freecount);
    dummy1->other = dummy3;
    dummy2->other = dummy3;
    gc_root(gc, dummy1);
    TEST(gc->num_objects == 3, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(gc_header_t) + sizeof *dummy1) * 3,
        "failed at keeping track of object size statistics");
    gc_collect(gc);
    TEST(freecount == 1, "expected only dummy2 to be freed"); 
    TEST(dummy1->markcount == 1, "expected dummy1 to be marked once");
    TEST(dummy3->markcount == 1, "expected dummy3 to be marked once");

    freecount = 0;
    gc_unroot(gc, dummy1);
    TEST(gc->num_objects == 2, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(gc_header_t) + sizeof *dummy1) * 2,
        "failed at keeping track of object size statistics");
    gc_collect(gc);
    TEST(freecount == 2, "expected the 2 remaining dummies to be freed");

    freecount = 0;
    for (int i = 0; i < 30; i++)
        _gc_mkDummy(gc, &freecount);
    dummy1 = _gc_mkDummy(gc, &freecount);
    for (int i = 0; i < 1010; i++)
        _gc_mkDummy(gc, &freecount);
    dummy2 = _gc_mkDummy(gc, &freecount);
    for (int i = 0; i < 500010; i++)
        _gc_mkDummy(gc, &freecount);
    dummy3 = _gc_mkDummy(gc, &freecount);
    for (int i = 0; i < 498950; i++)
        _gc_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy3;
    gc_root(gc, dummy1);
    TEST(gc->num_objects == 1000003, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(gc_header_t) + sizeof *dummy1) * 1000003,
        "failed at keeping track of object size statistics");
    gc_collect(gc);
    TEST(freecount == 1000000, "expected 1000000 dummies to be freed"); 
    TEST(dummy1->markcount == 1, "expected dummy1 to be marked once");
    TEST(dummy2->markcount == 1, "expected dummy2 to be marked once");
    TEST(dummy3->markcount == 1, "expected dummy3 to be marked once");

    freecount = 0;
    gc_unroot(gc, dummy1);
    gc_collect(gc);
    TEST(freecount == 3, "expected the 3 remaining dummies to be freed");

    freecount = 0;
    dummy1 = _gc_mkDummy(gc, &freecount);
    dummy2 = _gc_mkDummy(gc, &freecount);
    dummy3 = _gc_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy3;
    gc_collect(gc);
    TEST(freecount == 3,
        "gc doesn't free circular references correctly when freeing"); 

    freecount = 0;
    dummy1 = _gc_mkDummy(gc, &freecount);
    dummy2 = _gc_mkDummy(gc, &freecount);
    dummy3 = _gc_mkDummy(gc, &freecount);
    dummy4 = _gc_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy4;
    dummy4->other = dummy4;
    gc_root(gc, dummy1);
    gc_root(gc, dummy3);
    gc_collect(gc);
    TEST(freecount == 0, 
        "gc doesn't free circular references correctly when freeing"); 
    TEST(dummy1->markcount == 1,
        "indirect circular references handled incorrectly when marking");
    TEST(dummy2->markcount == 1,
        "indirect circular references handled incorrectly when marking");
    TEST(dummy3->markcount == 1,
        "expected dummy3 to be marked only once");
    TEST(dummy4->markcount == 1,
        "direct circular references ghandled incorrectly when marking");

    freecount = 0;
    gc_unroot(gc, dummy1);
    gc_unroot(gc, dummy3);
    gc_collect(gc);
    TEST(freecount == 4, "expected remaining dummies to be freed");

    TEST(gc->num_objects == 0, "object counting failure");
    TEST(gc->size == sizeof *gc,
        "failed at keeping track of object size statistics");

    gc_end(gc);
}

