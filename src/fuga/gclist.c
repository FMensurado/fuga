#include "gclist.h"
#include "test.h"

void FugaGCList_init(FugaGCList* list) {
    NEVER(list == NULL);
    list->next = list;
    list->prev = list;
} TESTSUITE(FugaGCList_init) {
    FugaGCList a;
    FugaGCList_init(&a);
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

void FugaGCList_pushBack(FugaGCList* list, FugaGCList* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->next = list;
    item->prev = list->prev;
    list->prev = item;
    item->prev->next = item;
} TESTSUITE(FugaGCList_pushBack) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    FugaGCList_pushBack(&a, &c);
    TEST(a.next == &b, "For 2 item lists, dummy->next should == item1");
    TEST(b.next == &c, "For 2 item lists, item1->next should == item2");
    TEST(c.next == &a, "For 2 item lists, item2->next should == dummy");
    TEST(a.prev == &c, "For 2 item lists, dummy->prev should == item2");
    TEST(c.prev == &b, "For 2 item lists, item2->prev should == item1");
    TEST(b.prev == &a, "For 2 item lists, item1->prev should == dummy");
}

void FugaGCList_pushFront(FugaGCList* list, FugaGCList* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->prev = list;
    item->next = list->next;
    list->next->prev = item;
    list->next = item;
} TESTSUITE(FugaGCList_pushFront) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushFront(&a, &c);
    TEST(a.next == &c, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &c, "For 1 item lists, dummy->prev should == item");
    TEST(c.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(c.prev == &a, "For 1 item lists, item->prev should == dummy");
    FugaGCList_pushFront(&a, &b);
    TEST(a.next == &b, "For 2 item lists, dummy->next should == item1");
    TEST(b.next == &c, "For 2 item lists, item1->next should == item2");
    TEST(c.next == &a, "For 2 item lists, item2->next should == dummy");
    TEST(a.prev == &c, "For 2 item lists, dummy->prev should == item2");
    TEST(c.prev == &b, "For 2 item lists, item2->prev should == item1");
    TEST(b.prev == &a, "For 2 item lists, item1->prev should == dummy");
}

void FugaGCList_unlink(FugaGCList* item) {
    NEVER(item == NULL);
    NEVER(item->next == item);
    item->prev->next = item->next;
    item->next->prev = item->prev;
} TESTSUITE(FugaGCList_unlink) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    FugaGCList_unlink(&c);
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    FugaGCList_unlink(&b);
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

