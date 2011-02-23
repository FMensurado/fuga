
#include "test.h"
#include "fuga.h"

#include <string.h>

void _FugaRoot_mark(void*_root) {
    void** root = _root;
    // FIXME: do this manually, since right now this is risky.
    for (size_t i = 1; i < sizeof(FugaRoot) / sizeof(void*); i++)
        FugaGC_mark(root, root[i]);
}

void _Fuga_mark(void* _self) {
    Fuga* self = _self;
    FugaGC_mark(self, self->root);
    FugaGC_mark(self, self->proto);
    FugaGC_mark(self, self->slots);
    if (self->size)
        FugaGC_mark(self, self->data);
}


Fuga* _Fuga_new(FugaRoot *root)
{
    Fuga* self = FugaGC_alloc(root, sizeof(Fuga));
    FugaGC_onMark(self, _Fuga_mark);
    self->root = root;
    self->id   = ++(root->lastID);
    return self;
}

/**
 * Initialize a Fuga environment.
 */
Fuga* Fuga_init()
{
    void *gc = FugaGC_start();
   
    FugaRoot *root = FugaGC_alloc(gc, sizeof(FugaRoot));
    FugaGC_onMark(root, _FugaRoot_mark);    
    root->symbols = FugaSymbols_new(gc);
    
    Fuga* self = _Fuga_new(root);

    FUGA->Object  = self;
    FUGA->Prelude = Fuga_clone(FUGA->Object);

    FUGA->Bool  = Fuga_clone(FUGA->Object);
    FUGA->True  = Fuga_clone(FUGA->Bool);
    FUGA->False = Fuga_clone(FUGA->Bool);
    FUGA->nil   = Fuga_clone(FUGA->Bool);

    FUGA->Number = Fuga_clone(FUGA->Object);
    FUGA->Int    = Fuga_clone(FUGA->Number);
    FUGA->String = Fuga_clone(FUGA->Object);
    FUGA->Symbol = Fuga_clone(FUGA->Object);
    FUGA->Msg    = Fuga_clone(FUGA->Object);
    FUGA->Method = Fuga_clone(FUGA->Object);
    FUGA->Expr   = Fuga_clone(FUGA->Object);

    FUGA->Exception    = Fuga_clone(FUGA->Object);
    FUGA->SlotError    = Fuga_clone(FUGA->Exception);
    FUGA->ValueError   = Fuga_clone(FUGA->Exception);
    FUGA->TypeError    = Fuga_clone(FUGA->Exception);
    FUGA->MutableError = Fuga_clone(FUGA->Exception);
    FUGA->IOError      = Fuga_clone(FUGA->Exception);
    FUGA->SyntaxError  = Fuga_clone(FUGA->Exception);

    FugaInt_init(FUGA->Prelude);
    FugaString_init(FUGA->Prelude);
    FugaSymbol_init(FUGA->Prelude);
    FugaMsg_init(FUGA->Prelude);
    FugaMethod_init(FUGA->Prelude);

    Fuga_initObject(FUGA->Prelude);
    Fuga_initBool(FUGA->Prelude);

    return FUGA->Prelude;
}

void Fuga_initObject(Fuga* self) {
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("str"), 
        FUGA_METHOD_STR(Fuga_strSlots));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("hasSlot"),
        FUGA_METHOD_1ARG(Fuga_hasSlot));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("getSlot"),
        FUGA_METHOD_1ARG(Fuga_getSlot));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("setSlot"),
        FUGA_METHOD_2ARG(Fuga_setSlot));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("hasSlotRaw"),
        FUGA_METHOD_1ARG(Fuga_hasSlotRaw));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("getSlotRaw"),
        FUGA_METHOD_1ARG(Fuga_getSlotRaw));
    Fuga_setSlot(FUGA->Object, FUGA_SYMBOL("numSlots"),
        FUGA_METHOD_0ARG(Fuga_numSlots));

    // *cough* sorry. this is here, but this location doesn't make sense
    Fuga_setSlot(FUGA->nil,   FUGA_SYMBOL("name"), FUGA_STRING("nil"));
}

void Fuga_initBool(Fuga* self) {
    Fuga_setSlot(FUGA->True,  FUGA_SYMBOL("name"), FUGA_STRING("true"));
    Fuga_setSlot(FUGA->False, FUGA_SYMBOL("name"), FUGA_STRING("false"));
}

