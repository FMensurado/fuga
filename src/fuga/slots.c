
#include "slots.h"
#include "gc.h"
#include "test.h"
#include "fuga.h"


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
    FugaIndex index;
    FugaSlot slot;
};

/**
*** ### _FugaSlotsList_mark
***
*** Mark the list's dependencies. This is the FugaGCMarkFn for
*** the _FugaSlotsList.
**/
void _FugaSlotsList_mark(void* self) {
    _FugaSlotsList* list = self;
    FugaGC_mark(list, list->next);
    FugaGC_mark(list, list->slot.name);
    FugaGC_mark(list, list->slot.value);
}

/**
*** ### _FugaSlotsList_new
***
*** Return a new list, without any fields allocated.
**/
_FugaSlotsList* _FugaSlotsList_new(void* gc) {
    ALWAYS(gc);
    _FugaSlotsList *list = FugaGC_alloc(gc, sizeof(_FugaSlotsList));
    FugaGC_onMark(list, _FugaSlotsList_mark);
    return list;
}

/**
*** ### _FugaSlotsList_has
**/
bool _FugaSlotsList_has(_FugaSlotsList* list, FugaIndex index) {
    for (; list; list = list->next)
        if (list->index == index)
            return true;
    return false;
}

#ifdef TESTING
TESTS(_FugaSlotsList_has) {
    FugaGC *gc = FugaGC_start();
    
    _FugaSlotsList *a = _FugaSlotsList_new(gc);
    _FugaSlotsList *b = _FugaSlotsList_new(gc);

    TEST(!_FugaSlotsList_has(NULL, 0));
    TEST(!_FugaSlotsList_has(NULL, 1));

    a->next = NULL;
    a->index = 1;
    TEST(!_FugaSlotsList_has(a, 0));
    TEST(_FugaSlotsList_has(a, 1));

    b->next = NULL;
    b->index = 0;
    TEST(_FugaSlotsList_has(b, 0));
    TEST(!_FugaSlotsList_has(b, 1));
    
    b->next = a;
    TEST(_FugaSlotsList_has(b, 0));
    TEST(_FugaSlotsList_has(b, 1));
    TEST(!_FugaSlotsList_has(b, 2));

    FugaGC_end(gc);
}
#endif

/**
*** ### _FugaSlotsList_get
**/
FugaSlot* _FugaSlotsList_get(_FugaSlotsList* list, FugaIndex index) {
    for (; list; list = list->next)
        if (list->index == index)
            return &(list->slot);
    return NULL;
}

#ifdef TESTING
TESTS(_FugaSlotsList_get) {
    FugaGC *gc = FugaGC_start();
    
    _FugaSlotsList *a = _FugaSlotsList_new(gc);
    _FugaSlotsList *b = _FugaSlotsList_new(gc);

    TEST(_FugaSlotsList_get(NULL, 0) == NULL);
    TEST(_FugaSlotsList_get(NULL, 1) == NULL);

    a->next = NULL;
    a->index = 1;
    TEST(_FugaSlotsList_get(a, 0) == NULL);
    TEST(_FugaSlotsList_get(a, 1) == &a->slot);

    b->next = NULL;
    b->index = 0;
    TEST(_FugaSlotsList_get(b, 0) == &b->slot);
    TEST(_FugaSlotsList_get(b, 1) == NULL);
    
    b->next = a;
    TEST(_FugaSlotsList_get(b, 0) == &b->slot);
    TEST(_FugaSlotsList_get(b, 1) == &a->slot);
    TEST(_FugaSlotsList_get(b, 2) == NULL);

    FugaGC_end(gc);
}
#endif

/**
*** ### _FugaSlotsList_set
***
*** Return a new list with the given slot.
**/
_FugaSlotsList* _FugaSlotsList_set(
    _FugaSlotsList* oldlist,
    FugaIndex index,
    FugaSlot slot
) {
    ALWAYS(slot.name);
    ALWAYS(slot.value);

    _FugaSlotsList* newlist;
    
    for (newlist = oldlist; newlist; newlist = newlist->next) {
        if (newlist->index == index) {
            newlist->slot = slot;
            return oldlist;
        }
    }
    
    newlist = _FugaSlotsList_new(slot.name);
    newlist->next = oldlist;
    newlist->index = index;
    newlist->slot = slot;
    return newlist;
}

#ifdef TESTING
TESTS(_FugaSlotsList_set) {
    FugaGC *gc = FugaGC_start();

    FugaSlot slot = {(void*)gc,(void*)gc};

    FugaIndex ia = 0;
    FugaIndex ib = 1;
    FugaIndex ic = 2;

    _FugaSlotsList* a = NULL;
    _FugaSlotsList* b = _FugaSlotsList_set(a, ia, slot);
    _FugaSlotsList* c = _FugaSlotsList_set(b, ib, slot);
    _FugaSlotsList* d = _FugaSlotsList_set(c, ic, slot);
    _FugaSlotsList* e = _FugaSlotsList_set(d, ib, slot);

    TEST(b != NULL);
    TEST(_FugaSlotsList_has(b, ia));

    TEST(c != b);
    TEST(_FugaSlotsList_has(c, ia));
    TEST(_FugaSlotsList_has(c, ib));

    TEST(d != c);
    TEST(_FugaSlotsList_has(d, ia));
    TEST(_FugaSlotsList_has(d, ib));
    TEST(_FugaSlotsList_has(d, ic));

    TEST(e == d);

    FugaGC_end(gc);
}
#endif

