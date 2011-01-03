#include "gclist.h"
#include "test.h"

void FugaGCList_init(FugaGCList* list) {
    NEVER(list == NULL);
    list->next = list;
    list->prev = list;
}

#ifdef TESTING
TESTS(FugaGCList_init) {
    FugaGCList a;
    FugaGCList_init(&a);
    TEST(a.next == &a); 
    TEST(a.prev == &a);
}
#endif

void FugaGCList_pushBack(FugaGCList* list, FugaGCList* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->next = list;
    item->prev = list->prev;
    list->prev = item;
    item->prev->next = item;
}

#ifdef TESTING
TESTS(FugaGCList_pushBack) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    TEST(a.next == &b);
    TEST(a.prev == &b);
    TEST(b.next == &a);
    TEST(b.prev == &a);
    FugaGCList_pushBack(&a, &c);
    TEST(a.next == &b);
    TEST(b.next == &c);
    TEST(c.next == &a);
    TEST(a.prev == &c);
    TEST(c.prev == &b);
    TEST(b.prev == &a);
}
#endif

void FugaGCList_pushFront(FugaGCList* list, FugaGCList* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->prev = list;
    item->next = list->next;
    list->next->prev = item;
    list->next = item;
}

#ifdef TESTING
TESTS(FugaGCList_pushFront) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushFront(&a, &c);
    TEST(a.next == &c);
    TEST(a.prev == &c);
    TEST(c.next == &a);
    TEST(c.prev == &a);
    FugaGCList_pushFront(&a, &b);
    TEST(a.next == &b);
    TEST(b.next == &c);
    TEST(c.next == &a);
    TEST(a.prev == &c);
    TEST(c.prev == &b);
    TEST(b.prev == &a);
}
#endif

void FugaGCList_unlink(FugaGCList* item) {
    NEVER(item == NULL);
    NEVER(item->next == item);
    item->prev->next = item->next;
    item->next->prev = item->prev;
}

#ifdef TESTING
TESTS(FugaGCList_unlink) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    FugaGCList_unlink(&c);
    TEST(a.next == &b);
    TEST(a.prev == &b);
    TEST(b.next == &a);
    TEST(b.prev == &a);
    FugaGCList_unlink(&b);
    TEST(a.next == &a);
    TEST(a.prev == &a);
}
#endif

FugaGCList* FugaGCList_popFront(FugaGCList* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    FugaGCList* item = list->next;
    FugaGCList_unlink(item);
    return item;
}

#ifdef TESTING
TESTS(FugaGCList_popFront) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    TEST(FugaGCList_popFront(&a) == &b);
    TEST(a.next == &c);
    TEST(a.prev == &c);
    TEST(c.next == &a);
    TEST(c.prev == &a);
    TEST(FugaGCList_popFront(&a) == &c);
    TEST(a.next == &a);
    TEST(a.prev == &a);
}
#endif

FugaGCList* FugaGCList_popBack(FugaGCList* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    FugaGCList* item = list->prev;
    FugaGCList_unlink(item);
    return item;
}

#ifdef TESTING
TESTS(FugaGCList_popBack) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    TEST(FugaGCList_popBack(&a) == &c);
    TEST(a.next == &b);
    TEST(a.prev == &b);
    TEST(b.next == &a);
    TEST(b.prev == &a);
    TEST(FugaGCList_popBack(&a) == &b);
    TEST(a.next == &a);
    TEST(a.prev == &a);
}
#endif

void FugaGCList_appendFront (FugaGCList* dest, FugaGCList* src) {
    NEVER(dest == NULL);
    NEVER(src == NULL);
    NEVER(dest == src);
    if (src->next == src)
        return;
    src->next->prev = dest;
    src->prev->next = dest->next;
    dest->next->prev = src->prev;
    dest->next = src->next;
    FugaGCList_init(src);
}