#ifdef TESTING
TESTS(Fuga_init) {
    Fuga* self = Fuga_init();
    for (size_t i = 0; i < sizeof *FUGA / sizeof(size_t); i++)
        TEST(((size_t*)FUGA)[i]);
    Fuga_quit(self);
}
#endif

/**
 * Clean up after Fuga.
 */
void Fuga_quit(Fuga* self)
{
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));
    FugaGC_end(self);
}

/**
 * Clone.
 */
Fuga* Fuga_clone(Fuga* proto)
{
    ALWAYS(proto);
    FUGA_CHECK(proto);
    Fuga* self = _Fuga_new(proto->root);
    self->proto = proto;
    return self;
}

/**
 * Raise an exception.
 */
Fuga* Fuga_raise(Fuga* self)
{
    ALWAYS(self);
    return (Fuga*)((size_t)self | 0x01);
}

#ifdef TESTING
Fuga* _Fuga_raise_test(Fuga* self) {
    FUGA_RAISE(FUGA->TypeError,
        "just testing FUGA_RAISE"
    );
}

TESTS(Fuga_raise) {
    Fuga* self = Fuga_init();
    TEST(!Fuga_isRaised(FUGA->Int));
    TEST(!Fuga_isRaised(FUGA->Exception));
    TEST( Fuga_isRaised(Fuga_raise(FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_raise(FUGA->Exception)));
    TEST( Fuga_isRaised(_Fuga_raise_test(self)));
    Fuga_quit(self);
}
#endif

/**
 * Catch a raised exception. Returns NULL if no exception.
 */
Fuga* Fuga_catch(Fuga* self)
{
    ALWAYS(self);
    if ((size_t)self & 0x01)
        return (Fuga*)((size_t)self & ~0x01);
    return NULL;
}

#ifdef TESTING
TESTS(Fuga_catch) {
    Fuga* self = Fuga_init();
    TEST(!Fuga_catch(FUGA->Int));
    TEST(!Fuga_catch(FUGA->Exception));
    TEST( Fuga_catch(Fuga_raise(FUGA->Int)) == FUGA->Int);
    TEST( Fuga_catch(Fuga_raise(FUGA->Exception)) == FUGA->Exception);
    Fuga_quit(self);
}
#endif

/**
 * Are two objects the same (taking into account thunks).
 */
Fuga* Fuga_is(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    return FUGA_BOOL(self->id == other->id);
}

/**
 * Does one object inherit from the other?
 */
Fuga* Fuga_isa(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    Fuga* proto = self->proto;
    if (proto) {
        FUGA_NEED(proto);
        return FUGA_BOOL((proto->id == other->id)
                    || Fuga_isTrue(Fuga_isa(proto, other)));
    } else {
        return FUGA->False;
    }
}

bool Fuga_isRaised(Fuga* self) {
    return ((size_t)self) & 0x01;
}
bool Fuga_isTrue(Fuga* self) {
    return !Fuga_isRaised(self) && self->id == self->root->True->id;
}
bool Fuga_isFalse(Fuga* self) {
    return !Fuga_isRaised(self) && self->id == self->root->False->id;
}
bool Fuga_isNil(Fuga* self) {
    return !Fuga_isRaised(self) && self->id == self->root->nil->id;
}
bool Fuga_isInt(Fuga* self) {
    return !Fuga_isRaised(self) && self->type == FUGA_TYPE_INT;
}
bool Fuga_isString(Fuga* self) {
    return !Fuga_isRaised(self) && self->type == FUGA_TYPE_STRING;
}
bool Fuga_isSymbol(Fuga* self) {
    return !Fuga_isRaised(self) && self->type == FUGA_TYPE_SYMBOL;
}
bool Fuga_isMsg(Fuga* self) {
    return !Fuga_isRaised(self) && self->type == FUGA_TYPE_MSG;
}
bool Fuga_isMethod(Fuga* self) {
    return !Fuga_isRaised(self) && self->type == FUGA_TYPE_METHOD;
}
bool Fuga_isExpr(Fuga* self) {
    return !Fuga_isRaised(self) &&
        Fuga_isTrue(Fuga_isa(self, FUGA->Expr));
}

#ifdef TESTING
TESTS(Fuga_isa) {
    Fuga* self = Fuga_init();
    TEST(Fuga_isFalse (Fuga_isa(FUGA->Object, FUGA->Object)));
    TEST(Fuga_isTrue  (Fuga_isa(FUGA->Symbol, FUGA->Object)));
    TEST(Fuga_isFalse (Fuga_isa(FUGA->Symbol, FUGA->Symbol)));
    TEST(Fuga_isFalse (Fuga_isa(FUGA->Symbol, FUGA->Symbol)));
    TEST(Fuga_isFalse (Fuga_isa(FUGA->Symbol, FUGA->String)));
    TEST(Fuga_isTrue  (Fuga_isa(FUGA->Int,    FUGA->Number)));
    TEST(Fuga_isTrue  (Fuga_isa(FUGA->Number, FUGA->Object)));
    TEST(Fuga_isTrue  (Fuga_isa(FUGA->Int,    FUGA->Object)));
    TEST(Fuga_isRaised(Fuga_isa(Fuga_raise(FUGA->Symbol),FUGA->Object)));
    TEST(Fuga_isRaised(Fuga_isa(FUGA->Symbol,Fuga_raise(FUGA->Object))));
    Fuga_quit(self);
}
#endif

/**
 * Converts a primitive into either an int or a symbol (raising an
 * exception on failure).
 */ 
Fuga* Fuga_toName(Fuga* name)
{
    ALWAYS(name);
    FUGA_NEED(name);
    if (Fuga_isSymbol(name))
        return name;
    if (Fuga_isInt(name))
        return name;
    if (Fuga_isString(name))
        return FugaString_toSymbol(name);
    if (Fuga_isMsg(name))
       return FugaMsg_toSymbol(name);
    Fuga* self = name;
    FUGA_RAISE(FUGA->TypeError,
        "toName: expected primitive int, symbol, string, or msg"
    );
}

#ifdef TESTING
TESTS(Fuga_toName) {
    Fuga* self = Fuga_init();
    TEST(Fuga_isInt   (Fuga_toName(FUGA_INT   (10))));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_SYMBOL("hello"))));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_STRING("hello"))));
    TEST(Fuga_isRaised(Fuga_toName(FUGA_STRING(""))));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_MSG("hello"))));
    TEST(Fuga_isRaised(Fuga_toName(FUGA->Int)));
    TEST(Fuga_isRaised(Fuga_toName(Fuga_raise(FUGA_INT(10)))));
    Fuga_quit(self);
}
#endif

