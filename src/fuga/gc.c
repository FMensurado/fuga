/**
*** # Garbage Collection
***
*** 
**/

#include "gc.h"
#include "test.h"

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
    FugaGC *gc;
    FugaGCFreeFn freeFn;
    FugaGCMarkFn markFn;
    unsigned char pass;
    unsigned char root;
    size_t size;
    char data[];
} FugaGCHeader;

#define HEADER(obj) ((FugaGCHeader*)((char*)(obj)-sizeof(FugaGCHeader)))
#define DATA(obj)   ((void*)(((FugaGCHeader*)(obj))->data))

FugaGC* FugaGC_start() {
    size_t gcsize = sizeof(FugaGCHeader) + sizeof(FugaGC);

    FugaGC* gc = DATA(calloc(gcsize, 1));
   
    FugaGCList_init(&gc->root);
    FugaGCList_init(&gc->black);
    FugaGCList_init(&gc->gray);
    FugaGCList_init(&gc->white);

    gc->num_objects = 0;
    gc->size = gcsize;
    gc->pass = 0;

    HEADER(gc)->gc = gc;

    return gc;
}

#ifdef TESTING
TESTS(FugaGC_start) {
    FugaGC* gc = FugaGC_start();

    TEST(FugaGCList_empty(&gc->root));
    TEST(FugaGCList_empty(&gc->black));
    TEST(FugaGCList_empty(&gc->root));
    TEST(FugaGCList_empty(&gc->white));

    TEST(HEADER(gc)->gc == gc);
    TEST(gc->num_objects == 0);
    TEST(gc->size == sizeof(FugaGC) + sizeof(FugaGCHeader));
    TEST(gc->pass == 0);

    FugaGC_end(gc);
}
#endif

void _FugaGCFreeFn_count(void* data) {
    // for testing purposes
    (**(int**)data)++;
}

void FugaGC_end(void* self) {
    ALWAYS(self);
    FugaGC *gc = HEADER(self)->gc;
    FugaGCList *link;

    FugaGCList_append_(&gc->white, &gc->root);
    FugaGCList_append_(&gc->white, &gc->black);
    FugaGCList_append_(&gc->white, &gc->gray);

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        FugaGCList_unlink(link);
        if (((FugaGCHeader*)link)->freeFn)
            ((FugaGCHeader*)link)->freeFn(((FugaGCHeader*)link)->data);
        free(link);
    }

    free(HEADER(gc));
}

#ifdef TESTING
TESTS(FugaGC_end) {
    int freed = 0;

    FugaGC* gc = FugaGC_start();
    for (int i = 0; i < 3; i++) {
        int** p = FugaGC_alloc_(gc, sizeof(int**));
        *p = &freed;
        FugaGC_onFree_(p, _FugaGCFreeFn_count);
    }
    FugaGC_end(gc);
    TEST(freed == 3);

    gc = FugaGC_start();
    for (int i = 0; i < 2; i++) {
        int** p = FugaGC_alloc_(gc, sizeof(int**));
        *p = &freed;
        FugaGC_onFree_(p, _FugaGCFreeFn_count);
    }
    FugaGC_end(gc);
    TEST(freed == 5);
}
#endif

void FugaGC_mark_(void* parent, void* child) {
    ALWAYS(parent);
    FugaGC* gc = HEADER(parent)->gc;
    if (!child) return;
    if (HEADER(child)->pass != gc->pass) {
        HEADER(child)->pass = gc->pass;
        if (!HEADER(child)->root) {
            FugaGCList_unlink(&HEADER(child)->list);
            FugaGCList_push_(&gc->gray, &HEADER(child)->list);
        }
    }
}

#ifdef TESTING
TESTS(FugaGC_mark_) {
    FugaGC* gc = FugaGC_start();
    void* root = FugaGC_alloc_(gc, 16);
    void* item = FugaGC_alloc_(gc, 16);

    FugaGC_root(root);

    TEST(FugaGCList_contains_(&gc->root, HEADER(root)));
    TEST(FugaGCList_contains_(&gc->white, HEADER(item)));
    
    FugaGC_mark_(root, root);
    FugaGC_mark_(root, item);

    TEST(FugaGCList_contains_(&gc->root, HEADER(root)));
    TEST(FugaGCList_contains_(&gc->white, HEADER(item)));

    gc->pass += 1;
    FugaGC_mark_(root, root);
    FugaGC_mark_(root, item);

    TEST(FugaGCList_contains_(&gc->root, HEADER(root)));
    TEST(FugaGCList_contains_(&gc->gray, HEADER(item)));

    // Try marking NULL -- should handle it gracefully, not crash.
    FugaGC_mark_(root, NULL);

    FugaGC_end(gc);
}
#endif

