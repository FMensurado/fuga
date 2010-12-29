
#include "slots.h"
#include "test.h"


/**
*** # The underlying data structure: an associative list
*** ### _FugaSlotsList
***
*** This is a highly inefficient way to implement slots! 'Tis an
*** an associative list. All operations are O(n), which can be problematic.
*** This is such a bad data structure to use that I will eventually force
*** myself to implement something a little nicer.
**/
typedef struct _FugaSlotsList _FugaSlotsList;
struct _FugaSlotsList {
    _FugaSlotsList* next;
    FugaSlotsIndex index;
    FugaSlot slot;
};

/**
*** ### _FugaSlotsList_mark
***
*** Mark the list's dependencies. This is the FugaGCMarkFn for
*** the _FugaSlotsList.
**/
void _FugaSlotsList_mark(void* self, FugaGC* gc) {
    _FugaSlotsList* list = self;
    if (list->next) FugaGC_mark(gc, list, list->next);
    FugaGC_mark(gc, list, list->slot.name);
    FugaGC_mark(gc, list, list->slot.value);
}

/**
*** ### _FugaSlotsList_new
***
*** Return a new list, without any fields allocated.
**/
_FugaSlotsList* _FugaSlotsList_new(FugaGC* gc) {
    ALWAYS(gc);
    return FugaGC_alloc(
        gc, sizeof(_FugaSlotsList),
        NULL, _FugaSlotsList_mark
    );
}

/**
*** ### _FugaSlotsList_has
**/
bool _FugaSlotsList_has(_FugaSlotsList* list, FugaSlotsIndex index) {
    for (; list; list = list->next)
        if (list->index == index)
            return true;
    return false;
} TESTSUITE(_FugaSlotsList_has) {
    FugaGC *gc = FugaGC_start();
    
    _FugaSlotsList *a = _FugaSlotsList_new(gc);
    _FugaSlotsList *b = _FugaSlotsList_new(gc);

    TEST(!_FugaSlotsList_has(NULL, FugaSlotsIndex_fromInt(0)),
         "Empty list should not contain 0.");
    TEST(!_FugaSlotsList_has(NULL, FugaSlotsIndex_fromInt(1)),
         "Empty list should not contain 1.");

    a->next = NULL;
    a->index = FugaSlotsIndex_fromInt(1);
    TEST(!_FugaSlotsList_has(a, FugaSlotsIndex_fromInt(0)),
         "list(1) should not contain 0.");
    TEST(_FugaSlotsList_has(a, FugaSlotsIndex_fromInt(1)),
         "list(1) should contain 1.");

    b->next = NULL;
    b->index = 0;
    TEST(_FugaSlotsList_has(b, FugaSlotsIndex_fromInt(0)),
         "list(0) should contain 0.");
    TEST(!_FugaSlotsList_has(b, FugaSlotsIndex_fromInt(1)),
         "list(1) should not contain 1.");
    
    b->next = a;
    TEST(_FugaSlotsList_has(b, FugaSlotsIndex_fromInt(0)),
         "list(0, 1) should contain 0.");
    TEST(_FugaSlotsList_has(b, FugaSlotsIndex_fromInt(1)),
         "list(0, 1) should contain 1.");
    TEST(!_FugaSlotsList_has(b, FugaSlotsIndex_fromInt(2)),
         "list(0, 1) should not contain 2.");

    FugaGC_end(gc);
}