/**
 * Return the number of slots.
 */
Fuga* Fuga_numSlots(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    return FUGA_INT(self->slots ? FugaSlots_length(self->slots) : 0);
}

/**
 * Get the slot associated with a given name. Return NULL if no such
 * slot. Do not pass raised exceptions to this function. name must 
 * be a FugaInt or a FugaSymbol.
 */
FugaSlot* _Fuga_getActualSlot(Fuga* self, Fuga* name) 
{
    ALWAYS(self); ALWAYS(name);
    ALWAYS(!Fuga_isRaised(self));
    ALWAYS(!Fuga_isRaised(name));

    if (self->slots) {
        if (Fuga_isInt(name)) {
            long index = FugaInt_value(name);
            return FugaSlots_getByIndex(self->slots, index);
        } else {
            return FugaSlots_getBySymbol(self->slots, name);
        }
    }
    return NULL;
}

/**
 * Does a slot have a name for a given index?
 */
Fuga* Fuga_hasSlotName(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    if (!Fuga_isInt(name))
        FUGA_RAISE(FUGA->TypeError,
            "hasSlotName: expected a primitive int"
        );
    if (FugaInt_value(name) < 0)
        FUGA_RAISE(FUGA->ValueError,
            "hasSlotName: index < 0"
        );
    if (FugaInt_value(name) >= FugaInt_value(Fuga_numSlots(self)))
        FUGA_RAISE(FUGA->ValueError,
            "hasSlotName: index >= numSlots"
        );

    FugaSlot* slot = _Fuga_getActualSlot(self, name);
    if (slot->name)
        return FUGA->True;
    else
        return FUGA->False;
}

/**
 * Does a slot have a name for a given index?
 */