/**
*** # FugaSlots
*** ### FugaSlots
**/
struct FugaSlots {
    size_t length;
    size_t capacity;
    FugaSlot* slots;
};


void _FugaSlots_free(void* _self) {
    FugaSlots* self = _self;
    free(self->slots);
}

void _FugaSlots_mark(void* _self) {
    FugaSlots* self = _self;
    for (FugaIndex i = 0; i < self->length; i++) {
        FugaGC_mark(self, self->slots[i].name);
        FugaGC_mark(self, self->slots[i].value);
        FugaGC_mark(self, self->slots[i].doc);
    }
}

FugaSlots* FugaSlots_new(void* gc) {
    ALWAYS(gc);
    FugaSlots* self = FugaGC_alloc(gc, sizeof(FugaSlots));
    self->length   = 0;
    self->capacity = 4;
    self->slots    = malloc(self->capacity * sizeof(FugaSlot));
    FugaGC_onMark(self, _FugaSlots_mark);
    FugaGC_onFree(self, _FugaSlots_free);
    return self;
}

/**
*** ## Properties
*** ### FugaSlots_length
***
*** Return the number of canonical slots. In other words, return the number
*** of contiguous indexed slots starting from 0.
**/
size_t FugaSlots_length(FugaSlots* self) {
    ALWAYS(self);
    return self->length;
}

/**
*** ## Has
*** ### FugaSlots_hasByIndex
***
*** Determine whether there is a slot with the given index.
**/
bool FugaSlots_hasByIndex(FugaSlots* self, FugaIndex index) {
    ALWAYS(self);
    return index < self->length;
}

/**
*** ### FugaSlots_hasBySymbol
***
*** Determine whether there is a slot with the give symbol.
**/
bool FugaSlots_hasBySymbol(FugaSlots* self, Fuga* symbol) {
    ALWAYS(self);
    ALWAYS(symbol);
    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_isTrue(Fuga_is(self->slots[i].name, symbol)))
            return true;
    }
    return false;
}

/**
*** ## Get
*** ### FugaSlots_getByIndex
***
*** Get the slot associated with a given index.
**/
FugaSlot* FugaSlots_getByIndex(FugaSlots* self, FugaIndex index) {
    ALWAYS(self);
    if (index < self->length)
        return &self->slots[index];
    else
        return NULL;
}

/**
*** ### FugaSlots_getBySymbol
***
*** Get the slot associated with a given symbol.
**/
FugaSlot* FugaSlots_getBySymbol(FugaSlots* self, Fuga* symbol) {
    ALWAYS(self);
    ALWAYS(symbol);
    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_isTrue(Fuga_is(self->slots[i].name, symbol)))
            return &self->slots[i];
    }
    return NULL;
}

/**
*** ## Append
***
*** Add a slot to the end.
**/
void FugaSlots_append(
    FugaSlots* self,
    FugaSlot   slot
) {
    ALWAYS(self);
    ALWAYS(slot.value);

    self->length++;
    if (self->capacity < self->length) {
        self->capacity *= 2;
        self->slots = realloc(self->slots, sizeof(FugaSlot)
                                         * self->capacity);
        ALWAYS(self->capacity >= self->length);
    }
    self->slots[self->length-1] = slot;
    self->slots[self->length-1].index = self->length-1;
}

#ifdef TESTING
TESTS(FugaSlots_append) {
    void* gc = FugaGC_start();
    FugaSlots* slots = FugaSlots_new(gc);
    FugaSlot slot = {
        .name = NULL,
        .value = FugaGC_alloc(gc, 4),
        .doc = NULL
    };

    TEST(slots->length == 0); TEST(slots->capacity == 4);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 1); TEST(slots->capacity == 4);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 2); TEST(slots->capacity == 4);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 3); TEST(slots->capacity == 4);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 4); TEST(slots->capacity == 4);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 5); TEST(slots->capacity == 8);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 6); TEST(slots->capacity == 8);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 7); TEST(slots->capacity == 8);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 8); TEST(slots->capacity == 8);
    FugaSlots_append(slots, slot);
    TEST(slots->length == 9); TEST(slots->capacity == 16);

    FugaGC_end(gc);
}
#endif


