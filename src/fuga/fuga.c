
#include "test.h"
#include "fuga.h"

#include <string.h>

void _FugaRoot_mark(void*_root) {
    void** root = _root;
    for (size_t i = 0; i < sizeof(FugaRoot) / sizeof(void*); i++)
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

    Fuga_set(FUGA->Object, FUGA_SYMBOL("str"), 
        FugaMethod_strMethod(self, Fuga_strSlots));

    return FUGA->Prelude;
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
Fuga* Fuga_length(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    return FUGA_INT(self->slots ? FugaSlots_length(self->slots) : 0);
}

/**
 * Look in an object's slots.
 */
Fuga* Fuga_rawHas(Fuga* self, Fuga* name)
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
TESTS(Fuga_rawHas) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_rawHas(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_rawHas(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_rawHas(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_rawHas(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_rawHas(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_rawHas(a, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_rawHas(b, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_rawHas(a, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_rawHas(b, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_rawHas(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_rawHas(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_rawHas(a, a)));
    TEST( Fuga_isRaised(Fuga_rawHas(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_rawHas(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_rawHas(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_rawHas(a, Fuga_raise(FUGA_INT(10)))));


    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(Fuga_isTrue(Fuga_rawHas(Fuga_thunk(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_rawHas(obj, Fuga_thunk(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_rawHas(a,Fuga_thunk(FUGA_SYMBOL("a"),self))));
}
#endif

/**
 * Generic has.
 */ 
Fuga* Fuga_has(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_CHECK(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    Fuga* result = Fuga_rawHas(self, name);
    FUGA_CHECK(result);
    if (Fuga_isTrue(result))
        return result;
    if (!Fuga_isFalse(result))
        FUGA_RAISE(FUGA->TypeError,
            "has: Expected boolean from rawHas"
        );

    if (self->proto && Fuga_isSymbol(name))
        return Fuga_has(self->proto, name);

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_has) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_has(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_has(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_has(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_has(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_has(a, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_has(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_has(a, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_has(b, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_has(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_has(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_has(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_has(a, a)));
    TEST( Fuga_isRaised(Fuga_has(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_has(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_has(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_has(a, Fuga_raise(FUGA_INT(10)))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(Fuga_isTrue(Fuga_has(Fuga_thunk(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_has(obj, Fuga_thunk(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_has(a,Fuga_thunk(FUGA_SYMBOL("a"),self))));
}
#endif


/**
 * Get the slot associated with a given name. Return NULL if no such
 * slot. Do not pass raised exceptions to this function. name must 
 * be a FugaInt or a FugaSymbol.
 */
FugaSlot* Fuga_rawGetSlot(Fuga* self, Fuga* name) 
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
 * Get the value of a slot, without looking in proto.
 */
Fuga* Fuga_rawGet(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_rawGetSlot(self, name);
    if (slot)
        return slot->value;

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "rawGet: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_rawGet) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, c)));

    TEST( a  ==        (Fuga_rawGet(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_rawGet(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_rawGet(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_rawGet(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_rawGet(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_rawGet(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_rawGet(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_rawGet(a, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_rawGet(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_rawGet(b, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_rawGet(a, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_rawGet(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_rawGet(b, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_rawGet(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_rawGet(c, FUGA_MSG("c"))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(prim == Fuga_rawGet(Fuga_thunk(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_rawGet(obj, Fuga_thunk(FUGA_INT(0), self)));
    TEST(a == Fuga_rawGet(a, Fuga_thunk(FUGA_SYMBOL("a"), self)));
}
#endif

/**
 * Generic get.
 */
Fuga* Fuga_get(Fuga* self, Fuga* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_rawGetSlot(self, name);
    if (slot)
        return slot->value;

    if (self->proto && Fuga_isSymbol(name))
        return Fuga_get(self->proto, name);

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "get: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_get) {
    Fuga* self = Fuga_init();
    
    Fuga* a = Fuga_clone(FUGA->Object);
    Fuga* b = Fuga_clone(a);
    Fuga* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, c)));

    TEST( a  ==        (Fuga_get(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_get(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_get(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_get(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_get(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_get(a, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_get(b, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_get(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_get(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_get(b, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_get(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_get(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_get(a, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_get(b, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_get(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_get(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_get(b, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_get(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_get(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_get(a, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_get(b, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_get(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_get(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_get(b, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_get(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_get(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_get(c, FUGA_MSG("c"))));

    Fuga* prim = FUGA_INT(10);
    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(prim == Fuga_get(Fuga_thunk(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_get(obj, Fuga_thunk(FUGA_INT(0), self)));
    TEST(a == Fuga_get(a, Fuga_thunk(FUGA_SYMBOL("a"), self)));

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
Fuga* Fuga_set(Fuga* self, Fuga* name, Fuga* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_NEED(self);
    FUGA_CHECK(value);
    if (!self->slots)
        self->slots = FugaSlots_new(self);

    if (!name) 
        return Fuga_append(self, value);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot slot = {.name = name, .value = value, .doc = NULL};

    if (Fuga_isInt(name)) {
        slot.name = NULL;
        long index = FugaInt_value(name);
        if (index > FugaSlots_length(self->slots))
            FUGA_RAISE(FUGA->ValueError,
                "set: index out of bounds (too large)"
            );
        if (index < 0)
            FUGA_RAISE(FUGA->ValueError,
                "set: index out of bounds (negative)"
            );
        FugaSlots_setByIndex(self->slots, index, slot);
    } else {
        FugaSlots_setBySymbol(self->slots, name, slot);
    }
    
    return FUGA->nil;
}

#ifdef TESTING
TESTS(Fuga_set) {
    Fuga* self = Fuga_init();
    Fuga* a = Fuga_clone(FUGA->Object);

    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(0), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(1), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_INT(3), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, NULL,        a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(3), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_INT(-1), a)) );

    TEST( Fuga_isNil    (Fuga_set(a, FUGA_SYMBOL("a"), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_SYMBOL("b"), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_SYMBOL("a"), a)) );

    TEST( Fuga_isNil    (Fuga_set(a, FUGA_STRING("x"), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_STRING(""), a)) );

    TEST( Fuga_isRaised (Fuga_set(Fuga_raise(a), FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, Fuga_raise(FUGA_INT(0)), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_INT(0), Fuga_raise(a))) );

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
    if (FugaInt_value(Fuga_length(args)))
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

    Fuga* method = Fuga_get(self, name);
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

    return Fuga_evalSlots(self, scope);
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
    TEST(!Fuga_isRaised(Fuga_set(scope, FUGA_SYMBOL("hello"), prim)));
    TEST(Fuga_is(prim, Fuga_eval(FUGA_MSG("hello"), scope, scope)));

    Fuga* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append(obj, prim)));
    TEST(!Fuga_isRaised(Fuga_append(obj, FUGA_MSG("hello"))));
    TEST(!Fuga_isRaised(Fuga_set(obj, FUGA_SYMBOL("hi"), prim)));
    Fuga* eobj = Fuga_eval(obj, scope, scope);
    TEST(!Fuga_isRaised(eobj));
    TEST(FugaInt_isEqualTo(Fuga_length(eobj), 3));
    TEST(prim == Fuga_get(eobj, FUGA_INT(0)));
    TEST(prim == Fuga_get(eobj, FUGA_INT(1)));
    TEST(prim == Fuga_get(eobj, FUGA_INT(2)));
    TEST(prim == Fuga_get(eobj, FUGA_SYMBOL("hi")));

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
    long length = FugaInt_value(Fuga_length(self));
    ALWAYS(length >= 0);
    for (long i = 0; i < length; i++) {
        FugaSlot* slot = Fuga_rawGetSlot(self, FUGA_INT(i));
        Fuga* value = Fuga_eval(slot->value, scope, scope);
        FUGA_CHECK(value);
        FUGA_CHECK(Fuga_set(result, slot->name, value));
    }
    return result;
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

Fuga* Fuga_strSlots(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    size_t numSlots = FugaInt_value(Fuga_length(self));
    // FIXME: use dynamic memory. (this causes overflows)
    char buffer[numSlots * 1024];
    size_t index = 0;

    buffer[index++] = '(';
    for (size_t slotNum = 0; slotNum < numSlots; slotNum++) {
        Fuga* slot = Fuga_get(self, FUGA_INT(slotNum));
        Fuga* string = Fuga_str(slot);

        FUGA_NEED(string);
        memcpy(buffer+index, string->data, string->size-1);
        index += string->size-1;
        buffer[index++] = ',';
        buffer[index++] = ' ';
    }
    if (numSlots)
        index -= 2;
    buffer[index++] = ')';
    buffer[index++] = '\0';
    return FUGA_STRING(buffer);
}

void Fuga_printException(Fuga* self) 
{
    Fuga *msg = Fuga_get(self, FUGA_SYMBOL("msg"));
    if (!Fuga_isString(msg)) {
        printf("Exception raised.\n");
    } else {
        printf("Exception: %s\n", (char*)msg->data);
    }
}