/**
*** ### _FugaSlotsList_get
**/
FugaSlot* _FugaSlotsList_get(_FugaSlotsList* list, FugaSlotsIndex index) {
    for (; list; list = list->next)
        if (list->index == index)
            return &(list->slot);
    return NULL;
} TESTSUITE(_FugaSlotsList_get) {
    FugaGC *gc = FugaGC_start();
    
    _FugaSlotsList *a = _FugaSlotsList_new(gc);
    _FugaSlotsList *b = _FugaSlotsList_new(gc);

    TEST(_FugaSlotsList_get(NULL, FugaSlotsIndex_fromInt(0)) == NULL,
         "Empty list should not contain 0.");
    TEST(_FugaSlotsList_get(NULL, FugaSlotsIndex_fromInt(1)) == NULL,
         "Empty list should not contain 1.");

    a->next = NULL;
    a->index = FugaSlotsIndex_fromInt(1);
    TEST(_FugaSlotsList_get(a, FugaSlotsIndex_fromInt(0)) == NULL,
         "list(1) should not contain 0.");
    TEST(_FugaSlotsList_get(a, FugaSlotsIndex_fromInt(1)) == &a->slot,
         "list(1) should contain 1.");

    b->next = NULL;
    b->index = 0;
    TEST(_FugaSlotsList_get(b, FugaSlotsIndex_fromInt(0)) == &b->slot,
         "list(0) should contain 0.");
    TEST(_FugaSlotsList_get(b, FugaSlotsIndex_fromInt(1)) == NULL,
         "list(0) should not contain 1.");
    
    b->next = a;
    TEST(_FugaSlotsList_get(b, FugaSlotsIndex_fromInt(0)) == &b->slot,
         "list(0, 1) should contain 0.");
    TEST(_FugaSlotsList_get(b, FugaSlotsIndex_fromInt(1)) == &a->slot,
         "list(0, 1) should contain 1.");
    TEST(_FugaSlotsList_get(b, FugaSlotsIndex_fromInt(2)) == NULL,
         "list(0, 1) should not contain 2.");

    FugaGC_end(gc);
}


/**
*** ### _FugaSlotsList_set
***
*** Return a new list with the given slot.
**/
_FugaSlotsList* _FugaSlotsList_set(
    _FugaSlotsList* oldlist,
    FugaGC* gc,
    FugaSlotsIndex index,
    FugaSlot slot
) {
    ALWAYS(gc);

    _FugaSlotsList* newlist;
    
    for (newlist = oldlist; newlist; newlist = newlist->next) {
        if (newlist->index == index) {
            newlist->slot = slot;
            return oldlist;
        }
    }
    
    newlist = _FugaSlotsList_new(gc);
    newlist->next = oldlist;
    newlist->index = index;
    newlist->slot = slot;
    return newlist;
} TESTSUITE(_FugaSlotsList_set) {
    FugaGC *gc = FugaGC_start();

    FugaSlot slot = {NULL, NULL};

    FugaSlotsIndex ia = FugaSlotsIndex_fromInt(0);
    FugaSlotsIndex ib = FugaSlotsIndex_fromInt(1);
    FugaSlotsIndex ic = FugaSlotsIndex_fromInt(2);

    _FugaSlotsList* a = NULL;
    _FugaSlotsList* b = _FugaSlotsList_set(a, gc, ia, slot);
    _FugaSlotsList* c = _FugaSlotsList_set(b, gc, ib, slot);
    _FugaSlotsList* d = _FugaSlotsList_set(c, gc, ic, slot);
    _FugaSlotsList* e = _FugaSlotsList_set(d, gc, ib, slot);

    TEST(b != NULL, "Inserting into empty list must result in a "
                    "non-empty list.");
    TEST(_FugaSlotsList_has(b, ia), "New key should be present after "
                                    "insertion.");

    TEST(c != b, "Inserting a new key into list must result in a "
                 "new list.");
    TEST(_FugaSlotsList_has(c, ia), "Old key should be present after "
                                    "insertion.");
    TEST(_FugaSlotsList_has(c, ib), "New key should be present after "
                                    "insertion.");

    TEST(d != c, "Inserting a new key into list must result in a "
                 "new list.");
    TEST(_FugaSlotsList_has(d, ia), "Old key #1 should be present after "
                                    "insertion.");
    TEST(_FugaSlotsList_has(d, ib), "Old key #2 should be present after "
                                    "insertion.");
    TEST(_FugaSlotsList_has(d, ic), "New key should be present after "
                                    "insertion.");

    TEST(e == d, "Inserting an existing keyi into list must modify the "
                 "list, not create a new one");

    FugaGC_end(gc);
}

/**
*** # FugaSlots
*** ### FugaSlots
**/
struct FugaSlots {
    size_t length;
    struct _FugaSlotsList* byIndex;
    struct _FugaSlotsList* bySymbol;
};

/**
*** ### _FugaSlots_mark
***
*** Mark `FugaSlots`'s references.
**/
void _FugaSlots_mark(void* self, FugaGC *gc) {
    FugaSlots* slots = self;
    FugaGC_mark(gc, self, slots->byIndex);
    FugaGC_mark(gc, self, slots->bySymbol);
}

