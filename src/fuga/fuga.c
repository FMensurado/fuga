#include "fuga.h"
#include "slots.h"
#include "test.h"
#include "gc.h"
#include "symbols.h"
#include <string.h>

/**
*** This should only be used for LONG, STRING, OBJECT, or SYMBOL
**/
void _Fuga_freeFn(void* _self) {
    Fuga* self = _self;
    ALWAYS(self->type == FUGA_TYPE_LONG ||
           self->type == FUGA_TYPE_SYMBOL ||
           self->type == FUGA_TYPE_STRING ||
           self->type == FUGA_TYPE_OBJECT);
    free(self->data.data);
}

void _Fuga_markFn(void* _self) {
    Fuga* self = _self;
    FugaGC_mark(self, self->slots);


    switch (Fuga_type(self)) {
    case FUGA_TYPE_NONE:
    case FUGA_TYPE_INT: case FUGA_TYPE_LONG:
    case FUGA_TYPE_REAL: case FUGA_TYPE_STRING:
    case FUGA_TYPE_SYMBOL: case FUGA_TYPE_METHOD:
    case FUGA_TYPE_NIL: case FUGA_TYPE_TRUE:
    case FUGA_TYPE_FALSE:
        break;

    case FUGA_TYPE_OBJECT:
        FugaGC_mark(self, self->data.OBJECT->symbols);
        FugaGC_mark(self, FUGA_Prelude);
        FugaGC_mark(self, FUGA_Int);
        FugaGC_mark(self, FUGA_Real);
        FugaGC_mark(self, FUGA_String);
        FugaGC_mark(self, FUGA_Symbol);
        break;

    default:
        FugaGC_mark(self, self->data.data);
    }
}

void _Fuga_markFnMsg(void* _self) {
    Fuga* self = _self;
    FugaGC_mark(self, self->slots);
    FugaGC_mark(self, self->data.MSG);
}

void _Fuga_markFnObject(void* _self) {
    Fuga* self = _self;
    ALWAYS(self->type == FUGA_TYPE_OBJECT);
    FugaGC_mark(self, self->slots);
    FugaGC_mark(self, self->data.OBJECT->symbols);
    FugaGC_mark(self, FUGA_Prelude);
    FugaGC_mark(self, FUGA_Number);
    FugaGC_mark(self, FUGA_Int);
    FugaGC_mark(self, FUGA_Real);
    FugaGC_mark(self, FUGA_String);
    FugaGC_mark(self, FUGA_Symbol);
    FugaGC_mark(self, FUGA_Bool);
    FugaGC_mark(self, FUGA_true);
    FugaGC_mark(self, FUGA_false);
    FugaGC_mark(self, FUGA_nil);
}

/**
*** ### Fuga_new
**/
Fuga* Fuga_new() {
    FugaGC* gc = FugaGC_start();
    Fuga* self = FugaGC_alloc(gc, sizeof(Fuga));
    FugaGC_onFree(self, _Fuga_freeFn);
    FugaGC_onMark(self, _Fuga_markFnObject);
    
    self->Object = self;
    self->proto  = NULL;
    self->slots  = FugaSlots_new(gc);
    self->id     = FUGA_ID_OBJECT;
    self->type  = FUGA_TYPE_OBJECT;
    self->size   = sizeof(FugaObject);
    
    self->data.OBJECT = calloc(sizeof(FugaObject), 1);
    self->data.OBJECT->id      = FUGA_ID_OBJECT;
    self->data.OBJECT->symbols = FugaSymbols_new(gc);
    
    FUGA_Prelude = Fuga_clone(self);
    FUGA_Number  = Fuga_clone(self);
    FUGA_Int     = Fuga_clone(FUGA_Number);
    FUGA_Real    = Fuga_clone(FUGA_Number);
    FUGA_String  = Fuga_clone(self);
    FUGA_Symbol  = Fuga_clone(self);
    FUGA_Msg     = Fuga_clone(self);
    FUGA_Bool    = Fuga_clone(self);

    FUGA_true = Fuga_clone(FUGA_Bool);
    FUGA_true->type = FUGA_TYPE_TRUE;
    FUGA_true->data.BOOL = true;

    FUGA_false = Fuga_clone(FUGA_Bool);
    FUGA_false->type = FUGA_TYPE_FALSE;
    FUGA_false->data.BOOL = false;

    FUGA_nil = Fuga_clone(self);
    FUGA_nil->type = FUGA_TYPE_NIL;

    return self;
}