/**
*** ## Set
*** ### FugaSlots_setByIndex
***
*** Set or update the slot associated with a given index.
**/
void FugaSlots_setByIndex(
    FugaSlots* self,
    FugaIndex index,
    FugaSlot  slot
) {
    ALWAYS(self);
    ALWAYS(slot.value);
    ALWAYS(index <= self->length);

    if (index == self->length) {
        FugaSlots_append(self, slot); 
    } else {
        self->slots[index] = slot;
        self->slots[index].index = index;
    }
}

#ifdef TESTING
TESTS(FugaSlots_setByIndex) {
    FugaGC* gc = FugaGC_start();
    FugaSlots* slots = FugaSlots_new(gc);
    Fuga* value1 = FugaGC_alloc(gc, 1);
    Fuga* value2 = FugaGC_alloc(gc, 1);
    FugaSlot slot1 = {.name = NULL, .value = value1, .doc = NULL};
    FugaSlot slot2 = {.name = NULL, .value = value2, .doc = NULL};

    bool h;

    TEST(FugaSlots_length(slots) == 0);
    TEST(!FugaSlots_hasByIndex(slots, 0));
    TEST(FugaSlots_getByIndex(slots, 0) == NULL)
    FugaSlots_setByIndex(slots, 0, slot1);
    TEST(h = FugaSlots_hasByIndex(slots, 0));
    if (h) {
        TEST(FugaSlots_getByIndex(slots, 0)->value == value1)
    }
    TEST(FugaSlots_length(slots) == 1);

    TEST(!FugaSlots_hasByIndex(slots, 1));
    TEST(FugaSlots_getByIndex(slots, 1) == NULL);
    FugaSlots_setByIndex(slots, 1, slot2);
    TEST(h = FugaSlots_hasByIndex(slots, 1));
    if (h) {
        TEST(FugaSlots_getByIndex(slots, 1)->value == value2)
        FugaSlots_setByIndex(slots, 1, slot1);
        TEST(FugaSlots_getByIndex(slots, 1)->value == value1)
    }
    TEST(FugaSlots_length(slots) == 2);

    FugaSlots_setByIndex(slots, 2, slot1);
    TEST( FugaSlots_length(slots) == 3);
    FugaSlots_setByIndex(slots, 3, slot1);
    TEST( FugaSlots_length(slots) == 4);

    FugaGC_end(gc);
}
#endif

/**
*** ### FugaSlots_setBySymbol
***
*** Set or update the slot associated with a given symbol.
**/
void FugaSlots_setBySymbol(
    FugaSlots* self,
    Fuga* name,
    FugaSlot slot
) {
    ALWAYS(self);
    ALWAYS(slot.value);
    ALWAYS(name);

    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_isTrue(Fuga_is(self->slots[i].name, name))) {
            self->slots[i] = slot;
            self->slots[i].index = i;
            return;
        }
    }
    FugaSlots_append(self, slot);
}

#ifdef TESTING
TESTS(FugaSlots_setBySymbol) {
    Fuga* self = Fuga_init();
    Fuga* name1 = FUGA_SYMBOL("hello");
    Fuga* name2 = FUGA_SYMBOL("goodbye");
    Fuga* value1 = FugaGC_alloc(self, 4);
    Fuga* value2 = FugaGC_alloc(self, 4);
    FugaSlot slot1 = {.name = name1, .value = value1, .doc = NULL};
    FugaSlot slot2 = {.name = name2, .value = value2, .doc = NULL};
    bool h;

    FugaSlots* slots = FugaSlots_new(self);

    TEST(FugaSlots_length(slots) == 0);
    TEST(!FugaSlots_hasBySymbol(slots, name1));
    TEST( FugaSlots_getBySymbol(slots, name1) == NULL)
    FugaSlots_setBySymbol(slots, name1, slot1);
    TEST(h = FugaSlots_hasBySymbol(slots, name1));
    if (h) {
        TEST(FugaSlots_getBySymbol(slots, name1)->value == value1)
    }

    TEST(FugaSlots_length(slots) == 1);
    TEST(!FugaSlots_hasBySymbol(slots, name2));
    TEST( FugaSlots_getBySymbol(slots, name2) == NULL)
    FugaSlots_setBySymbol(slots, name2, slot2);
    TEST(h = FugaSlots_hasBySymbol(slots, name2));
    if (h) {
        TEST(FugaSlots_length(slots) == 2);
        TEST(FugaSlots_getBySymbol(slots, name2)->value == value2)
        slot2.value = value1;
        FugaSlots_setBySymbol(slots, name2, slot2);
        TEST(FugaSlots_getBySymbol(slots, name2)->value == value1);
        TEST(FugaSlots_length(slots) == 2);
    }

    TEST(h = FugaSlots_hasByIndex(slots, 0));
    if (h) {
        TEST(FugaSlots_getByIndex(slots, 0)->value == value1);
    }
    TEST(h = FugaSlots_hasByIndex(slots, 1));
    if (h) {
        TEST(FugaSlots_getByIndex(slots, 1)->value == value1);
    }   

    Fuga_quit(self);
}
#endif