void FugaGC_onFree_(void* self, FugaGCFreeFn freeFn)
{
    ALWAYS(self);
    HEADER(self)->freeFn = freeFn;
}

void FugaGC_onMark_(void* self, FugaGCMarkFn markFn)
{
    ALWAYS(self);
    HEADER(self)->markFn = markFn;
}

void* FugaGC_alloc_(void* self, size_t size)
{
    ALWAYS(self);
    ALWAYS(size > 0);
    FugaGC* gc = HEADER(self)->gc;
    FugaGCHeader *header = calloc(sizeof *header + size, 1);
    FugaGCList_push_(&gc->white, &header->list);
    gc->num_objects++;
    header->freeFn = NULL;
    header->markFn = NULL;
    header->size   = size;
    header->pass   = gc->pass;
    header->root   = 0;
    header->gc     = gc;
    gc->size += sizeof *header + size;
    return header->data;
}

#ifdef TESTING
TESTS(FugaGC_alloc) {
    FugaGC* gc = FugaGC_start();
    size_t size = gc->size;
    
    FugaGC_alloc_(gc, 16);
    TEST(gc->size == size + sizeof(FugaGCHeader) + 16);
    size = gc->size;

    FugaGC_alloc_(gc, 32);
    TEST(gc->size == size + sizeof(FugaGCHeader) + 32);
    size = gc->size;
    
    FugaGCHeader *header = HEADER(FugaGC_alloc_(gc, 128));
    TEST(!header->root);

    FugaGC_end(gc);
}
#endif

void FugaGC_root(void* self) {
    ALWAYS(self);
    HEADER(self)->root = true;
    FugaGCList_unlink(&HEADER(self)->list);
    FugaGCList_push_(&HEADER(self)->gc->root, &HEADER(self)->list);
}

#ifdef TESTING
TESTS(FugaGC_root) {
    FugaGC* gc = FugaGC_start();
    void* item = FugaGC_alloc_(gc, 8);

    TEST(!FugaGCList_contains_(&gc->root, HEADER(item)));
    FugaGC_root(item);
    TEST( FugaGCList_contains_(&gc->root, HEADER(item)));

    FugaGC_end(gc);
}
#endif

void FugaGC_unroot(void* self) {
    ALWAYS(self);
    HEADER(self)->root = false;
    FugaGCList_unlink(&HEADER(self)->list);
    FugaGCList_push_(&HEADER(self)->gc->white, &HEADER(self)->list);
}

#ifdef TESTING
TESTS(FugaGC_unroot) {
    FugaGC* gc = FugaGC_start();

    void* item = FugaGC_alloc_(gc, 8);
    FugaGC_root(item);
    FugaGC_unroot(item);
    TEST(!FugaGCList_contains_(&gc->root, HEADER(item)));

    FugaGC_end(gc);
}
#endif

// For testing purposes.
typedef struct _FugaGCDummy {
    void* other;
    size_t* freecount;
    size_t  markcount;
} _FugaGCDummy;
void _FugaGCFreeFn_dummy(void* data) {
    _FugaGCDummy* dummy = data;
    (*dummy->freecount)++;
}
void _FugaGCMarkFn_dummy(void* data) {
    _FugaGCDummy* dummy = data;
    dummy->markcount++;
    FugaGC_mark_(dummy, dummy->other);
}
_FugaGCDummy* _FugaGC_mkDummy(void* gc, size_t* freecount) {
    _FugaGCDummy* dummy = FugaGC_alloc_(gc, sizeof *dummy);
    FugaGC_onFree_(dummy, _FugaGCFreeFn_dummy);
    FugaGC_onMark_(dummy, _FugaGCMarkFn_dummy);

    dummy->other     = NULL;
    dummy->markcount = 0;
    dummy->freecount = freecount;
    return dummy;
}

