#include "fuga.h"
#include "slots.h"
#include "test.h"
#include "gc.h"
#include "symbols.h"


/**
***
**/
void _Fuga_freeFn(void* _self) {
    Fuga* self = _self;
    switch (Fuga_type(self)) {
    case FUGA_TYPE_LONG:   case FUGA_TYPE_STRING:
    case FUGA_TYPE_SYMBOL: case FUGA_TYPE_OBJECT:
        if (self->data.data) free(self->data.data);
        break;
    }
    FugaGC_free(self);
}


void _Fuga_markFn(void* _self, FugaGC *gc) {
    Fuga* self = _self;
    switch (Fuga_type(self)) {
    case FUGA_TYPE_NONE:
    case FUGA_TYPE_INT: case FUGA_TYPE_LONG:
    case FUGA_TYPE_REAL: case FUGA_TYPE_STRING:
    case FUGA_TYPE_SYMBOL: case FUGA_TYPE_METHOD:
    case FUGA_TYPE_NIL: case FUGA_TYPE_TRUE:
    case FUGA_TYPE_FALSE:
        break;

    case FUGA_TYPE_OBJECT:
        FugaGC_mark(gc, self, self->data.OBJECT->symbols);
        FugaGC_mark(gc, self, FUGA_Prelude);
        FugaGC_mark(gc, self, FUGA_Int);
        FugaGC_mark(gc, self, FUGA_Real);
        FugaGC_mark(gc, self, FUGA_String);
        FugaGC_mark(gc, self, FUGA_Symbol);
        break;

    default:
        FugaGC_mark(gc, self, self->data.data);
    }
}

/**
*** ### Fuga_new
**/
Fuga* Fuga_new() {
    FugaGC* gc = FugaGC_start();
    Fuga* self = FugaGC_alloc(gc, sizeof(Fuga),
                              _Fuga_freeFn, _Fuga_markFn);
    
    self->Object = self;
    self->proto  = NULL;
    self->slots  = FugaSlots_new(gc);
    self->id     = FUGA_ID_OBJECT;
    self->_type  = FUGA_TYPE_OBJECT;
    self->size   = sizeof(FugaObject);
    
    self->data.OBJECT = calloc(sizeof(FugaObject), 1);
    self->data.OBJECT->gc      = gc;
    self->data.OBJECT->id      = FUGA_ID_OBJECT;
    self->data.OBJECT->symbols = FugaSymbols_new(gc);
    
    FUGA_Prelude = Fuga_clone(self);
    FUGA_Number  = Fuga_clone(self);
    FUGA_Int     = Fuga_clone(FUGA_Number);
    FUGA_Real    = Fuga_clone(FUGA_Number);
    FUGA_String  = Fuga_clone(self);
    FUGA_Symbol  = Fuga_clone(self);

    return self;
}

#ifdef TESTING
TESTS(Fuga_new) {
    Fuga* self = Fuga_new();

    TEST(self);
    TEST(FUGA_Object);
    TEST(FUGA_Object->id == FUGA_ID_OBJECT);
    TEST(FUGA_Object->data.OBJECT->gc);
    TEST(FUGA_Object->data.OBJECT->symbols);

    TEST(FUGA_Prelude);
    TEST(FUGA_Number);
    TEST(FUGA_Int);
    TEST(FUGA_Real);
    TEST(FUGA_String);
    TEST(FUGA_Symbol);
    
    Fuga_free(self);
}
#endif


/**
*** ### Fuga_free
**/
void Fuga_free(Fuga* self) {
    ALWAYS(self);
    FugaGC_end(FUGA_Object->data.OBJECT->gc);
}

/**
*** ## Prototyping
*** ### Fuga_clone
***
*** `Fuga_clone` creates an object, using the old object as a prototype.
*** The new object will invoke the prototype through delegation, so changes
*** to the prototype can have an effect on the new object. This is often
*** desirable.
***
*** - Params:
***     - `Fuga* proto`: the new object's prototype.
*** - Returns: the new object
**/
Fuga* Fuga_clone(Fuga* self) {
    ALWAYS(self);

    Fuga* result = FugaGC_alloc(FUGA_gc, sizeof(Fuga),
                                _Fuga_freeFn, _Fuga_markFn);

    result->Object = FUGA_Object;
    result->proto  = self;
    result->slots  = NULL;
    result->id     = ++FUGA_Object->data.OBJECT->id;
    result->_type  = FUGA_TYPE_NONE;
    result->size   = 0;
    result->data.data = NULL;

    return result;
}