Fuga* Fuga_getSlotName(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    if (!Fuga_isInt(name))
        FUGA_RAISE(FUGA->TypeError,
            "getSlotName: expected a primitive int"
        );
    if (FugaInt_value(name) < 0)
        FUGA_RAISE(FUGA->ValueError,
            "getSlotName: index < 0"
        );
    if (FugaInt_value(name) >= FugaInt_value(Fuga_numSlots(self)))
        FUGA_RAISE(FUGA->ValueError,
            "getSlotName: index >= numSlots"
        );

    FugaSlot* slot = _Fuga_getActualSlot(self, name);
    if (slot->name) {
        return slot->name;
    } else {
        FUGA_RAISE(FUGA->SlotError,
            "getSlotName: slot has no name"
        );
    }
}

/**
 * Look in an object's slots.
 */
Fuga* Fuga_hasSlotRaw(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    if (self->slots) {
        if (Fuga_isInt(name)) {
            long index = FugaInt_value(name);
            return FUGA_BOOL(FugaSlots_hasByIndex(self->slots, index));
        } else {
            return FUGA_BOOL(FugaSlots_hasBySymbol(self->slots, name));
        }
    }

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_hasSlotRaw) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(a, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(b, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(a, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlotRaw(b, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlotRaw(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_hasSlotRaw(a, a)));
    TEST( Fuga_isRaised(Fuga_hasSlotRaw(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_hasSlotRaw(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_hasSlotRaw(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_hasSlotRaw(a, Fuga_raise(FUGA_INT(10)))));


    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(Fuga_isTrue(Fuga_hasSlotRaw(Fuga_thunk(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_hasSlotRaw(obj, Fuga_thunk(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_hasSlotRaw(a,Fuga_thunk(FUGA_SYMBOL("a"),self))));
}
#endif

/**
 * Generic has.
 */ 
Fuga* Fuga_hasSlot(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_CHECK(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    Fuga* result = Fuga_hasSlotRaw(self, name);
    FUGA_CHECK(result);
    if (Fuga_isTrue(result))
        return result;
    if (!Fuga_isFalse(result))
        FUGA_RAISE(FUGA->TypeError,
            "hasSlot: Expected boolean from hasSlotRaw"
        );

    if (self->proto && Fuga_isSymbol(name))
        return Fuga_hasSlot(self->proto, name);

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_hasSlot) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_hasSlot(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlot(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_hasSlot(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_hasSlot(a, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_hasSlot(a, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(b, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasSlot(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasSlot(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_hasSlot(a, a)));
    TEST( Fuga_isRaised(Fuga_hasSlot(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_hasSlot(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_hasSlot(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_hasSlot(a, Fuga_raise(FUGA_INT(10)))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(Fuga_isTrue(Fuga_hasSlot(Fuga_thunk(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_hasSlot(obj, Fuga_thunk(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_hasSlot(a,Fuga_thunk(FUGA_SYMBOL("a"),self))));
}
#endif



/**
 * Get the value of a slot, without looking in proto.
 */
Fuga* Fuga_getSlotRaw(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = _Fuga_getActualSlot(self, name);
    if (slot)
        return slot->value;

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "getSlotRaw: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_getSlotRaw) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, NULL, c)));

    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_getSlotRaw(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_getSlotRaw(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getSlotRaw(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getSlotRaw(b, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getSlotRaw(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getSlotRaw(b, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getSlotRaw(c, FUGA_MSG("c"))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(prim == Fuga_getSlotRaw(Fuga_thunk(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_getSlotRaw(obj, Fuga_thunk(FUGA_INT(0), self)));
    TEST(a == Fuga_getSlotRaw(a, Fuga_thunk(FUGA_SYMBOL("a"), self)));
}
#endif

/**
 * Generic get.
 */
Fuga* Fuga_getSlot(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = _Fuga_getActualSlot(self, name);
    if (slot)
        return slot->value;

    if (self->proto && Fuga_isSymbol(name))
        return Fuga_getSlot(self->proto, name);

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "getSlot: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_getSlot) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_setSlot(b, NULL, c)));

    TEST( a  ==        (Fuga_getSlot(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_getSlot(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_getSlot(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_getSlot(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlot(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_getSlot(a, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getSlot(b, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getSlot(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getSlot(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getSlot(b, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getSlot(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_getSlot(a, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getSlot(b, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getSlot(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getSlot(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getSlot(b, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getSlot(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_getSlot(a, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getSlot(b, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getSlot(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getSlot(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getSlot(b, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getSlot(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getSlot(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getSlot(c, FUGA_MSG("c"))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(prim == Fuga_getSlot(Fuga_thunk(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_getSlot(obj, Fuga_thunk(FUGA_INT(0), self)));
    TEST(a == Fuga_getSlot(a, Fuga_thunk(FUGA_SYMBOL("a"), self)));

    Fuga_quit(self);
}
#endif

Fuga* Fuga_append(Fuga* self, Fuga* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_NEED(self);
    FUGA_CHECK(value);
    if (!self->slots)
        self->slots = FugaSlots_new(self);

    FugaSlot slot = {.name = NULL, .value = value, .doc = NULL};
    FugaSlots_append(self->slots, slot);
    return FUGA->nil;
}

/**
 * Set a slot to a given value.
 *
 * @param self Object in which to set slot.
 * @param name Name of the slot. Can be NULL, Int, String, Symbol, Msg
 * @param value Value to set.
 * @return nil on success, some raised exception otherwise.
 */
Fuga* Fuga_setSlot(Fuga* self, Fuga* name, Fuga* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_NEED(self);
    FUGA_CHECK(value);
    if (!self->slots)
        self->slots = FugaSlots_new(self);

    if (!name || Fuga_isNil(name))
        return Fuga_append(self, value);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot slot = {.name = name, .value = value, .doc = NULL};

    if (Fuga_isInt(name)) {
        slot.name = NULL;
        long index = FugaInt_value(name);
        if (index > FugaSlots_length(self->slots))
            FUGA_RAISE(FUGA->ValueError,
                "setSlot: index out of bounds (too large)"
            );
        if (index < 0)
            FUGA_RAISE(FUGA->ValueError,
                "setSlot: index out of bounds (negative)"
            );
        FugaSlots_setByIndex(self->slots, index, slot);
    } else {
        FugaSlots_setBySymbol(self->slots, name, slot);
    }
    
    return FUGA->nil;
}

#ifdef TESTING
TESTS(Fuga_setSlot) {
    Fuga* self = Fuga_init();
    Fuga* a = Fuga_clone(FUGA->Object);

    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_INT(0), a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_INT(1), a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_setSlot(a, FUGA_INT(3), a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, NULL,        a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_INT(3), a)) );
    TEST( Fuga_isRaised (Fuga_setSlot(a, FUGA_INT(-1), a)) );

    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_SYMBOL("a"), a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_SYMBOL("b"), a)) );
    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_SYMBOL("a"), a)) );

    TEST( Fuga_isNil    (Fuga_setSlot(a, FUGA_STRING("x"), a)) );
    TEST( Fuga_isRaised (Fuga_setSlot(a, FUGA_STRING(""), a)) );

    TEST( Fuga_isRaised (Fuga_setSlot(Fuga_raise(a), FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_setSlot(a, Fuga_raise(FUGA_INT(0)), a)) );
    TEST( Fuga_isRaised (Fuga_setSlot(a, FUGA_INT(0), Fuga_raise(a))) );

    Fuga_quit(self);
}
#endif

/**
 * Create a thunk.
 *
 * Thunk data is stored in the proto and data fields.
 */
Fuga* Fuga_thunk(Fuga* self, Fuga* scope)
{
    ALWAYS(self); ALWAYS(scope);
    FUGA_CHECK(self); FUGA_CHECK(scope);
    Fuga* thunk = _Fuga_new(self->root);
    thunk->id = 0;
    thunk->proto = scope;
    thunk->size = 1;
    thunk->data = self;
    return thunk;
}

/**
 * Evaluate a thunk. (If the result is a thunk, evaluate it again
 * and again and again...)
 */
Fuga* Fuga_need(Fuga* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    while (self->id == 0) {
        Fuga* result = Fuga_eval(self->data, self->proto, self->proto);
        FUGA_CHECK(result);
        *self = *result;
    }
    return self;
}

/**
 * Evaluate a thunk, but if the result is a thunk, return it without
 * evaluating it.
 */
Fuga* Fuga_needOnce(Fuga* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (self->id == 0) {
        Fuga* result = Fuga_eval(self->data, self->proto, self->proto);
        FUGA_CHECK(result);
        *self = *result;
    }
    return self;
}

/**
 * Call / resolve a method.
 */
Fuga* Fuga_call(Fuga* self, Fuga* recv, Fuga* args)
{
    ALWAYS(self);
    ALWAYS(recv);
    ALWAYS(args);
    FUGA_NEED(self);
    FUGA_CHECK(recv);
    FUGA_CHECK(args);
    if (Fuga_isMethod(self))
        return FugaMethod_call(self, recv, args);
    FUGA_NEED(args);
    if (FugaInt_value(Fuga_numSlots(args)))
        FUGA_RAISE(FUGA->TypeError,
            "attempt to call a non-method with arguments"
        );
    return self;
}


/**
 * Send
 */
Fuga* Fuga_send(Fuga* self, Fuga* name, Fuga* args)
{
    ALWAYS(self);    ALWAYS(name);    ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(name); FUGA_CHECK(args);

    Fuga* method = Fuga_getSlot(self, name);
    return Fuga_call(method, self, args);
}

/**
 * Evaluate an object
 */
Fuga* Fuga_eval(Fuga* self, Fuga* recv, Fuga* scope)
{
    ALWAYS(self); ALWAYS(recv); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);

    if (Fuga_isInt(self) || Fuga_isString(self) || Fuga_isSymbol(self))
        return self;

    if (Fuga_isMsg(self))
        return FugaMsg_eval(self, recv, scope);

    if (Fuga_isExpr(self))
        return Fuga_evalExpr(self, recv, scope);

    return Fuga_evalSlots(self, scope);
}

Fuga* Fuga_evalSlot(Fuga* self, Fuga* scope, Fuga* result)
{
    ALWAYS(self); ALWAYS(result); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(result); FUGA_NEED(scope);
    
    scope = Fuga_clone(scope);
    Fuga_setSlot(scope, FUGA_SYMBOL("this"), result);
    Fuga* value = Fuga_eval(self, scope, scope);
    if (!Fuga_isNil(value))
        FUGA_CHECK(Fuga_append(result, value));
    return value;
}

#ifdef TESTING
TESTS(Fuga_eval) {
    Fuga* self = Fuga_init();

    Fuga* prim = FUGA_INT(10);
    TEST(prim == Fuga_eval(prim, self, self));
    prim = FUGA_STRING("Hello World!");
    TEST(prim == Fuga_eval(prim, self, self));
    prim = FUGA_SYMBOL("hello");
    TEST(prim == Fuga_eval(prim, self, self));

    Fuga* scope = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_setSlot(scope, FUGA_SYMBOL("hello"),prim)));
    TEST(Fuga_is(prim, Fuga_eval(FUGA_MSG("hello"), scope, scope)));

    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(!Fuga_isRaised(Fuga_append(obj, FUGA_MSG("hello"))));
    TEST(!Fuga_isRaised(Fuga_setSlot(obj, FUGA_SYMBOL("hi"), prim)));
    Fuga* eobj = Fuga_eval(obj, scope, scope);
    TEST(!Fuga_isRaised(eobj));
    TEST(FugaInt_isEqualTo(Fuga_numSlots(eobj), 3));
    TEST(prim == Fuga_getSlot(eobj, FUGA_INT(0)));
    TEST(prim == Fuga_getSlot(eobj, FUGA_INT(1)));
    TEST(prim == Fuga_getSlot(eobj, FUGA_INT(2)));

    TEST(Fuga_isRaised(Fuga_eval(Fuga_raise(prim), scope, scope)));
    TEST(Fuga_isRaised(Fuga_eval(prim, Fuga_raise(scope), scope)));
    TEST(Fuga_isRaised(Fuga_eval(prim, scope, Fuga_raise(scope))));

    Fuga_quit(self);
}
#endif



Fuga* Fuga_evalSlots(Fuga* self, Fuga* scope)
{
    // Ultimately, [thunkSlots needSlots], not this.
    ALWAYS(self); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(scope);
    Fuga* result = Fuga_clone(FUGA->Object);
    long length = FugaInt_value(Fuga_numSlots(self));
    ALWAYS(length >= 0);
    for (long i = 0; i < length; i++) {
        Fuga *slot = Fuga_getSlot(self, FUGA_INT(i));
        FUGA_CHECK(Fuga_evalSlot(slot, scope, result));
    }
    return result;
}

Fuga* Fuga_evalExpr(
    Fuga* self,
    Fuga* recv,
    Fuga* scope
) {
    ALWAYS(self); ALWAYS(recv); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);
    long length = FugaInt_value(Fuga_numSlots(self));
    if (length < 1) {
        FUGA_RAISE(FUGA->ValueError,
            "Expr eval: can't evaluate empty expression"
        );
    }
    for (long i = 0; i < length; i++) {
        Fuga* part = Fuga_getSlot(self, FUGA_INT(i));
        recv = Fuga_eval(part, recv, scope);
        FUGA_CHECK(recv);
    }
    return recv;
}


Fuga* Fuga_str(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    Fuga* result = Fuga_send(self, FUGA_SYMBOL("str"),
                             Fuga_clone(FUGA->Object));
    FUGA_NEED(result);
    if (!Fuga_isString(result)) {
        FUGA_RAISE(FUGA->TypeError,
            "str: expected a string to be returned"
        );
    }
    return result;
}
#ifdef TESTING
bool Fuga_str_test(Fuga* self, const char* str) {
    return FugaString_isEqualTo(Fuga_str(self), str);
}
#define FUGA_STR_TEST(a,b) TEST(Fuga_str_test(a, b))

TESTS(Fuga_str) {
    Fuga* self = Fuga_init();

    FUGA_STR_TEST(Fuga_clone(FUGA->Object), "()");
    FUGA_STR_TEST(FUGA_INT(0), "0");
    FUGA_STR_TEST(FUGA_INT(10), "10");
    FUGA_STR_TEST(FUGA_INT(-10), "-10");
    FUGA_STR_TEST(FUGA_STRING(""), "\"\"");
    FUGA_STR_TEST(FUGA_STRING("Hello"), "\"Hello\"");
    FUGA_STR_TEST(FUGA_STRING("good\nbye\""), "\"good\\nbye\\\"\"");
    FUGA_STR_TEST(FUGA_STRING("\\\t\r"), "\"\\\\\\t\\r\"");
    FUGA_STR_TEST(FUGA_SYMBOL("abcdef"), ":abcdef");
    FUGA_STR_TEST(FUGA_SYMBOL("1st"), ":1st");
    FUGA_STR_TEST(FUGA_SYMBOL("+"), ":\\+");
    FUGA_STR_TEST(FUGA_MSG("abcdef"), "abcdef");
    FUGA_STR_TEST(FUGA_MSG("1st"), "1st");
    FUGA_STR_TEST(FUGA_MSG("+"), "\\+");
    // FIXME: add tests for objects and msgs with args.

    FUGA_STR_TEST(Fuga_getSlot(FUGA->Int, FUGA_SYMBOL("str")),
                  "method(...)");

    // test names
    FUGA_STR_TEST(FUGA->True,  "true");
    FUGA_STR_TEST(FUGA->False, "false");
    FUGA_STR_TEST(FUGA->nil,   "nil");

    Fuga_quit(self);
}
#endif

Fuga* Fuga_strSlots(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    size_t numSlots = FugaInt_value(Fuga_numSlots(self));

    Fuga* result = FUGA_STRING("(");

    for (size_t slotNum = 0; slotNum < numSlots; slotNum++) {
        Fuga* fSlotNum = FUGA_INT(slotNum);
        if (Fuga_isTrue(Fuga_hasSlotName(self, fSlotNum))) {
            Fuga* name = Fuga_getSlotName(self, fSlotNum);
            name = FugaSymbol_toString(name);
            FUGA_CHECK(name);
            result = FugaString_cat(result, name);
            result = FugaString_cat(result, FUGA_STRING("="));
            // FIXME: handle op msg edge case
        }
        FUGA_CHECK(result);
        Fuga* slot = Fuga_getSlot(self, FUGA_INT(slotNum));
        Fuga* str  = Fuga_str(slot);
        FUGA_NEED(str);
        result = FugaString_cat(result, str);
        if (slotNum < numSlots-1)
            result = FugaString_cat(result, FUGA_STRING(", "));
    }
    result = FugaString_cat(result, FUGA_STRING(")"));
    return result;
}

void Fuga_printException(Fuga* self) 
{
    ALWAYS(self);
    if (Fuga_isRaised(self))
        self = Fuga_catch(self);
    Fuga *msg = Fuga_getSlot(self, FUGA_SYMBOL("msg"));
    if (!Fuga_isString(msg)) {
        printf("Exception raised:\n\t");
        Fuga_print(self);
    } else {
        printf("Exception raised:\n\t%s\n", (char*)msg->data);
    }
}

Fuga* Fuga_print(Fuga* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    Fuga* str = Fuga_str(self);
    FUGA_CHECK(str);
    ALWAYS(Fuga_isString(str));
    FugaString_print(str);
    return FUGA->nil;
}

