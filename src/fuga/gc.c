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

struct FugaGC {
    FugaGCList root;
    FugaGCList black;
    FugaGCList gray;
    FugaGCList white;
    size_t num_objects;
    size_t size;
    size_t pass;
};

typedef struct FugaGCHeader {
    FugaGCList list;
    FugaGCFreeFn freeFn;
    FugaGCMarkFn markFn;
    unsigned char pass;
    unsigned char root;
    size_t size;
    char data[];
} FugaGCHeader;

#define _FUGAGCHEADER(obj) ((FugaGCHeader*)((char*)(obj)-sizeof(FugaGCHeader)))

FugaGC* FugaGC_start() {
    FugaGC* gc = malloc(sizeof *gc);
    ALWAYS(gc);
    FugaGCList_init(&gc->root);
    FugaGCList_init(&gc->black);
    FugaGCList_init(&gc->gray);
    FugaGCList_init(&gc->white);
    gc->num_objects = 0;
    gc->size = sizeof *gc;
    gc->pass = 0;
    return gc;
} TESTSUITE(FugaGC_start) {
    FugaGC* gc = FugaGC_start();

    TEST(FugaGCList_empty(&gc->root), "FugaGC_start should initialize root list");
    TEST(FugaGCList_empty(&gc->black),"FugaGC_start should initialize black list");
    TEST(FugaGCList_empty(&gc->root), "FugaGC_start should initialize gray list");
    TEST(FugaGCList_empty(&gc->white),"FugaGC_start should initialize white list");

    TEST(gc->num_objects == 0, "gc should start without any objects");
    TEST(gc->size == sizeof(FugaGC),
        "gc->size should start out as the size of the gc structure");
    TEST(gc->pass == 0, "gc->pass should be initialized");

    FugaGC_end(gc);
}

void _FugaGCFreeFn_count(void* data) {
    // for testing purposes
    (**(int**)data)++;
    FugaGC_free(data);
}

void FugaGC_end(FugaGC* gc) {
    ALWAYS(gc);
    FugaGCList *link;

    FugaGCList_appendFront(&gc->white, &gc->root);
    FugaGCList_appendFront(&gc->white, &gc->black);
    FugaGCList_appendFront(&gc->white, &gc->gray);

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        FugaGCList_unlink(link);
        ((FugaGCHeader*)link)->freeFn(((FugaGCHeader*)link)->data);
    }

    free(gc);
} TESTSUITE(FugaGC_end) {
    int freed = 0;

    FugaGC* gc = FugaGC_start();
    *((int**)FugaGC_alloc(gc, sizeof(int**),
                          _FugaGCFreeFn_count, NULL)) = &freed;
    *((int**)FugaGC_alloc(gc, sizeof(int**),
                          _FugaGCFreeFn_count, NULL)) = &freed;
    *((int**)FugaGC_alloc(gc, sizeof(int**),
                          _FugaGCFreeFn_count, NULL)) = &freed;
    FugaGC_end(gc);
    TEST(freed == 3, "I've freed 3 objects so far... I hope.");

    gc = FugaGC_start();
    *((int**)FugaGC_alloc(gc, sizeof(int**),
                          _FugaGCFreeFn_count, NULL)) = &freed;
    *((int**)FugaGC_alloc(gc, sizeof(int**),
                          _FugaGCFreeFn_count, NULL)) = &freed;
    FugaGC_end(gc);
    TEST(freed == 5, "I've freed 5 objects so far... I hope.");
}



void FugaGCFreeFn_default(void* object) {
    ALWAYS(object);
    free(_FUGAGCHEADER(object));
} TESTSUITE(gc_default_freeFn) {
    FugaGC* gc = FugaGC_start();
    FugaGC_alloc(gc, 16, NULL, NULL);
    FugaGC_end(gc);
}

void FugaGC_free(void* object) {
    FugaGCFreeFn_default(object);
}

void FugaGCMarkFn_default(void* parent, FugaGC* gc) {
    ALWAYS(gc);
    ALWAYS(parent);
    // Not doing anything is the right default action.
}