#ifdef TESTING
TESTS(Fuga_clone) {
    Fuga* Object = Fuga_new();
    Fuga* self  = Fuga_clone(Object);
    Fuga* self2 = Fuga_clone(Object);
    Fuga* self3 = Fuga_clone(self);

    TEST(self != Object);

    TEST(self->proto == Object);
    TEST(self2->proto == Object);
    TEST(self3->proto == self);

    TEST(self->Object == Object);
    TEST(self2->Object == Object);
    TEST(self3->Object == Object);
    
    TEST(!self->slots);
    TEST(!self2->slots);
    TEST(!self3->slots);

    Fuga_free(Object);
}
#endif

/**
*** ## Properties
*** ### Fuga_length
***
*** Return the number of canonical slots in self. 
***
*** - Params:
***     - `Fuga* self`
*** - Returns: `
**/

/**
*** ### Fuga_type
***
*** Determine the primitive type of the object.
***
*** - Params:
***     - `Fuga* obj`
*** - Returns: the primitive type of `obj`
**/
#define Fuga_type(obj)  (((obj)->_type) & ~FUGA_FLAG_ERROR)

/**
*** ### Fuga_error
***
*** Determine whether an error was raised.
***
*** - Params:
***     - `Fuga* obj`
*** - Returns: true if `obj` is a raised error, false otherwise.
**/
#define Fuga_error(obj) (((obj)->_type) & FUGA_FLAG_ERROR)


/**
*** ## Slot Manipulation
*** ### Fuga_rawHas
**/

bool Fuga_rawHas(Fuga* self, Fuga* name)
{
    ALWAYS(self);
    switch (Fuga_type(name)) {
    case FUGA_TYPE_STRING: case FUGA_TYPE_MSG:
        return Fuga_rawHass(self, name->data.STRING);
    case FUGA_TYPE_INT:
        return Fuga_rawHasi(self, name->data.INT);
    case FUGA_TYPE_SYMBOL:
        return self->slots && FugaSlots_hasBySymbol(self->slots, name);
    default:
        return false; // should raise error :-(
    }
}

bool Fuga_rawHasi(Fuga* self, int64_t index) {
    ALWAYS(self);
    return self->slots && FugaSlots_hasByIndex(self->slots, index);
}

bool Fuga_rawHass(Fuga* self, const char* name) {
    ALWAYS(self);
    return false;
}


/**
*** ### Fuga_has
**/
bool Fuga_has(Fuga* self, Fuga* name) {
    ALWAYS(self);
    return false;
}

bool Fuga_hasi(Fuga* self, int64_t index) {
    ALWAYS(self);
    return Fuga_rawHasi(self, index) || (
        self->proto && Fuga_hasi(self, index)
    );
}

bool Fuga_hass(Fuga* self, const char* name) {
    ALWAYS(self);
    return false;
}

/**
*** ### Fuga_get
*** 
*** Return the value associated with a given name in a given object. If
*** there is no such slot, raise a SlotError.
***
*** - Params:
***     - `Fuga* self`: object to look in
***     - `name` or `index`: name or index of the slot to look for
*** - Returns: the value of the slot, or SlotError.
**/
Fuga* Fuga_get(Fuga* self, Fuga* name);
Fuga* Fuga_geti(Fuga* self, int64_t index);
Fuga* Fuga_gets(Fuga* self, const char* name);

/**
*** ### Fuga_set
***
*** Associate a value with a name in an object. If there is already such a
*** slot, or the index is too large, raise a SlotError.
***
*** - Params:
***     - `Fuga* self`: object to modify
***     - `name` or `index`: name of slot to add. If NULL, will insert
***     value in the first available index.
***     - `value`: the value of the slot to add.
*** - Returns: NULL on success, SlotError on failure.
**/

Fuga* Fuga_set(Fuga* self, Fuga* name, Fuga* value);
Fuga* Fuga_seti(Fuga* self, int64_t index, Fuga* value);
Fuga* Fuga_sets(Fuga* self, const char* name, Fuga* value);