/**
*** ## Constructors
*** ### FugaSlots_new
***
*** Create an empty `FugaSlots`.
**/
FugaSlots* FugaSlots_new(FugaGC* gc) {
    ALWAYS(gc);
    FugaSlots* slots = FugaGC_alloc(gc, sizeof(FugaSlots),
                                    NULL, _FugaSlots_mark);
    slots->length = 0;
    slots->byIndex = NULL;
    slots->bySymbol = NULL;
    return slots;
}

/**
*** ## Properties
*** ### FugaSlots_length
***
*** Return the number of canonical slots. In other words, return the number
*** of contiguous indexed slots starting from 0.
**/
size_t FugaSlots_length(FugaSlots* slots) {
    ALWAYS(slots);
    return slots->length;
}

/**
*** ## Has
*** ### FugaSlots_hasByIndex
***
*** Determine whether there is a slot with the given index.
**/
bool FugaSlots_hasByIndex(FugaSlots* slots, FugaSlotsIndex index) {
    ALWAYS(slots);
    return _FugaSlotsList_has(slots->byIndex, index);
}

/**
*** ### FugaSlots_hasBySymbol
***
*** Determine whether there is a slot with the give symbol.
**/
bool FugaSlots_hasBySymbol(FugaSlots* slots, Fuga* symbol) {
    ALWAYS(slots);
    ALWAYS(symbol);
    FugaSlotsIndex index = FugaSlotsIndex_fromPtr(symbol);
    return _FugaSlotsList_has(slots->bySymbol, index);
}

/**
*** ## Get
*** ### FugaSlots_getByIndex
***
*** Get the slot associated with a given index.
**/
FugaSlot* FugaSlots_getByIndex(FugaSlots* slots, FugaSlotsIndex index) {
    ALWAYS(slots);
    return _FugaSlotsList_get(slots->byIndex, index);
}

/**
*** ### FugaSlots_getBySymbol
***
*** Get the slot associated with a given symbol.
**/
FugaSlot* FugaSlots_getBySymbol(FugaSlots* slots, Fuga* symbol) {
    ALWAYS(slots);
    ALWAYS(symbol);
    FugaSlotsIndex index = FugaSlotsIndex_fromPtr(symbol);
    return _FugaSlotsList_get(slots->bySymbol, index);
}