void FugaGC_collect(void* self) {
    ALWAYS(self);
    FugaGC* gc = HEADER(self)->gc;
    FugaGCList *link = gc->root.next;
    gc->pass++;

    FugaGCList_append_(&gc->white, &gc->black);
    FugaGCList_append_(&gc->white, &gc->gray);

    for(link = gc->root.next; link != &gc->root; link = link->next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        linkh->pass = gc->pass;
        if (linkh->markFn)
            linkh->markFn(linkh->data);
    }

    for(link = gc->gray.next; link != &gc->gray; link = gc->gray.next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        linkh->pass = gc->pass;
        FugaGCList_unlink(link);
        FugaGCList_push_(&gc->black, link);
        if (linkh->markFn)
            linkh->markFn(linkh->data);
    }

    for(link = gc->white.next; link != &gc->white; link = gc->white.next) {
        FugaGCHeader* linkh = (FugaGCHeader*)link;
        // update statistics
        gc->num_objects -= 1;
        gc->size -= linkh->size + sizeof(FugaGCHeader);
        // remove object
        FugaGCList_unlink(link);
        if (linkh->freeFn)
            linkh->freeFn(linkh->data);
        free(linkh);
    }
}

#ifdef TESTING
TESTS(FugaGC_collect) {
    FugaGC* gc = FugaGC_start();

    size_t freecount;
    _FugaGCDummy *dummy1, *dummy2, *dummy3, *dummy4;

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    TEST(gc->num_objects == 3);
    TEST(gc->size == sizeof(FugaGCHeader) + sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 3);
    FugaGC_collect(gc);
    TEST(freecount == 3); 

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy3;
    dummy2->other = dummy3;
    FugaGC_root(dummy1);
    TEST(gc->num_objects == 3);
    TEST(gc->size == sizeof(FugaGCHeader) + sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 3);
    FugaGC_collect(gc);
    TEST(freecount == 1); 
    TEST(dummy1->markcount == 1);
    TEST(dummy3->markcount == 1);

    freecount = 0;
    FugaGC_unroot(dummy1);
    TEST(gc->num_objects == 2);
    TEST(gc->size == sizeof(FugaGCHeader) + sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 2);
    FugaGC_collect(gc);
    TEST(freecount == 2);

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
    FugaGC_root(dummy1);
    TEST(gc->num_objects == 1000003);
    TEST(gc->size == sizeof(FugaGCHeader) + sizeof *gc +
        (sizeof(FugaGCHeader) + sizeof *dummy1) * 1000003);
    FugaGC_collect(gc);
    TEST(freecount == 1000000); 
    TEST(dummy1->markcount == 1);
    TEST(dummy2->markcount == 1);
    TEST(dummy3->markcount == 1);

    freecount = 0;
    FugaGC_unroot(dummy1);
    FugaGC_collect(gc);
    TEST(freecount == 3);

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy3;
    FugaGC_collect(gc);
    TEST(freecount == 3); 

    freecount = 0;
    dummy1 = _FugaGC_mkDummy(gc, &freecount);
    dummy2 = _FugaGC_mkDummy(gc, &freecount);
    dummy3 = _FugaGC_mkDummy(gc, &freecount);
    dummy4 = _FugaGC_mkDummy(gc, &freecount);
    dummy1->other = dummy2;
    dummy2->other = dummy1;
    dummy3->other = dummy4;
    dummy4->other = dummy4;
    FugaGC_root(dummy1);
    FugaGC_root(dummy3);
    FugaGC_collect(gc);
    TEST(freecount == 0); 
    TEST(dummy1->markcount == 1);
    TEST(dummy2->markcount == 1);
    TEST(dummy3->markcount == 1);
    TEST(dummy4->markcount == 1);

    freecount = 0;
    FugaGC_unroot(dummy1);
    FugaGC_unroot(dummy3);
    FugaGC_collect(gc);
    TEST(freecount == 4);

    TEST(gc->num_objects == 0);
    TEST(gc->size == sizeof(FugaGCHeader) + sizeof *gc);

    FugaGC_end(gc);
}
#endif