void FugaGC_mark(FugaGC* gc, void* parent, void* child) {
    ALWAYS(gc);
    ALWAYS(parent);
    if (!child) return;

    FugaGCHeader *childh = _FUGAGCHEADER(child);
    if (childh->pass != gc->pass) {
        childh->pass = gc->pass;
        if (!childh->root) {
            FugaGCList_unlink(&childh->list);
            FugaGCList_pushFront(&gc->gray, &childh->list);
        }
    }
} TESTSUITE(FugaGC_mark) {

    FugaGC* gc = FugaGC_start();
    void* root = FugaGC_alloc(gc, 16, NULL, NULL);
    void* item = FugaGC_alloc(gc, 16, NULL, NULL);

    FugaGC_root(gc, root);

    TEST(FugaGCList_contains(&gc->root, _FUGAGCHEADER(root)),
        "FugaGC_root not working correctly");
    TEST(FugaGCList_contains(&gc->white, _FUGAGCHEADER(item)),
        "FugaGC_alloc puts object in the wrong gc list");
    
    FugaGC_mark(gc, root, root);
    FugaGC_mark(gc, root, item);

    TEST(FugaGCList_contains(&gc->root, _FUGAGCHEADER(root)),
       "gc->pass should have prevented change, and root should stay root");
    TEST(FugaGCList_contains(&gc->white, _FUGAGCHEADER(item)),
        "gc->pass should have prevented change in gc_lists");

    gc->pass += 1;
    FugaGC_mark(gc, root, root);
    FugaGC_mark(gc, root, item);

    TEST(FugaGCList_contains(&gc->root, _FUGAGCHEADER(root)),
        "root should stay root");
    TEST(FugaGCList_contains(&gc->gray, _FUGAGCHEADER(item)),
        "after marking, object should be in gray list");

    // Try marking NULL -- should handle it gracefully, not crash.
    FugaGC_mark(gc, root, NULL);

    FugaGC_end(gc);
}

void* FugaGC_alloc(
    FugaGC*        gc,
    size_t      size,
    FugaGCFreeFn freeFn,
    FugaGCMarkFn markFn
) {
    ALWAYS(gc);
    ALWAYS(size > 0);
    FugaGCHeader *header = malloc(sizeof *header + size);
    FugaGCList_pushFront(&gc->white, &header->list);
    header->freeFn = freeFn ? freeFn : FugaGCFreeFn_default;
    header->markFn = markFn ? markFn : FugaGCMarkFn_default;
    header->size   = size;
    header->pass   = gc->pass;
    header->root   = 0;
    gc->num_objects++;
    gc->size += sizeof *header + size;
    return header->data;
} TESTSUITE(FugaGC_alloc) {
    FugaGC* gc = FugaGC_start();
    size_t size = gc->size;
    
    FugaGC_alloc(gc, 16, NULL, NULL);
    TEST(gc->size == size + sizeof(FugaGCHeader) + 16,
         "gc->size calculation if faulty");
    size = gc->size;

    FugaGC_alloc(gc, 32, NULL, NULL);
    TEST(gc->size == size + sizeof(FugaGCHeader) + 32,
         "gc->size calculation if faulty");
    size = gc->size;
    
    FugaGCHeader *header = _FUGAGCHEADER(FugaGC_alloc(gc, 128, NULL, NULL));
    TEST(!header->root,
         "Objectes should be rooted explicitly, not implicitly.")

    FugaGC_end(gc);
}


void FugaGC_root(FugaGC* gc, void* data) {
    ALWAYS(gc);
    ALWAYS(data);
    FugaGCHeader *header = (void*)((char*)data - sizeof *header);
    header->root = TRUE;
    FugaGCList_unlink(&header->list);
    FugaGCList_pushBack(&gc->root, &header->list);
} TESTSUITE(FugaGC_root) {
    FugaGC* gc = FugaGC_start();

    void* item = FugaGC_alloc(gc, 8, NULL, NULL);

    TEST(!FugaGCList_contains(&gc->root, _FUGAGCHEADER(item)),
        "FugaGC_alloc shouldn't put objects in root list.");
    FugaGC_root(gc, item);
    TEST(FugaGCList_contains(&gc->root, _FUGAGCHEADER(item)),
        "FugaGC_root failed to move object to root list.");

    FugaGC_end(gc);
}

void FugaGC_unroot(FugaGC* gc, void* data) {
    ALWAYS(gc);
    ALWAYS(data);
    FugaGCHeader *header = (void*)((char*)data - sizeof *header);
    header->root = FALSE;
    FugaGCList_unlink(&header->list);
    FugaGCList_pushBack(&gc->white, &header->list);
} TESTSUITE(FugaGC_unroot) {
    FugaGC* gc = FugaGC_start();

    void* item = FugaGC_alloc(gc, 8, NULL, NULL);
    FugaGC_root(gc, item);
    FugaGC_unroot(gc, item);
    TEST(!FugaGCList_contains(&gc->root, _FUGAGCHEADER(item)),
        "FugaGC_unroot failed to move object back from root list.");

    FugaGC_end(gc);
}

