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
}

void gc_list_appendBack  (gc_list_t* dest, gc_list_t* src) {
}

