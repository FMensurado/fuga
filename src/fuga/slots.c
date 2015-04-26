
#include "slots.h"
#include "test.h"
#include "fuga.h"
#include "log.h"

/**
*** # FugaSlots
*** ### FugaSlots
**/
struct FugaSlots {
    size_t length;
    size_t capacity;
    FugaSlot* slots;
};

void FugaSlots_free(void* _self) {
    FugaSlots* self = _self;
    free(self->slots);
}

void FugaSlots_mark(void* _self) {
    FugaSlots* self = _self;
    for (FugaIndex i = 0; i < self->length; i++) {
        Fuga_mark_(self, self->slots[i].name);
        Fuga_mark_(self, self->slots[i].value);
        Fuga_mark_(self, self->slots[i].doc);
    }
}

FugaSlots* FugaSlots_new(void* self) {
    ALWAYS(self);
    FugaSlots* result = Fuga_clone_(FUGA->Object, sizeof(FugaSlots));
    result->length   = 0;
    result->capacity = 4;
    result->slots    = calloc(result->capacity, sizeof(FugaSlot));
    Fuga_onMark_(result, FugaSlots_mark);
    Fuga_onFree_(result, FugaSlots_free);
    FUGA_HEADER(result)->slots = result;
    return result;
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
bool FugaSlots_hasBySymbol(FugaSlots* self, void* symbol) {
    ALWAYS(self);
    ALWAYS(symbol);
    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_is_(self->slots[i].name, symbol))
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
FugaSlot* FugaSlots_getBySymbol(FugaSlots* self, void* symbol) {
    ALWAYS(self);
    ALWAYS(symbol);
    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_is_(self->slots[i].name, symbol))
            return &self->slots[i];
    }
    return NULL;
}

/**
*** ## Append
***
*** Add a slot to the end.
**/
void FugaSlots_append_(
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
TESTS(FugaSlots_append_) {
    void* self = Fuga_init();
    FugaSlots* slots = FugaSlots_new(self);
    FugaSlot slot = {
        .name = NULL,
        .value = FUGA_SYMBOL("FOO"),
        .doc = NULL
    };

    TEST(slots->length == 0); TEST(slots->capacity == 4);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 1); TEST(slots->capacity == 4);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 2); TEST(slots->capacity == 4);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 3); TEST(slots->capacity == 4);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 4); TEST(slots->capacity == 4);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 5); TEST(slots->capacity == 8);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 6); TEST(slots->capacity == 8);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 7); TEST(slots->capacity == 8);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 8); TEST(slots->capacity == 8);
    FugaSlots_append_(slots, slot);
    TEST(slots->length == 9); TEST(slots->capacity == 16);

    Fuga_quit(self);
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
    FugaLog_log2("SETSLOT", self, slot.value);

    if (index == self->length) {
        FugaSlots_append_(self, slot); 
    } else {
        self->slots[index] = slot;
        self->slots[index].index = index;
    }
}

#ifdef TESTING
TESTS(FugaSlots_setByIndex) {
    void* self = Fuga_init();
    FugaSlots* slots = FugaSlots_new(self);
    void* value1 = Fuga_clone(FUGA->Object);
    void* value2 = Fuga_clone(FUGA->Object);
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

    Fuga_quit(self);
}
#endif

/**
*** ### FugaSlots_setBySymbol
***
*** Set or update the slot associated with a given symbol.
**/
void FugaSlots_setBySymbol(
    FugaSlots* self,
    void* name,
    FugaSlot slot
) {
    ALWAYS(self);
    ALWAYS(slot.value);
    ALWAYS(name);
    FugaLog_log3("SETSLOT", self, name, slot.value);

    for (FugaIndex i = 0; i < self->length; i++) {
        if (self->slots[i].name &&
            Fuga_is_(self->slots[i].name, name)) {
            self->slots[i] = slot;
            self->slots[i].index = i;
            return;
        }
    }
    FugaSlots_append_(self, slot);
}

#ifdef TESTING
TESTS(FugaSlots_setBySymbol) {
    void* self = Fuga_init();
    void* name1 = FUGA_SYMBOL("hello");
    void* name2 = FUGA_SYMBOL("goodbye");
    void* value1 = Fuga_clone(FUGA->Object);
    void* value2 = Fuga_clone(FUGA->Object);
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


void FugaSlots_delByIndex(
    FugaSlots* self,
    FugaIndex index
) {
    ALWAYS(self);
    /* GCC WARNS Tautological compare:
     *   if ((index >= 0) && (index < self->length)) {
     * TODO remove comment.
     */
    FugaLog_log1("DELSLOT", self);
    if ((index < self->length)) {
        for(FugaIndex i = index+1; i < self->length; i++) {
            self->slots[i-1] = self->slots[i];
            self->slots[i-1].index = i-1;
        }
        self->length--;
    }
}

void FugaSlots_delBySymbol(
    FugaSlots*  self,
    void* symbol
) {
    ALWAYS(self); ALWAYS(symbol);
    FugaLog_log2("DELSLOT", self, symbol);
    FugaSlot* slot = FugaSlots_getBySymbol(self, symbol);
    if (slot) {
        FugaSlots_delByIndex(self, slot->index);
    }
}