// For testing purposes.
typedef struct _FugaGCDummy {
    void* other;
    size_t* freecount;
    size_t  markcount;
} _FugaGCDummy;
void _FugaGCFreeFn_dummy(void* data) {
    _FugaGCDummy* dummy = data;
    (*dummy->freecount)++;
    FugaGCFreeFn_default(data);
}
void _FugaGCMarkFn_dummy(void* data, FugaGC* gc) {
    _FugaGCDummy* dummy = data;
    dummy->markcount++;
    FugaGC_mark(gc, data, dummy->other);
}
_FugaGCDummy* _FugaGC_mkDummy(FugaGC* gc, size_t* freecount) {
    _FugaGCDummy* dummy = FugaGC_alloc(
        gc,
        sizeof*dummy,
        _FugaGCFreeFn_dummy,
        _FugaGCMarkFn_dummy
    );
    dummy->other     = NULL;
    dummy->markcount = 0;
    dummy->freecount = freecount;
    return dummy;
}

void FugaGC_collect(FugaGC* gc) {
    ALWAYS(gc);
    FugaGCList *link = gc->root.next;
    gc->pass++;

    FugaGCList_appendFront(&gc->white, &gc->black);
    FugaGCList_appendFront(&gc->white, &gc->gray);

    for(link = gc->root.next; link != &gc->root; link = link->next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        linkh->pass = gc->pass;
        linkh->markFn(linkh->data, gc);
    }

    for(link = gc->gray.next; link != &gc->gray; link = gc->gray.next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        linkh->pass = gc->pass;
        FugaGCList_unlink(link);
        FugaGCList_pushBack(&gc->black, link);
        linkh->markFn(linkh->data, gc);
    }

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        // update statistics
        gc->num_objects -= 1;
        gc->size -= linkh->size + sizeof(FugaGCHeader);
        // remove object
        FugaGCList_unlink(link);
        linkh->freeFn(linkh->data);
    }
} TESTSUITE(FugaGC_collect) {
    FugaGC* gc = FugaGC_start();

    size_t freecount;
    _FugaGCDummy *dummy1, *dummy2, *dummy3, *dummy4;

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    TEST(gc->num_objects == 3, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 3,
        "failed at keeping track of object size statistics");
    FugaGC_collect(gc);
    TEST(freecount == 3, "expected all 3 dummies to be freed"); 

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy3;
    dummy2->other = dummy3;
    FugaGC_root(gc, dummy1);
    TEST(gc->num_objects == 3, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 3,
        "failed at keeping track of object size statistics");
    FugaGC_collect(gc);
    TEST(freecount == 1, "expected only dummy2 to be freed"); 
    TEST(dummy1->markcount == 1, "expected dummy1 to be marked once");
    TEST(dummy3->markcount == 1, "expected dummy3 to be marked once");

    freecount = 0;
    FugaGC_unroot(gc, dummy1);
    TEST(gc->num_objects == 2, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 2,
        "failed at keeping track of object size statistics");
    FugaGC_collect(gc);
    TEST(freecount == 2, "expected the 2 remaining dummies to be freed");

    freecount = 0;
    for (int i = 0; i < 30; i++)
        _FugaGC_mkDummy(gc, &freecount);
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    for (int i = 0; i < 1010; i++)
        _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    for (int i = 0; i < 500010; i++)
        _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    for (int i = 0; i < 498950; i++)
        _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy3;
    FugaGC_root(gc, dummy1);
    TEST(gc->num_objects == 1000003, "object counting failure");
    TEST(gc->size == sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 1000003,
        "failed at keeping track of object size statistics");
    FugaGC_collect(gc);
    TEST(freecount == 1000000, "expected 1000000 dummies to be freed"); 
    TEST(dummy1->markcount == 1, "expected dummy1 to be marked once");
    TEST(dummy2->markcount == 1, "expected dummy2 to be marked once");
    TEST(dummy3->markcount == 1, "expected dummy3 to be marked once");

    freecount = 0;
    FugaGC_unroot(gc, dummy1);
    FugaGC_collect(gc);
    TEST(freecount == 3, "expected the 3 remaining dummies to be freed");

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy3;
    FugaGC_collect(gc);
    TEST(freecount == 3,
        "gc doesn't free circular references correctly when freeing"); 

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy4 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy4;
    dummy4->other = dummy4;
    FugaGC_root(gc, dummy1);
    FugaGC_root(gc, dummy3);
    FugaGC_collect(gc);
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
    FugaGC_unroot(gc, dummy1);
    FugaGC_unroot(gc, dummy3);
    FugaGC_collect(gc);
    TEST(freecount == 4, "expected remaining dummies to be freed");

    TEST(gc->num_objects == 0, "object counting failure");
    TEST(gc->size == sizeof *gc,
        "failed at keeping track of object size statistics");

    FugaGC_end(gc);
}