/**
*** ## Set
*** ### FugaSlots_setByIndex
***
*** Set or update the slot associated with a given index. You still
*** need to pass in a Fuga* object for the name because FugaSlots is
*** completely Fuga-agnostic. It doesn't know anything about Fuga
*** objects other than their addresses, and the fact that they need to
*** be garbage-collected, so FugaSlots can't fabricate its own Fuga
*** object for the name.
**/
void FugaSlots_setByIndex(
    FugaSlots* slots,
    FugaGC *gc,
    Fuga* name,
    Fuga* value,
    FugaSlotsIndex index
) {
    ALWAYS(slots);
    ALWAYS(gc);
    ALWAYS(name);
    ALWAYS(value);

    FugaSlot slot;
    slot.name = name;
    slot.value = value;

    slots->byIndex = _FugaSlotsList_set(slots->byIndex, gc, index, slot);
    if (FugaSlotsIndex_eq(index, FugaSlotsIndex_fromInt(slots->length))) {
        slots->length += 1;
        for (;;) {
            index = FugaSlotsIndex_fromInt(slots->length);
            if (!FugaSlots_hasByIndex(slots, index)) break;
            slots->length += 1;
        }
    }
} TESTSUITE(FugaSlots_setByIndex) {
    FugaGC* gc = FugaGC_start();
    FugaSlots* slots = FugaSlots_new(gc);
    Fuga* name  = FugaGC_alloc(gc, 1, NULL, NULL);
    Fuga* value1 = FugaGC_alloc(gc, 1, NULL, NULL);
    Fuga* value2 = FugaGC_alloc(gc, 1, NULL, NULL);

    FugaSlotsIndex i0 = FugaSlotsIndex_fromInt(0);
    FugaSlotsIndex i1 = FugaSlotsIndex_fromInt(1);
    FugaSlotsIndex i2 = FugaSlotsIndex_fromInt(2);
    FugaSlotsIndex i3 = FugaSlotsIndex_fromInt(3);

    bool h;

    TEST(FugaSlots_length(slots) == 0, "empty slots should have length 0.");
    TEST(!FugaSlots_hasByIndex(slots, i0),
         "slots should not have index before insertion.");
    TEST(FugaSlots_getByIndex(slots, i0) == NULL,
         "there should be no slot before insertion")
    FugaSlots_setByIndex(slots, gc, name, value1, i0);
    TEST(h = FugaSlots_hasByIndex(slots, i0),
         "slots should have symbol after insertion.");
    if (h) {
        TEST(FugaSlots_getByIndex(slots, i0)->value == value1,
             "slot should have the value assigned to it")
    }
    TEST(FugaSlots_length(slots) == 1, "(do) should have length 1.");

    TEST(!FugaSlots_hasByIndex(slots, i1),
         "slots should not have index before insertion.");
    TEST(FugaSlots_getByIndex(slots, i1) == NULL,
         "there should be no slot before insertion")
    FugaSlots_setByIndex(slots, gc, name, value2, i1);
    TEST(h = FugaSlots_hasByIndex(slots, i1),
         "slots should have symbol after insertion.");
    if (h) {
        TEST(FugaSlots_getByIndex(slots, i1)->value == value2,
             "slot should have the value assigned to it")
        FugaSlots_setByIndex(slots, gc, name, value1, i1);
        TEST(FugaSlots_getByIndex(slots, i1)->value == value1,
             "slot should have the new value assigned to it")
    }
    TEST(FugaSlots_length(slots) == 2, "(do, re) should have length 2.");

    FugaSlots_setByIndex(slots, gc, name, value1, i3);
    TEST(FugaSlots_length(slots) == 2, "(do, re, 3=mi) should have "
                                       "length 2.");
    FugaSlots_setByIndex(slots, gc, name, value1, i2);
    TEST(FugaSlots_length(slots) == 4, "(do, re, fa, mi) should have "
                                       "length 4.");

    FugaGC_end(gc);
}

/**
*** ### FugaSlots_setBySymbol
***
*** Set or update the slot associated with a given symbol.
**/
void FugaSlots_setBySymbol(
    FugaSlots* slots,
    FugaGC *gc,
    Fuga* name,
    Fuga* value
) {
    ALWAYS(slots);
    ALWAYS(gc);
    ALWAYS(name);
    ALWAYS(value);

    FugaSlot slot;
    slot.name = name;
    slot.value = value;

    FugaSlotsIndex index = FugaSlotsIndex_fromPtr(name);

    slots->bySymbol = _FugaSlotsList_set(slots->bySymbol, gc, index, slot);
} TESTSUITE(FugaSlots_setBySymbol) {
    FugaGC* gc = FugaGC_start();
    Fuga* name1 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* name2 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* value1 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* value2 = FugaGC_alloc(gc, 4, NULL, NULL);
    bool h;

    FugaSlots* slots = FugaSlots_new(gc);

    TEST(!FugaSlots_hasBySymbol(slots, name1),
         "slots should not have symbol before insertion.");
    TEST(FugaSlots_getBySymbol(slots, name1) == NULL,
         "there should be no slot before insertion")
    FugaSlots_setBySymbol(slots, gc, name1, value1);
    TEST(h = FugaSlots_hasBySymbol(slots, name1),
         "slots should have symbol after insertion.");
    if (h) {
        TEST(FugaSlots_getBySymbol(slots, name1)->value == value1,
             "slot should have the value assigned to it")
    }

    TEST(!FugaSlots_hasBySymbol(slots, name2),
         "slots should not have symbol before insertion.");
    TEST(FugaSlots_getBySymbol(slots, name2) == NULL,
         "there should be no slot before insertion")
    FugaSlots_setBySymbol(slots, gc, name2, value2);
    TEST(h = FugaSlots_hasBySymbol(slots, name2),
         "slots should have symbol after insertion.");
    if (h) {
        TEST(FugaSlots_getBySymbol(slots, name2)->value == value2,
             "slot should have the value assigned to it")

        FugaSlots_setBySymbol(slots, gc, name2, value1);
        TEST(FugaSlots_getBySymbol(slots, name2)->value == value1,
             "slot should have the new value assigned to it")
    }

    FugaGC_end(gc);
}

