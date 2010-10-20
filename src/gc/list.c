#include "list.h"
#include "../test/test.h"

void gc_list_init(gc_list_t* list) {
    NEVER(list == NULL);
    list->next = list;
    list->prev = list;
} TESTSUITE(gc_list_init) {
    gc_list_t a;
    gc_list_init(&a);
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

void gc_list_pushBack(gc_list_t* list, gc_list_t* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->next = list;
    item->prev = list->prev;
    list->prev = item;
    item->prev->next = item;
} TESTSUITE(gc_list_pushBack) {
    gc_list_t a, b, c;
    gc_list_init(&a);
    gc_list_pushBack(&a, &b);
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    gc_list_pushBack(&a, &c);
    TEST(a.next == &b, "For 2 item lists, dummy->next should == item1");
    TEST(b.next == &c, "For 2 item lists, item1->next should == item2");
    TEST(c.next == &a, "For 2 item lists, item2->next should == dummy");
    TEST(a.prev == &c, "For 2 item lists, dummy->prev should == item2");
    TEST(c.prev == &b, "For 2 item lists, item2->prev should == item1");
    TEST(b.prev == &a, "For 2 item lists, item1->prev should == dummy");
}

void gc_list_pushFront(gc_list_t* list, gc_list_t* item) {
    NEVER(list == NULL);
    NEVER(item == NULL);
    item->prev = list;
    item->next = list->next;
    list->next->prev = item;
    list->next = item;
} TESTSUITE(gc_list_pushFront) {
    gc_list_t a, b, c;
    gc_list_init(&a);
    gc_list_pushFront(&a, &c);
    TEST(a.next == &c, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &c, "For 1 item lists, dummy->prev should == item");
    TEST(c.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(c.prev == &a, "For 1 item lists, item->prev should == dummy");
    gc_list_pushFront(&a, &b);
    TEST(a.next == &b, "For 2 item lists, dummy->next should == item1");
    TEST(b.next == &c, "For 2 item lists, item1->next should == item2");
    TEST(c.next == &a, "For 2 item lists, item2->next should == dummy");
    TEST(a.prev == &c, "For 2 item lists, dummy->prev should == item2");
    TEST(c.prev == &b, "For 2 item lists, item2->prev should == item1");
    TEST(b.prev == &a, "For 2 item lists, item1->prev should == dummy");
}

void gc_list_unlink(gc_list_t* item) {
    NEVER(item == NULL);
    NEVER(item->next == item);
    item->prev->next = item->next;
    item->next->prev = item->prev;
} TESTSUITE(gc_list_unlink) {
    gc_list_t a, b, c;
    gc_list_init(&a);
    gc_list_pushBack(&a, &b);
    gc_list_pushBack(&a, &c);
    gc_list_unlink(&c);
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    gc_list_unlink(&b);
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

gc_list_t* gc_list_popFront(gc_list_t* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    gc_list_t* item = list->next;
    gc_list_unlink(item);
    return item;
} TESTSUITE(gc_list_popFront) {
    gc_list_t a, b, c;
    gc_list_init(&a);
    gc_list_pushBack(&a, &b);
    gc_list_pushBack(&a, &c);
    TEST(gc_list_popFront(&a) == &b,
        "Should return the first item (for 2 item list).");
    TEST(a.next == &c, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &c, "For 1 item lists, dummy->prev should == item");
    TEST(c.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(c.prev == &a, "For 1 item lists, item->prev should == dummy");
    TEST(gc_list_popFront(&a) == &c,
        "Should return the first item (for 1 item list).");
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

gc_list_t* gc_list_popBack(gc_list_t* list) {
    NEVER(list == NULL);
    NEVER(list->next == list);
    gc_list_t* item = list->prev;
    gc_list_unlink(item);
    return item;
} TESTSUITE(gc_list_popBack) {
    gc_list_t a, b, c;
    gc_list_init(&a);
    gc_list_pushBack(&a, &b);
    gc_list_pushBack(&a, &c);
    TEST(gc_list_popBack(&a) == &c,
        "Should return the first item (for 2 item list).");
    TEST(a.next == &b, "For 1 item lists, dummy->next should == item");
    TEST(a.prev == &b, "For 1 item lists, dummy->prev should == item");
    TEST(b.next == &a, "For 1 item lists, item->next should == dummy");
    TEST(b.prev == &a, "For 1 item lists, item->prev should == dummy");
    TEST(gc_list_popBack(&a) == &b,
        "Should return the first item (for 1 item list).");
    TEST(a.next == &a, "For 0 item lists, dummy->next should == dummy.");
    TEST(a.prev == &a, "For 0 item lists, dummy->prev should == dummy.");
}

void gc_list_appendFront (gc_list_t* dest, gc_list_t* src) {
    NEVER(dest == NULL);
    NEVER(src == NULL);
    NEVER(dest == src);
    src->next->prev = dest;
    src->prev->next = dest->next;
    dest->next->prev = src->prev;
    dest->next = src->next->next->prev;
    gc_list_init(src);
} TESTSUITE(gc_list_appendFront) {
    gc_list_t d1, d2, i1, i2, i3;
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_pushBack(&d2, &i1);
    gc_list_pushBack(&d2, &i2);
    gc_list_pushBack(&d1, &i3);
    gc_list_appendFront(&d1, &d2);
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
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_appendFront(&d1, &d2);
    TEST(d1.next == &d1, "appending two empty lists - d.next should == d")
    TEST(d1.prev == &d1, "appending two empty lists - d.prev should == d")

    // now with a single list.
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_pushFront(&d2, &i1);
    gc_list_appendBack(&d1, &d2);
    TEST(d1.next == &i1, "for single list, d.next != i -- but it should")
    TEST(d1.prev == &i1, "for single list, d.prev != i -- but it should")
    TEST(i1.next == &d1, "for single list, i.next != d -- but it should")
    TEST(i1.prev == &d1, "for single list, i.prev != d -- but it should")
}

void gc_list_appendBack  (gc_list_t* dest, gc_list_t* src) {
    NEVER(dest == NULL);
    NEVER(src == NULL);
    NEVER(dest == src);
    src->prev->next = dest;
    src->next->prev = dest->prev;
    dest->prev->next = src->next;
    dest->prev = src->prev->prev->next;
    gc_list_init(src);
} TESTSUITE(gc_list_appendBack) {
    gc_list_t d1, d2, i1, i2, i3;
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_pushBack(&d1, &i1);
    gc_list_pushBack(&d2, &i2);
    gc_list_pushBack(&d2, &i3);
    gc_list_appendBack(&d1, &d2);
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
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_appendBack(&d1, &d2);
    TEST(d1.next == &d1, "appending two empty lists - d.next should == d")
    TEST(d1.prev == &d1, "appending two empty lists - d.prev should == d")

    // now with a single list.
    gc_list_init(&d1);
    gc_list_init(&d2);
    gc_list_pushFront(&d2, &i1);
    gc_list_appendBack(&d1, &d2);
    TEST(d1.next == &i1, "for single list, d.next != i -- but it should")
    TEST(d1.prev == &i1, "for single list, d.prev != i -- but it should")
    TEST(i1.next == &d1, "for single list, i.next != d -- but it should")
    TEST(i1.prev == &d1, "for single list, i.prev != d -- but it should")
}