#ifdef TESTING
TESTS(Fuga_new) {
    Fuga* self = Fuga_new();

    TEST(self);
    TEST(FUGA_Object);
    TEST(FUGA_Object->id == FUGA_ID_OBJECT);
    TEST(FUGA_Object->data.OBJECT->symbols);

    TEST(FUGA_Prelude);
    TEST(FUGA_Number);
    TEST(FUGA_Int);
    TEST(FUGA_Real);
    TEST(FUGA_String);
    TEST(FUGA_Symbol);
    TEST(FUGA_Msg);

    TEST(FUGA_Bool);
    TEST(FUGA_true);  TEST(FUGA_true->proto  == FUGA_Bool);
    TEST(FUGA_false); TEST(FUGA_false->proto == FUGA_Bool);
    
    TEST(FUGA_nil);

    Fuga_free(self);
}
#endif


/**
*** ### Fuga_free
**/
void Fuga_free(Fuga* self) {
    ALWAYS(self);
    FugaGC_end(self);
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

    Fuga* result = FugaGC_alloc(self, sizeof(Fuga));
    FugaGC_onMark(result, _Fuga_markFn);

    result->Object = FUGA_Object;
    result->proto  = self;
    result->slots  = NULL;
    result->id     = ++FUGA_Object->data.OBJECT->id;
    result->type  = FUGA_TYPE_NONE;
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
*** # Primitives
**/

Fuga* Fuga_bool(Fuga* self, bool value) {
    ALWAYS(self);
    return value ? FUGA_true : FUGA_false;
}
#ifdef TESTING
TESTS(Fuga_bool) {
    Fuga *self = Fuga_new();
    TEST(FUGA_BOOL(true) == FUGA_true);
    TEST(FUGA_BOOL(false) == FUGA_false);
    Fuga_free(self);
}
#endif

Fuga* Fuga_int(Fuga* self, FugaIndex index) {    
    ALWAYS(self);
    Fuga* value = Fuga_clone(FUGA_Int);
    value->type = FUGA_TYPE_INT;
    value->data.INT = index;
    return value;
}
#ifdef TESTING
TESTS(Fuga_int) {
    Fuga *self = Fuga_new();

    Fuga *value = FUGA_INT(0);
    TEST(value);
    if (value) {
        TEST(value->type == FUGA_TYPE_INT);
        TEST(value->data.INT == 0);
    }

    value = FUGA_INT(10);
    TEST(value);
    if (value) {
        TEST(value->type == FUGA_TYPE_INT);
        TEST(value->data.INT == 10);
    }

    value = FUGA_INT(-1024);
    TEST(value);
    if (value) {
        TEST(value->type == FUGA_TYPE_INT);
        TEST(value->data.INT == -1024);
    }

    Fuga_free(self);
}
#endif

Fuga* Fuga_string(Fuga* self, const char* str) {
    ALWAYS(self);
    Fuga* value = Fuga_clone(FUGA_String);
    value->type = FUGA_TYPE_STRING;
    value->size = strlen(str)+1;
    value->data.STRING = malloc(value->size);
    memcpy(value->data.STRING, str, value->size);
    FugaGC_onFree(value, _Fuga_freeFn);
    return value;
}

Fuga* Fuga_symbol(Fuga* self, const char* str) {
    ALWAYS(self);
    Fuga* value = FugaSymbols_get(FUGA_Object->data.OBJECT->symbols, str);
    if (!value) {
        value = Fuga_clone(FUGA_Symbol);
        value->type = FUGA_TYPE_SYMBOL;
        value->size = strlen(str)+1;
        value->data.STRING = malloc(value->size);
        memcpy(value->data.STRING, str, value->size);
        FugaGC_onFree(value, _Fuga_freeFn);
        FugaSymbols_set(FUGA_Object->data.OBJECT->symbols, str, value);
    }
    return value;
}

#ifdef TESTING
TESTS(Fuga_symbol) {
    Fuga* self = Fuga_new();

    Fuga* foo = FUGA_SYMBOL("foo");

    TEST(foo);
    if (foo) {
        TEST(foo->proto == FUGA_Symbol);
        TEST(foo->type == FUGA_TYPE_SYMBOL);
        TEST(foo == FUGA_SYMBOL("foo"));
        TEST(foo == FUGA_SYMBOL("foo"));
        FUGA_SYMBOL("bar");
        TEST(foo == FUGA_SYMBOL("foo"));
    }

    Fuga_free(self);
}
#endif

Fuga* Fuga_msg(Fuga* self) {
    ALWAYS(self);
    Fuga* value = Fuga_clone(FUGA_Msg);
    FugaGC_onMark(value, _Fuga_markFnMsg);
    value->type = FUGA_TYPE_MSG;
    value->data.MSG = self;
    return value;
}

Fuga* Fuga_msg1(Fuga* self, Fuga* arg0) {
    ALWAYS(self); ALWAYS(arg0);
    Fuga* value = Fuga_clone(FUGA_Msg);
    FugaGC_onMark(value, _Fuga_markFnMsg);
    value->type = FUGA_TYPE_MSG;
    value->data.MSG = self;
    Fuga_setByIndex(value, 0, arg0);
    return value;
}

Fuga* Fuga_msg2(Fuga* self, Fuga* arg0, Fuga* arg1) {
    ALWAYS(self); ALWAYS(arg0); ALWAYS(arg1);
    Fuga* value = Fuga_clone(FUGA_Msg);
    FugaGC_onMark(value, _Fuga_markFnMsg);
    value->type = FUGA_TYPE_MSG;
    value->data.MSG = self;
    Fuga_setByIndex(value, 0, arg0);
    Fuga_setByIndex(value, 1, arg1);
    return value;
}

Fuga* Fuga_msg3(Fuga* self, Fuga* arg0, Fuga* arg1, Fuga* arg2) {
    ALWAYS(self); ALWAYS(arg0); ALWAYS(arg1); ALWAYS(arg2);
    Fuga* value = Fuga_clone(FUGA_Msg);
    FugaGC_onMark(value, _Fuga_markFnMsg);
    value->type = FUGA_TYPE_MSG;
    value->data.MSG = self;
    Fuga_setByIndex(value, 0, arg0);
    Fuga_setByIndex(value, 1, arg1);
    Fuga_setByIndex(value, 2, arg2);
    return value;
}

Fuga* Fuga_msg4(
    Fuga* self,
    Fuga* arg0,
    Fuga* arg1,
    Fuga* arg2,
    Fuga* arg3
) {
    ALWAYS(self);
    ALWAYS(arg0); ALWAYS(arg1); ALWAYS(arg2); ALWAYS(arg3);
    Fuga* value = Fuga_clone(FUGA_Msg);
    FugaGC_onMark(value, _Fuga_markFnMsg);
    value->type = FUGA_TYPE_MSG;
    value->data.MSG = self;
    Fuga_setByIndex(value, 0, arg0);
    Fuga_setByIndex(value, 1, arg1);
    Fuga_setByIndex(value, 2, arg2);
    Fuga_setByIndex(value, 3, arg3);
    return value;
}

#ifdef TESTING
TESTS(Fuga_msg) {
    Fuga* self = Fuga_new();

    TEST(FUGA_MSG("HELLO")->proto == FUGA_Msg);
    TEST(FUGA_MSG("HELLO")->type == FUGA_TYPE_MSG);
    TEST(FUGA_MSG("HELLO")->data.MSG->type == FUGA_TYPE_SYMBOL);

    Fuga_free(self);
}
#endif

Fuga* Fuga_raise(Fuga* self) {
    ALWAYS(self);
    FUGA_NEED(self);
    self->type |= FUGA_ERROR;
    return self;
}

/**
*** # Slot Operations
*** ## Has
**/
bool Fuga_rawHasByIndex(Fuga* self, FugaIndex index) {
    ALWAYS(self);
    ALWAYS(FUGA_READY(self));
    return self->slots && FugaSlots_hasByIndex(self->slots, index);
}

bool Fuga_hasByIndex(Fuga* self, FugaIndex index) {
    ALWAYS(FUGA_READY(self));
    return Fuga_rawHasByIndex(self, index);
}

bool Fuga_rawHasBySymbol(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    ALWAYS(FUGA_READY(self));
    ALWAYS(name->type == FUGA_TYPE_SYMBOL);
    return self->slots && FugaSlots_hasBySymbol(self->slots, name);
}

bool Fuga_hasBySymbol(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    ALWAYS(FUGA_READY(self));
    ALWAYS(name->type == FUGA_TYPE_SYMBOL);
    return Fuga_rawHasBySymbol(self, name) ||
        (self->proto && Fuga_hasBySymbol(self->proto, name));
}

bool Fuga_rawHasByString(Fuga* self, const char* str) {
    ALWAYS(self); ALWAYS(str);
    ALWAYS(FUGA_READY(self));
    return Fuga_rawHasBySymbol(self, FUGA_SYMBOL(str));
}

bool Fuga_hasByString(Fuga* self, const char* str) {
    ALWAYS(self); ALWAYS(str);
    ALWAYS(FUGA_READY(self));
    return Fuga_hasBySymbol(self, FUGA_SYMBOL(str));
}

Fuga* Fuga_rawHas(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    switch (name->type) {
    case FUGA_TYPE_SYMBOL:
        return FUGA_BOOL(Fuga_rawHasBySymbol(self, name));
    case FUGA_TYPE_MSG:
        return FUGA_BOOL(Fuga_rawHasBySymbol(self, name->data.MSG));
    case FUGA_TYPE_STRING:
        return FUGA_BOOL(Fuga_rawHasByString(self, name->data.STRING));
    case FUGA_TYPE_INT:
        return FUGA_BOOL(Fuga_rawHasByIndex(self, name->data.INT));
    case FUGA_TYPE_LONG:
        FUGA_RAISE(
            FUGA_MSG1("ValueError",
                FUGA_STRING("rawHas: index is too positive or"
                            " too negative.")
            )
        );
    default:
        FUGA_RAISE(
            FUGA_MSG1("TypeError",
                FUGA_STRING("rawHas: name must be a primitive symbol,"
                            " msg, string, or int.")
            )
        );
    }
}

Fuga* Fuga_has(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    switch (name->type) {
    case FUGA_TYPE_SYMBOL:
        return FUGA_BOOL(Fuga_hasBySymbol(self, name));
    case FUGA_TYPE_MSG:
        return FUGA_BOOL(Fuga_hasBySymbol(self, name->data.MSG));
    case FUGA_TYPE_STRING:
        return FUGA_BOOL(Fuga_hasByString(self, name->data.STRING));
    case FUGA_TYPE_INT:
        return FUGA_BOOL(Fuga_hasByIndex(self, name->data.INT));
    case FUGA_TYPE_LONG:
        FUGA_RAISE(
            FUGA_MSG1("ValueError",
                FUGA_STRING("has: index is too positive or too negative.")
            )
        );
    default:
        FUGA_RAISE(
            FUGA_MSG1("TypeError",
                FUGA_STRING("has: name must be a primitive symbol,"
                            " msg, string, or int.")
            )
        );
    }
}


#ifdef TESTING
TESTS(Fuga_has) {
    Fuga* self = Fuga_new();
    Fuga* obj1 = Fuga_clone(FUGA_Object);
    Fuga* obj2 = Fuga_clone(obj1);
    Fuga* iobj1 = Fuga_clone(FUGA_Object);
    Fuga* iobj2 = Fuga_clone(iobj1);

    Fuga_setByIndex  (iobj1, 0, iobj1);
    Fuga_setByString (obj1, "foo", obj1);
    Fuga_setByString (obj2, "bar", obj1);
    Fuga_setBySymbol (obj1, FUGA_SYMBOL("do"), obj1);
    Fuga_setBySymbol (obj2, FUGA_SYMBOL("re"), obj1);

    TEST( Fuga_rawHasByIndex  (iobj1, 0));
    TEST(!Fuga_rawHasByIndex  (iobj2, 0));
    TEST(!Fuga_rawHasByIndex  (iobj1, 1));
    TEST(!Fuga_rawHasByIndex  (iobj2, 1));
    TEST( Fuga_rawHasByString (obj1, "foo"));
    TEST(!Fuga_rawHasByString (obj2, "foo"));
    TEST(!Fuga_rawHasByString (obj1, "bar"));
    TEST( Fuga_rawHasByString (obj2, "bar"));
    TEST(!Fuga_rawHasByString (obj1, "baz"));
    TEST(!Fuga_rawHasByString (obj2, "baz"));
    TEST( Fuga_rawHasBySymbol (obj1, FUGA_SYMBOL("do")));
    TEST(!Fuga_rawHasBySymbol (obj2, FUGA_SYMBOL("do")));
    TEST(!Fuga_rawHasBySymbol (obj1, FUGA_SYMBOL("re")));
    TEST( Fuga_rawHasBySymbol (obj2, FUGA_SYMBOL("re")));
    TEST(!Fuga_rawHasBySymbol (obj1, FUGA_SYMBOL("mi")));
    TEST(!Fuga_rawHasBySymbol (obj2, FUGA_SYMBOL("mi")));

    TEST(Fuga_isTrue (Fuga_rawHas(obj1, FUGA_STRING("foo"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_STRING("foo"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_STRING("bar"))));
    TEST(Fuga_isTrue (Fuga_rawHas(obj2, FUGA_STRING("bar"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_STRING("baz"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_STRING("baz"))));
    TEST(Fuga_isTrue (Fuga_rawHas(obj1, FUGA_SYMBOL("do"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_SYMBOL("do"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_SYMBOL("re"))));
    TEST(Fuga_isTrue (Fuga_rawHas(obj2, FUGA_SYMBOL("re"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_SYMBOL("mi"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_SYMBOL("mi"))));
    TEST(Fuga_isTrue (Fuga_rawHas(obj1, FUGA_MSG("do"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_MSG("do"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_MSG("re"))));
    TEST(Fuga_isTrue (Fuga_rawHas(obj2, FUGA_MSG("re"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj1, FUGA_MSG("mi"))));
    TEST(Fuga_isFalse(Fuga_rawHas(obj2, FUGA_MSG("mi"))));
    TEST(Fuga_isError(Fuga_rawHas(obj1, obj1)));
    TEST(Fuga_isError(Fuga_rawHas(obj1, FUGA_Object)));

    TEST( Fuga_hasByString (obj1, "foo"));
    TEST( Fuga_hasByString (obj2, "foo"));
    TEST(!Fuga_hasByString (obj1, "bar"));
    TEST( Fuga_hasByString (obj2, "bar"));
    TEST(!Fuga_hasByString (obj1, "baz"));
    TEST(!Fuga_hasByString (obj2, "baz"));
    TEST( Fuga_hasBySymbol (obj1, FUGA_SYMBOL("do")));
    TEST( Fuga_hasBySymbol (obj2, FUGA_SYMBOL("do")));
    TEST(!Fuga_hasBySymbol (obj1, FUGA_SYMBOL("re")));
    TEST( Fuga_hasBySymbol (obj2, FUGA_SYMBOL("re")));
    TEST(!Fuga_hasBySymbol (obj1, FUGA_SYMBOL("mi")));
    TEST(!Fuga_hasBySymbol (obj2, FUGA_SYMBOL("mi")));

    TEST(Fuga_isTrue (Fuga_has(obj1, FUGA_STRING("foo"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_STRING("foo"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_STRING("bar"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_STRING("bar"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_STRING("baz"))));
    TEST(Fuga_isFalse(Fuga_has(obj2, FUGA_STRING("baz"))));
    TEST(Fuga_isTrue (Fuga_has(obj1, FUGA_SYMBOL("do"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_SYMBOL("do"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_SYMBOL("re"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_SYMBOL("re"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_SYMBOL("mi"))));
    TEST(Fuga_isFalse(Fuga_has(obj2, FUGA_SYMBOL("mi"))));
    TEST(Fuga_isTrue (Fuga_has(obj1, FUGA_MSG("do"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_MSG("do"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_MSG("re"))));
    TEST(Fuga_isTrue (Fuga_has(obj2, FUGA_MSG("re"))));
    TEST(Fuga_isFalse(Fuga_has(obj1, FUGA_MSG("mi"))));
    TEST(Fuga_isFalse(Fuga_has(obj2, FUGA_MSG("mi"))));
    TEST(Fuga_isError(Fuga_rawHas(obj1, obj1)));
    TEST(Fuga_isError(Fuga_rawHas(obj1, FUGA_Object)));

    Fuga_free(self);
}
#endif


/**
*** ## Get
**/

Fuga* Fuga_rawGetByIndex(Fuga* self, FugaIndex index) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (self->slots) {
        FugaSlot* slot = FugaSlots_getByIndex(self->slots, index);
        if (slot)
            return slot->value;
    }
    FUGA_RAISE(
        FUGA_MSG2("SlotError",
            FUGA_STRING("rawGet: no slot with index"),
            FUGA_INT(index)
        )
    );
}

Fuga* Fuga_rawGetBySymbol(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    if (self->slots) {
        FugaSlot* slot = FugaSlots_getBySymbol(self->slots, name);
        if (slot)
            return slot->value;
    }
    FUGA_RAISE(
        FUGA_MSG2("SlotError",
            FUGA_STRING("rawGet: no slot with name"),
            name 
        )
    );
}

Fuga* Fuga_rawGetByString(Fuga* self, const char* str) {
    ALWAYS(self); ALWAYS(str);
    FUGA_NEED(self);
    return Fuga_rawGetBySymbol(self, FUGA_SYMBOL(str));
}

Fuga* Fuga_rawGet(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    switch (name->type) {
    case FUGA_TYPE_SYMBOL:
        return Fuga_rawGetBySymbol(self, name);
    case FUGA_TYPE_MSG:
        return Fuga_rawGetBySymbol(self, name->data.MSG);
    case FUGA_TYPE_STRING:
        return Fuga_rawGetByString(self, name->data.STRING);
    case FUGA_TYPE_INT:
        return Fuga_rawGetByIndex(self, name->data.INT);
    case FUGA_TYPE_LONG:
        FUGA_RAISE(
            FUGA_MSG1("ValueError",
                FUGA_STRING("rawGet: index is too positive"
                            " or too negative.")
            )
        );
    default:
        FUGA_RAISE(
            FUGA_MSG1("TypeError",
                FUGA_STRING("rawGet: name must be a primitive symbol,"
                            " msg, string, or int.")
            )
        );
    }
}

Fuga* Fuga_getByIndex(Fuga* self, FugaIndex index) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (self->slots) {
        FugaSlot* slot = FugaSlots_getByIndex(self->slots, index);
        if (slot)
            return slot->value;
    }
    FUGA_RAISE(
        FUGA_MSG2("SlotError",
            FUGA_STRING("get: no slot with index"),
            FUGA_INT(index)
        )
    );
}

Fuga* Fuga_getBySymbol(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);

    if (self->slots) {
        FugaSlot* slot = FugaSlots_getBySymbol(self->slots, name);
        if (slot)
            return slot->value;
    }

    if (self->proto)
        return Fuga_getBySymbol(self->proto, name);

    FUGA_RAISE(
        FUGA_MSG2("SlotError",
            FUGA_STRING("get: no slot with name"),
            name 
        )
    );
}

Fuga* Fuga_getByString(Fuga* self, const char* str) {
    ALWAYS(self); ALWAYS(str);
    FUGA_NEED(self);
    return Fuga_getBySymbol(self, FUGA_SYMBOL(str));
}

Fuga* Fuga_get(Fuga* self, Fuga* name) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    switch (name->type) {
    case FUGA_TYPE_SYMBOL:
        return Fuga_getBySymbol(self, name);
    case FUGA_TYPE_MSG:
        return Fuga_getBySymbol(self, name->data.MSG);
    case FUGA_TYPE_STRING:
        return Fuga_getByString(self, name->data.STRING);
    case FUGA_TYPE_INT:
        return Fuga_getByIndex(self, name->data.INT);
    case FUGA_TYPE_LONG:
        FUGA_RAISE(
            FUGA_MSG1("ValueError",
                FUGA_STRING("get: index is too positive"
                            " or too negative.")
            )
        );
    default:
        FUGA_RAISE(
            FUGA_MSG1("TypeError",
                FUGA_STRING("set: name must be a primitive symbol,"
                            " msg, string, or int.")
            )
        );
    }
}

#ifdef TESTING
TESTS(Fuga_get) {
    Fuga* self = Fuga_new();
    Fuga* obj1 = Fuga_clone(FUGA_Object);
    Fuga* obj2 = Fuga_clone(obj1);
    Fuga* iobj1 = Fuga_clone(FUGA_Object);
    Fuga* iobj2 = Fuga_clone(iobj1);

    Fuga_setByIndex  (iobj1, 0, iobj1);
    Fuga_setByIndex  (iobj1, 1, iobj2);
    Fuga_setByIndex  (iobj2, 0, iobj2);
    Fuga_setByString (obj1, "foo", obj1);
    Fuga_setByString (obj2, "bar", obj1);
    Fuga_setByString (obj1, "baz", obj1);
    Fuga_setByString (obj2, "baz", obj2);
    Fuga_setBySymbol (obj1, FUGA_SYMBOL("do"), obj1);
    Fuga_setBySymbol (obj2, FUGA_SYMBOL("re"), obj1);
    Fuga_setBySymbol (obj1, FUGA_SYMBOL("mi"), obj1);
    Fuga_setBySymbol (obj2, FUGA_SYMBOL("mi"), obj2);
    
    TEST(iobj1 ==     Fuga_rawGetByIndex(iobj1, 0) );
    TEST(iobj2 ==     Fuga_rawGetByIndex(iobj2, 0) );
    TEST(iobj2 ==     Fuga_rawGetByIndex(iobj1, 1) );
    TEST(Fuga_isError(Fuga_rawGetByIndex(iobj2, 1)));
    TEST(Fuga_isError(Fuga_rawGetByIndex(iobj1, 2)));
    TEST(Fuga_isError(Fuga_rawGetByIndex(iobj2, 2)));

    TEST(obj1 ==    Fuga_rawGetByString(obj1, "foo") );
    TEST(Fuga_isError(Fuga_rawGetByString(obj2, "foo")));
    TEST(Fuga_isError(Fuga_rawGetByString(obj1, "bar")));
    TEST(obj1 ==    Fuga_rawGetByString(obj2, "bar") );
    TEST(obj1 ==    Fuga_rawGetByString(obj1, "baz") );
    TEST(obj2 ==    Fuga_rawGetByString(obj2, "baz") );
    TEST(Fuga_isError(Fuga_rawGetByString(obj1, "bif")));
    TEST(Fuga_isError(Fuga_rawGetByString(obj2, "bif")));
    TEST(obj1 ==    Fuga_rawGetBySymbol(obj1, FUGA_SYMBOL("do")) );
    TEST(Fuga_isError(Fuga_rawGetBySymbol(obj2, FUGA_SYMBOL("do"))));
    TEST(Fuga_isError(Fuga_rawGetBySymbol(obj1, FUGA_SYMBOL("re"))));
    TEST(obj1 ==    Fuga_rawGetBySymbol(obj2, FUGA_SYMBOL("re")) );
    TEST(obj1 ==    Fuga_rawGetBySymbol(obj1, FUGA_SYMBOL("mi")) );
    TEST(obj2 ==    Fuga_rawGetBySymbol(obj2, FUGA_SYMBOL("mi")) );
    TEST(Fuga_isError(Fuga_rawGetBySymbol(obj1, FUGA_SYMBOL("fa"))));
    TEST(Fuga_isError(Fuga_rawGetBySymbol(obj2, FUGA_SYMBOL("fa"))));

    TEST(obj1 ==    Fuga_getByString(obj1, "foo") );
    TEST(obj1 ==    Fuga_getByString(obj2, "foo") );
    TEST(Fuga_isError(Fuga_getByString(obj1, "bar")));
    TEST(obj1 ==    Fuga_getByString(obj2, "bar") );
    TEST(obj1 ==    Fuga_getByString(obj1, "baz") );
    TEST(obj2 ==    Fuga_getByString(obj2, "baz") );
    TEST(Fuga_isError(Fuga_getByString(obj1, "bif")));
    TEST(Fuga_isError(Fuga_getByString(obj2, "bif")));
    TEST(obj1 ==    Fuga_getBySymbol(obj1, FUGA_SYMBOL("do")) );
    TEST(obj1 ==    Fuga_getBySymbol(obj2, FUGA_SYMBOL("do")) );
    TEST(Fuga_isError(Fuga_getBySymbol(obj1, FUGA_SYMBOL("re"))));
    TEST(obj1 ==    Fuga_getBySymbol(obj2, FUGA_SYMBOL("re")) );
    TEST(obj1 ==    Fuga_getBySymbol(obj1, FUGA_SYMBOL("mi")) );
    TEST(obj2 ==    Fuga_getBySymbol(obj2, FUGA_SYMBOL("mi")) );
    TEST(Fuga_isError(Fuga_getBySymbol(obj1, FUGA_SYMBOL("fa"))));
    TEST(Fuga_isError(Fuga_getBySymbol(obj2, FUGA_SYMBOL("fa"))));
}
#endif

/**
*** ## Set
**/

void Fuga_setByIndex(Fuga* self, FugaIndex index, Fuga* value) { 
    ALWAYS(self); ALWAYS(value);
    ALWAYS(FUGA_READY(self));
    if (!self->slots)
        self->slots = FugaSlots_new(self);
    FugaSlot slot = { .name = NULL, .value = value, .doc = NULL };
    FugaSlots_setByIndex(self->slots, index, slot);
}

void Fuga_setBySymbol(Fuga* self, Fuga* name, Fuga* value) {
    ALWAYS(self); ALWAYS(name); ALWAYS(value);
    ALWAYS(FUGA_READY(self));
    ALWAYS(name->type == FUGA_TYPE_SYMBOL);
    if (!self->slots)
        self->slots = FugaSlots_new(self);
    FugaSlot slot = { .name = name, .value = value, .doc = NULL };
    FugaSlots_setBySymbol(self->slots, name, slot);
}

void Fuga_setByString(Fuga* self, const char* name, Fuga* value) {
    ALWAYS(self); ALWAYS(name); ALWAYS(value);
    ALWAYS(FUGA_READY(self));
    Fuga_setBySymbol(self, FUGA_SYMBOL(name), value);
}

Fuga* Fuga_set(Fuga* self, Fuga* name, Fuga* value) {
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    switch (name->type) {
    case FUGA_TYPE_SYMBOL:
        Fuga_setBySymbol(self, name, value);
        return FUGA_nil;
    case FUGA_TYPE_MSG:
        Fuga_setBySymbol(self, name->data.MSG, value);
        return FUGA_nil;
    case FUGA_TYPE_STRING:
        Fuga_setByString(self, name->data.STRING, value);
        return FUGA_nil;
    case FUGA_TYPE_INT:
        if (name->data.INT > Fuga_length(self)) {
            FUGA_MSG1("ValueError",
                FUGA_STRING("set: index is too large")
            );
        }
        Fuga_setByIndex(self, name->data.INT, value);
        return FUGA_nil;
    case FUGA_TYPE_LONG:
        FUGA_RAISE(
            FUGA_MSG1("ValueError",
                FUGA_STRING("get: index is too large")
            )
        );
    default:
        FUGA_RAISE(
            FUGA_MSG1("TypeError",
                FUGA_STRING("set: name must be a primitive symbol,"
                            " msg, string, or int.")
            )
        );
    }
}


/**
*** ## Properties
*** ### Fuga_is
**/
Fuga* Fuga_is(Fuga* self, Fuga* other) {
    FUGA_NEED(self);
    FUGA_NEED(other);
    return FUGA_BOOL(self->id == other->id);
}

/**
*** ### Fuga_isa
**/
Fuga* Fuga_isa(Fuga* self, Fuga* other) {
    FUGA_NEED(self);
    for (; other; other = other->proto) {
        FUGA_DECL(result, Fuga_is(self->proto, other));
        if (result == FUGA_true) return FUGA_true;
        ALWAYS(result == FUGA_false);
    }
    return FUGA_false;
}


/**
*** ### Thunks
**/
Fuga* Fuga_need(Fuga* self) {
    // TODO: implement. This isn't supposed to be a no-op.
    ALWAYS(FUGA_READY(self));
    return self;
}