#ifdef TESTING
TESTS(FugaGCList_appendFront) {
    FugaGCList d1, d2, i1, i2, i3;
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushBack(&d2, &i1);
    FugaGCList_pushBack(&d2, &i2);
    FugaGCList_pushBack(&d1, &i3);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d2.next == &d2);
    TEST(d2.prev == &d2);
    TEST(d1.next == &i1);
    TEST(i1.next == &i2);
    TEST(i2.next == &i3);
    TEST(i3.next == &d1);
    TEST(d1.prev == &i3);
    TEST(i3.prev == &i2);
    TEST(i2.prev == &i1);
    TEST(i1.prev == &d1);

    // now with empty lists
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &d1);
    TEST(d1.prev == &d1);

    // now with a single list.
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1);
    TEST(d1.prev == &i1);
    TEST(i1.next == &d1);
    TEST(i1.prev == &d1);

    // now with an empty list on the right
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d1, &i2);
    FugaGCList_pushFront(&d1, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1);
    TEST(d1.prev == &i2);
    TEST(i1.next == &i2);
    TEST(i1.prev == &d1);
    TEST(i2.next == &d1);
    TEST(i2.prev == &i1);

    // now with an empty list on the left
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1);
    TEST(d1.prev == &i2);
    TEST(i1.next == &i2);
    TEST(i1.prev == &d1);
    TEST(i2.next == &d1);
    TEST(i2.prev == &i1);
}
#endif

void FugaGCList_appendBack  (FugaGCList* dest, FugaGCList* src) {
    NEVER(dest == NULL);
    NEVER(src == NULL);
    NEVER(dest == src);
    if (src->next == src)
        return;
    src->prev->next = dest;
    src->next->prev = dest->prev;
    dest->prev->next = src->next;
    dest->prev = src->prev;
    FugaGCList_init(src);
}

#ifdef TESTING
TESTS(FugaGCList_appendBack) {
    FugaGCList d1, d2, i1, i2, i3;
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushBack(&d1, &i1);
    FugaGCList_pushBack(&d2, &i2);
    FugaGCList_pushBack(&d2, &i3);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d2.next == &d2);
    TEST(d2.prev == &d2);
    TEST(d1.next == &i1);
    TEST(i1.next == &i2);
    TEST(i2.next == &i3);
    TEST(i3.next == &d1);
    TEST(d1.prev == &i3);
    TEST(i3.prev == &i2);
    TEST(i2.prev == &i1);
    TEST(i1.prev == &d1);

    // now with empty lists
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &d1)
    TEST(d1.prev == &d1)

    // now with a single list.
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1)
    TEST(d1.prev == &i1)
    TEST(i1.next == &d1)
    TEST(i1.prev == &d1)

    // now with an empty list on the right
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d1, &i2);
    FugaGCList_pushFront(&d1, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1)
    TEST(d1.prev == &i2)
    TEST(i1.next == &i2)
    TEST(i1.prev == &d1)
    TEST(i2.next == &d1)
    TEST(i2.prev == &i1)

    // now with an empty list on the left
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1)
    TEST(d1.prev == &i2)
    TEST(i1.next == &i2)
    TEST(i1.prev == &d1)
    TEST(i2.next == &d1)
    TEST(i2.prev == &i1)
}
#endif

bool FugaGCList_empty(FugaGCList *list) {
    NEVER((list->next == list) ^ (list->prev == list));
    return list->next == list;
}

#ifdef TESTING
TESTS(FugaGCList_empty) {
    FugaGCList d1, i1;
    FugaGCList_init(&d1);
    TEST( FugaGCList_empty(&d1));
    FugaGCList_pushBack(&d1, &i1);
    TEST(!FugaGCList_empty(&d1));
}
#endif

bool FugaGCList_contains(FugaGCList *list, void* data) {
    FugaGCList *link;
    for(link = list->next; link != list; link = link->next) {
        if (link == data)
            return true;
    }
    return false;
}

#ifdef TESTING
TESTS(FugaGCList_contains) {
    FugaGCList d1, i1, i2;
    FugaGCList_init(&d1);

    TEST(!FugaGCList_contains(&d1, &i1));
    TEST(!FugaGCList_contains(&d1, &i2));
    TEST(!FugaGCList_contains(&d1, &d1));
    FugaGCList_pushBack(&d1, &i1);
    TEST( FugaGCList_contains(&d1, &i1));
    TEST(!FugaGCList_contains(&d1, &i2));
    TEST(!FugaGCList_contains(&d1, &d1));
    FugaGCList_pushBack(&d1, &i2);
    TEST( FugaGCList_contains(&d1, &i1));
    TEST( FugaGCList_contains(&d1, &i2));
    TEST(!FugaGCList_contains(&d1, &d1));
}
#endif