FugaGCList* FugaGCList_popFront(FugaGCList* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    FugaGCList* item = list->next;
    FugaGCList_unlink(item);
    return item;
} TESTSUITE(FugaGCList_popFront) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    TEST(FugaGCList_popFront(&a) == &b,
        "Should return the first item (for 2 item list).");
    TEST(a.next == &c, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &c, "For 1 item lists, dummy->prev should == item");
    TEST(c.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(c.prev == &a, "For 1 item lists, item->prev should == dummy");
    TEST(FugaGCList_popFront(&a) == &c,
        "Should return the first item (for 1 item list).");
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

FugaGCList* FugaGCList_popBack(FugaGCList* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    FugaGCList* item = list->prev;
    FugaGCList_unlink(item);
    return item;
} TESTSUITE(FugaGCList_popBack) {
    FugaGCList a, b, c;
    FugaGCList_init(&a);
    FugaGCList_pushBack(&a, &b);
    FugaGCList_pushBack(&a, &c);
    TEST(FugaGCList_popBack(&a) == &c,
        "Should return the first item (for 2 item list).");
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    TEST(FugaGCList_popBack(&a) == &b,
        "Should return the first item (for 1 item list).");
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

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
} TESTSUITE(FugaGCList_appendFront) {
    FugaGCList d1, d2, i1, i2, i3;
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushBack(&d2, &i1);
    FugaGCList_pushBack(&d2, &i2);
    FugaGCList_pushBack(&d1, &i3);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d2.next == &d2, "src list not re-initialized correctly.");
    TEST(d2.prev == &d2, "src list not re-initialized correctly.");
    TEST(d1.next == &i1, "d1.next != &i1 -- but it should!");
    TEST(i1.next == &i2, "i1.next != &i2 -- but it should!");
    TEST(i2.next == &i3, "i2.next != &i3 -- but it should!");
    TEST(i3.next == &d1, "i3.next != &d1 -- but it should!");
    TEST(d1.prev == &i3, "d1.prev != &i3 -- but it should!");
    TEST(i3.prev == &i2, "i3.prev != &i2 -- but it should!");
    TEST(i2.prev == &i1, "i2.prev != &i1 -- but it should!");
    TEST(i1.prev == &d1, "i1.prev != &d1 -- but it should!");

    // now with empty lists
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &d1, "appending two empty lists - d.next should == d")
    TEST(d1.prev == &d1, "appending two empty lists - d.prev should == d")

    // now with a single list.
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1, "for single list, d.next != i -- but it should")
    TEST(d1.prev == &i1, "for single list, d.prev != i -- but it should")
    TEST(i1.next == &d1, "for single list, i.next != d -- but it should")
    TEST(i1.prev == &d1, "for single list, i.prev != d -- but it should")

    // now with an empty list on the right
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d1, &i2);
    FugaGCList_pushFront(&d1, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1, "failed at appending empty list on the right")
    TEST(d1.prev == &i2, "failed at appending empty list on the right")
    TEST(i1.next == &i2, "failed at appending empty list on the right")
    TEST(i1.prev == &d1, "failed at appending empty list on the right")
    TEST(i2.next == &d1, "failed at appending empty list on the right")
    TEST(i2.prev == &i1, "failed at appending empty list on the right")

    // now with an empty list on the left
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendFront(&d1, &d2);
    TEST(d1.next == &i1, "failed at appending empty list on the left")
    TEST(d1.prev == &i2, "failed at appending empty list on the left")
    TEST(i1.next == &i2, "failed at appending empty list on the left")
    TEST(i1.prev == &d1, "failed at appending empty list on the left")
    TEST(i2.next == &d1, "failed at appending empty list on the left")
    TEST(i2.prev == &i1, "failed at appending empty list on the lef")
}

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
} TESTSUITE(FugaGCList_appendBack) {
    FugaGCList d1, d2, i1, i2, i3;
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushBack(&d1, &i1);
    FugaGCList_pushBack(&d2, &i2);
    FugaGCList_pushBack(&d2, &i3);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d2.next == &d2, "src list not re-initialized correctly.");
    TEST(d2.prev == &d2, "src list not re-initialized correctly.");
    TEST(d1.next == &i1, "d1.next != &i1 -- but it should!");
    TEST(i1.next == &i2, "i1.next != &i2 -- but it should!");
    TEST(i2.next == &i3, "i2.next != &i3 -- but it should!");
    TEST(i3.next == &d1, "i3.next != &d1 -- but it should!");
    TEST(d1.prev == &i3, "d1.prev != &i3 -- but it should!");
    TEST(i3.prev == &i2, "i3.prev != &i2 -- but it should!");
    TEST(i2.prev == &i1, "i2.prev != &i1 -- but it should!");
    TEST(i1.prev == &d1, "i1.prev != &d1 -- but it should!");

    // now with empty lists
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &d1, "appending two empty lists - d.next should == d")
    TEST(d1.prev == &d1, "appending two empty lists - d.prev should == d")

    // now with a single list.
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1, "for single list, d.next != i -- but it should")
    TEST(d1.prev == &i1, "for single list, d.prev != i -- but it should")
    TEST(i1.next == &d1, "for single list, i.next != d -- but it should")
    TEST(i1.prev == &d1, "for single list, i.prev != d -- but it should")

    // now with an empty list on the right
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d1, &i2);
    FugaGCList_pushFront(&d1, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1, "failed at appending empty list on the right")
    TEST(d1.prev == &i2, "failed at appending empty list on the right")
    TEST(i1.next == &i2, "failed at appending empty list on the right")
    TEST(i1.prev == &d1, "failed at appending empty list on the right")
    TEST(i2.next == &d1, "failed at appending empty list on the right")
    TEST(i2.prev == &i1, "failed at appending empty list on the right")

    // now with an empty list on the left
    FugaGCList_init(&d1);
    FugaGCList_init(&d2);
    FugaGCList_pushFront(&d2, &i2);
    FugaGCList_pushFront(&d2, &i1);
    FugaGCList_appendBack(&d1, &d2);
    TEST(d1.next == &i1, "failed at appending empty list on the left")
    TEST(d1.prev == &i2, "failed at appending empty list on the left")
    TEST(i1.next == &i2, "failed at appending empty list on the left")
    TEST(i1.prev == &d1, "failed at appending empty list on the left")
    TEST(i2.next == &d1, "failed at appending empty list on the left")
    TEST(i2.prev == &i1, "failed at appending empty list on the lef")
}

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

bool FugaGCList_empty(FugaGCList *list) {
    NEVER((list->next == list) ^ (list->prev == list));
    return list->next == list;
} TESTSUITE(FugaGCList_empty) {
    FugaGCList d1, i1;
    FugaGCList_init(&d1);
    TEST( FugaGCList_empty(&d1), "empty list should be empty!");
    FugaGCList_pushBack(&d1, &i1);
    TEST(!FugaGCList_empty(&d1), "single list shouldn't be empty...");
}

bool FugaGCList_contains(FugaGCList *list, void* data) {
    FugaGCList *link;
    for(link = list->next; link != list; link = link->next) {
        if (link == data)
            return TRUE;
    }
    return FALSE;
} TESTSUITE(FugaGCList_contains) {
    FugaGCList d1, i1, i2;
    FugaGCList_init(&d1);

    TEST(!FugaGCList_contains(&d1, &i1), "empty list should be empty!");
    TEST(!FugaGCList_contains(&d1, &i2), "empty list should be empty!");
    TEST(!FugaGCList_contains(&d1, &d1),
         "empty list shouldn't contain itself!");
    FugaGCList_pushBack(&d1, &i1);
    TEST( FugaGCList_contains(&d1, &i1), "faulty contains for single list");
    TEST(!FugaGCList_contains(&d1, &i2), "faulty contains for single list");
    TEST(!FugaGCList_contains(&d1, &d1),
         "single list shouldn't contain itself!");
    FugaGCList_pushBack(&d1, &i2);
    TEST( FugaGCList_contains(&d1, &i1), "faulty contains for 2-list");
    TEST( FugaGCList_contains(&d1, &i2), "faulty contains for 2-list");
    TEST(!FugaGCList_contains(&d1, &d1), "list shouldn't contain itself!");
}

