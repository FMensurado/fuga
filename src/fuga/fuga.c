
#include "test.h"
#include "fuga.h"
#include "prelude.h"

#include <string.h>

void FugaRoot_mark(
    void* self
) {
    Fuga_mark_(self, FUGA->symbols);

    Fuga_mark_(self, FUGA->Object);
    Fuga_mark_(self, FUGA->Prelude);
    Fuga_mark_(self, FUGA->Number);
    Fuga_mark_(self, FUGA->Int);
    Fuga_mark_(self, FUGA->String);
    Fuga_mark_(self, FUGA->Symbol);
    Fuga_mark_(self, FUGA->Msg);
    Fuga_mark_(self, FUGA->Method);
    Fuga_mark_(self, FUGA->Expr);
    Fuga_mark_(self, FUGA->nil);
    Fuga_mark_(self, FUGA->Bool);
    Fuga_mark_(self, FUGA->True);
    Fuga_mark_(self, FUGA->False);

    Fuga_mark_(self, FUGA->Exception);
    Fuga_mark_(self, FUGA->TypeError);
    Fuga_mark_(self, FUGA->SlotError);
    Fuga_mark_(self, FUGA->IOError);
    Fuga_mark_(self, FUGA->ValueError);
    Fuga_mark_(self, FUGA->SyntaxError);
}

void Fuga_initObject (void* self);
void Fuga_initBool   (void* self);

void FugaRoot_init(
    void *self
) {

    FUGA->Object  = FUGA;
    FUGA->Prelude = Fuga_clone(FUGA->Object);

    FUGA->Bool  = Fuga_clone(FUGA->Object);
    FUGA->True  = Fuga_clone(FUGA->Bool);
    FUGA->False = Fuga_clone(FUGA->Bool);
    FUGA->nil   = Fuga_clone(FUGA->Object);

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
    FUGA->IOError      = Fuga_clone(FUGA->Exception);
    FUGA->SyntaxError  = Fuga_clone(FUGA->Exception);

    FUGA->symbols = FugaSymbols_new(self);

    FugaPrelude_init(FUGA->Prelude);
    FugaInt_init(FUGA->Prelude);
    FugaString_init(FUGA->Prelude);
    FugaSymbol_init(FUGA->Prelude);
    FugaMsg_init(FUGA->Prelude);
    FugaMethod_init(FUGA->Prelude);

    Fuga_initObject(FUGA->Prelude);
    Fuga_initBool(FUGA->Prelude);
}

/**
 * Initialize a Fuga environment.
 */
void* Fuga_init(
) {
    FugaHeader *header = calloc(sizeof(FugaHeader)+sizeof(FugaRoot), 1);
    FugaRoot   *self   = FUGA_DATA(header);
    
    header->root = self;
    header->gc.root = true;
    FugaGCList_init(&header->gc.list);
    FugaGCList_init(&self->white);
    FugaGCList_init(&self->grey);
    FugaGCList_init(&self->black);
    FugaGCList_init(&self->roots);
    FugaGCList_push_(&self->roots, &header->gc.list);
    Fuga_onMark_(self, FugaRoot_mark);


    FugaRoot_init(self);

    return self;
}


void Fuga_initObject(void* self) {
    Fuga_set_to_(FUGA->Object, "str", FUGA_METHOD_STR(Fuga_strSlots));
    Fuga_set_to_(FUGA->Object, "has", FUGA_METHOD_1ARG(Fuga_hasBy_));
    Fuga_set_to_(FUGA->Object, "get", FUGA_METHOD_1ARG(Fuga_getBy_));
    Fuga_set_to_(FUGA->Object, "set", FUGA_METHOD_2ARG(Fuga_setBy_to_));
    Fuga_set_to_(FUGA->Object, "hasRaw", FUGA_METHOD_1ARG(Fuga_hasRawBy_));
    Fuga_set_to_(FUGA->Object, "getRaw", FUGA_METHOD_1ARG(Fuga_getRawBy_));
    Fuga_set_to_(FUGA->Object, "length",
        FUGA_METHOD_0ARG((void*(*)(void*))Fuga_length));

    // *cough* sorry. this is here, but this location doesn't make sense
    Fuga_setBy_to_(FUGA->nil,   FUGA_SYMBOL("name"), FUGA_STRING("nil"));
}

void Fuga_initBool(void* self) {
    Fuga_set_to_(FUGA->True,  "name", FUGA_STRING("true"));
    Fuga_set_to_(FUGA->False, "name", FUGA_STRING("false"));
}

#ifdef TESTING
TESTS(Fuga_init) {
    void* self = Fuga_init();
    for (size_t i = 0; i < sizeof *FUGA / sizeof(size_t); i++)
        TEST(((size_t*)FUGA)[i]);
    
    FugaHeader* header = FUGA_HEADER(FUGA);
    TEST(FUGA->roots.next == (void*)header);
    TEST(header->gc.list.next == &FUGA->roots);
    TEST(header->gc.list.prev == &FUGA->roots);
    TEST(header->gc.pass == 0);

    Fuga_quit(self);
}
#endif


void FugaHeader_free(
    FugaHeader* self
) {
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));
    if (self->gc.free)
        self->gc.free(FUGA_DATA(self));
    free(self);
}

void FugaHeader_mark(
    FugaHeader* header
) {
    ALWAYS(header);
    NEVER(Fuga_isRaised(header));
    header->gc.pass = FUGA_HEADER(header->root)->gc.pass;
    Fuga_mark_(FUGA_DATA(header), header->root);
    Fuga_mark_(FUGA_DATA(header), header->slots);
    Fuga_mark_(FUGA_DATA(header), header->proto);
    if (header->gc.mark)
        header->gc.mark(FUGA_DATA(header));
}

/**
 * Clean up after Fuga.
 */
void Fuga_quit(void* self)
{
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));

    self = FUGA;
    FugaGCList_append_(&FUGA->white, &FUGA->black);
    FugaGCList_append_(&FUGA->white, &FUGA->grey);
    FugaGCList_append_(&FUGA->white, &FUGA->roots);
    FugaGCList *iter = FUGA->white.next;
    while (iter != &FUGA->white) {
        void* old = iter;
        iter = iter->next;
        if (old != FUGA_HEADER(self))
            FugaHeader_free(old);
    }
    FugaHeader_free(FUGA_HEADER(self));
}

/**
 * Clone.
 */
void* Fuga_clone(
    void* proto
) {
    return Fuga_clone_(proto, 0);
}

void* Fuga_clone_(
    void* proto,
    size_t size
) {
    ALWAYS(proto);
    FUGA_CHECK(proto);
    FugaHeader* header  = calloc(sizeof(FugaHeader)+size, 1);
    void* self    = FUGA_DATA(header);
    header->root  = FUGA_HEADER(proto)->root;
    header->proto = proto;
    header->gc.pass = FUGA_HEADER(header->root)->gc.pass;
    FugaGCList_init(&header->gc.list);
    FugaGCList_push_(&FUGA->grey, &header->gc.list);
    return self;
}

const FugaType* Fuga_type(void* self) {
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));
    return FUGA_HEADER(self)->type;
}

void Fuga_type_(void* self, const FugaType* type) {
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));
    FUGA_HEADER(self)->type = type;
}

/**
*** ## Garbage Collection
**/

void Fuga_onMark_(
    void* self,
    void (*markFn)(void*)
) {
    ALWAYS(self);
    FUGA_HEADER(self)->gc.mark = markFn;
}

void Fuga_onFree_(
    void* self,
    void (*freeFn)(void*)
) {
    ALWAYS(self);
    FUGA_HEADER(self)->gc.free = freeFn;
}

void Fuga_root(
    void* self
) {
    ALWAYS(self); NEVER(Fuga_isRaised(self));
    FugaHeader* header = FUGA_HEADER(self);
    NEVER(header->gc.root);
    header->gc.root = true;
    FugaGCList_unlink(&header->gc.list);
    FugaGCList_push_(&FUGA->roots, &header->gc.list);
}

void Fuga_unroot(
    void* self
) {
    ALWAYS(self); NEVER(Fuga_isRaised(self));
    FugaHeader* header = FUGA_HEADER(self);
    ALWAYS(header->gc.root);
    header->gc.root = false;
    FugaGCList_unlink(&header->gc.list);
    FugaGCList_push_(&FUGA->grey, &header->gc.list);
}

void Fuga_mark_(
    void* self,
    void* child
) {
    ALWAYS(self);
    if (!child) return;
    NEVER(Fuga_isRaised(self));
    NEVER(Fuga_isRaised(child));
    unsigned pass = FUGA_HEADER(FUGA)->gc.pass;
    FugaHeader* header = FUGA_HEADER(child);
    if (header->gc.pass != pass) {
        header->gc.pass = pass;
        if (header->gc.root)
            return;
        FugaGCList_unlink(&header->gc.list);
        FugaGCList_push_(&FUGA->grey, &header->gc.list);
    }
}


void Fuga_collect(
    void* self
) {
    ALWAYS(self);
    self = FUGA;
    FUGA_HEADER(FUGA)->gc.pass += 1;
    FugaGCList_append_(&FUGA->white, &FUGA->grey);
    FugaGCList_append_(&FUGA->white, &FUGA->black);
    FugaGCList* iter = FUGA->roots.next;
    FugaHeader* header;
    while (iter != &FUGA->roots) {
        header = (FugaHeader*)iter;
        iter = iter->next;
        FugaHeader_mark(header);
    }
    while (!FugaGCList_empty(&FUGA->grey)) {
        iter = FugaGCList_pop(&FUGA->grey);
        header = (FugaHeader*)iter;
        FugaGCList_push_(&FUGA->black, iter);
        FugaHeader_mark(header);
    }
    while (!FugaGCList_empty(&FUGA->white)) {
        iter = FugaGCList_pop(&FUGA->white);
        header = (FugaHeader*)iter;
        FugaHeader_free(header);
    }
}

/**
 * Was an exception raised?
 */
bool Fuga_isRaised(
    void* self
) {
    return ((size_t)self) & 0x01;
}

/**
 * Raise an exception.
 */
void* Fuga_raise(
    void* self
) {
    ALWAYS(self);
    return (void*)((size_t)self | 0x01);
}

#ifdef TESTING
void* Fuga_raise_test(void* self) {
    FUGA_RAISE(FUGA->TypeError,
        "just testing FUGA_RAISE"
    );
}

TESTS(Fuga_raise) {
    void* self = Fuga_init();
    TEST(!Fuga_isRaised(FUGA->Int));
    TEST(!Fuga_isRaised(FUGA->Exception));
    TEST( Fuga_isRaised(Fuga_raise(FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_raise(FUGA->Exception)));
    TEST( Fuga_isRaised(Fuga_raise_test(self)));
    Fuga_quit(self);
}
#endif

/**
 * Catch a raised exception. Returns NULL if no exception.
 */
void* Fuga_catch(
    void* self
) {
    ALWAYS(self);
    if ((size_t)self & 0x01)
        return (void*)((size_t)self & ~0x01);
    return NULL;
}

#ifdef TESTING
void* Fuga_catch_test(void* self) {
    FUGA_CHECK(self);
    return FUGA->nil;
}

TESTS(Fuga_catch) {
    void* self = Fuga_init();
    TEST(!Fuga_catch(FUGA->Int));
    TEST(!Fuga_catch(FUGA->Exception));
    TEST( Fuga_catch(Fuga_raise(FUGA->Int)) == FUGA->Int);
    TEST( Fuga_catch(Fuga_raise(FUGA->Exception)) == FUGA->Exception);

    TEST(!Fuga_isRaised(Fuga_catch_test(FUGA->Int)))
    TEST(!Fuga_isRaised(Fuga_catch_test(FUGA->Exception)))
    TEST( Fuga_isRaised(Fuga_catch_test(Fuga_raise(FUGA->Int))))
    TEST( Fuga_isRaised(Fuga_catch_test(Fuga_raise(FUGA->Exception))))

    Fuga_quit(self);
}
#endif

/**
 * Are two objects the same (taking into account thunks).
 */
bool Fuga_is_(void* self, void* other)
{
    ALWAYS(self); ALWAYS(other);
    self  = Fuga_need(self);  if (Fuga_isRaised(self))  return false;
    other = Fuga_need(other); if (Fuga_isRaised(other)) return false;
    return self == other;
}

/**
 * Does one object inherit from the other?
 */
bool Fuga_isa_(void* self, void* other)
{
    ALWAYS(self); ALWAYS(other);
    self  = Fuga_need(self);  if (Fuga_isRaised(self))  return false;
    other = Fuga_need(other); if (Fuga_isRaised(other)) return false;
    void* proto = FUGA_HEADER(self)->proto;
    if (!proto) return false;
    return Fuga_is_(proto, other) || Fuga_isa_(proto, other);
}

bool Fuga_isTrue(void* self) {
    return !Fuga_isRaised(self) && Fuga_is_(self, FUGA->True);
}
bool Fuga_isFalse(void* self) {
    return !Fuga_isRaised(self) && Fuga_is_(self, FUGA->False);
}
bool Fuga_isNil(void* self) {
    return !Fuga_isRaised(self) && Fuga_is_(self, FUGA->nil);
}
bool Fuga_isExpr(void* self) {
    return !Fuga_isRaised(self) && Fuga_isa_(self, FUGA->Expr);
}

bool Fuga_hasType_(void* self, const FugaType* type) {
    return !Fuga_isRaised(self) && FUGA_HEADER(self)->type == type;
}

bool Fuga_isLazy(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaLazy_type);
}
bool Fuga_isInt(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaInt_type);
}
bool Fuga_isString(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaString_type);
}
bool Fuga_isSymbol(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaSymbol_type);
}
bool Fuga_isMsg(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaMsg_type);
}
bool Fuga_isMethod(void* self) {
    return !Fuga_isRaised(self) && Fuga_hasType_(self, &FugaMethod_type);
}

#ifdef TESTING
TESTS(Fuga_isa) {
    void* self = Fuga_init();
    TEST(!Fuga_isa_(FUGA->Object, FUGA->Object));
    TEST( Fuga_isa_(FUGA->Symbol, FUGA->Object));
    TEST(!Fuga_isa_(FUGA->Symbol, FUGA->Symbol));
    TEST(!Fuga_isa_(FUGA->Symbol, FUGA->String));
    TEST( Fuga_isa_(FUGA->Int,    FUGA->Number));
    TEST( Fuga_isa_(FUGA->Number, FUGA->Object));
    TEST( Fuga_isa_(FUGA->Int,    FUGA->Object));
    TEST(!Fuga_isa_(Fuga_raise(FUGA->Symbol), FUGA->Object));
    TEST(!Fuga_isa_(FUGA->Symbol, Fuga_raise(FUGA->Object)));
    Fuga_quit(self);
}
#endif

/**
 * Return the number of slots.
 */
FugaInt* Fuga_length(void* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    if (FUGA_HEADER(self)->slots)
        return FUGA_INT(FugaSlots_length(FUGA_HEADER(self)->slots));
    else
        return FUGA_INT(0);
}

bool Fuga_hasLength_(void* self, long value)
{
    self = Fuga_need(self);
    if (Fuga_isRaised(self))
        return false;
    return FugaInt_is_(Fuga_length(self), value);
}

/**
 * Converts a primitive into either an int or a symbol (raising an
 * exception on failure).
 */ 
void* Fuga_toName(void* name)
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
    void* self = name;
    FUGA_RAISE(FUGA->TypeError,
        "toName: expected primitive int, symbol, string, or msg"
    );
}

#ifdef TESTING
TESTS(Fuga_toName) {
    void* self = Fuga_init();
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
 * Return a vanilla object that shares its slots with self.
 */
void* Fuga_slots(
    void* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (!FUGA_HEADER(self)->slots)
        FUGA_HEADER(self)->slots = FugaSlots_new(self);
    return FUGA_HEADER(self)->slots;
}

/**
 * Get the slot associated with a given name. Return NULL if no such
 * slot. Do not pass raised exceptions to this function. name must 
 * be a FugaInt or a FugaSymbol.
 */
FugaSlot* Fuga_getSlot_(void* self, void* name) 
{
    ALWAYS(self); ALWAYS(name);
    ALWAYS(!Fuga_isRaised(self));
    ALWAYS(!Fuga_isRaised(name));

    if (FUGA_HEADER(self)->slots) {
        if (Fuga_isInt(name)) {
            long index = FugaInt_value(name);
            return FugaSlots_getByIndex(FUGA_HEADER(self)->slots, index);
        } else {
            return FugaSlots_getBySymbol(FUGA_HEADER(self)->slots, name);
        }
    }
    return NULL;
}

/**
 * Does a slot have a name for a given index?
 */
void* Fuga_hasNameBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    if (!Fuga_isInt(name))
        FUGA_RAISE(FUGA->TypeError,
            "hasName_: expected a primitive int"
        );
    if (FugaInt_value(name) < 0)
        FUGA_RAISE(FUGA->ValueError,
            "hasName_: index < 0"
        );
    if (FugaInt_value(name) >= FugaInt_value(Fuga_length(self)))
        FUGA_RAISE(FUGA->ValueError,
            "hasName_: index >= numSlots"
        );

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot->name)
        return FUGA->True;
    else
        return FUGA->False;
}

void* Fuga_hasDocBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "hasDoc: index < 0"
            );
        if (FugaInt_value(name) >= FugaInt_value(Fuga_length(self)))
            FUGA_RAISE(FUGA->ValueError,
                "hasDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot->doc)
        return FUGA->True;
    else
        return FUGA->False;
}

/**
 * Does a slot have a name for a given index?
 */
void* Fuga_getNameBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    if (!Fuga_isInt(name))
        FUGA_RAISE(FUGA->TypeError,
            "getName_: expected a primitive int"
        );
    if (FugaInt_value(name) < 0)
        FUGA_RAISE(FUGA->ValueError,
            "getName_: index < 0"
        );
    if (FugaInt_value(name) >= FugaInt_value(Fuga_length(self)))
        FUGA_RAISE(FUGA->ValueError,
            "getName_: index >= numSlots"
        );

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot->name) {
        return slot->name;
    } else {
        FUGA_RAISE(FUGA->SlotError,
            "getName_: slot has no name"
        );
    }
}

void* Fuga_getDocBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "getDoc: index < 0"
            );
        if (FugaInt_value(name) >= FugaInt_value(Fuga_length(self)))
            FUGA_RAISE(FUGA->ValueError,
                "getDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot && slot->doc)
        return slot->doc;
    FUGA_RAISE(FUGA->SlotError,
        "getDoc: no slot with name"
    );
}
void* Fuga_setDocBy_to_(void* self, void* name, void* value)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "setDoc: index < 0"
            );
        if (FugaInt_value(name) >= FugaInt_value(Fuga_length(self)))
            FUGA_RAISE(FUGA->ValueError,
                "setDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (!slot && slot->doc)
        FUGA_RAISE(FUGA->SlotError,
            "setDoc: no slot with name"
        );
    slot->doc = value;
    return FUGA->nil;
}

/**
 * Look in an object's slots.
 */
void* Fuga_hasRawBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaHeader* header = FUGA_HEADER(self);
    if (header->slots) {
        if (Fuga_isInt(name)) {
            long index = FugaInt_value(name);
            return FUGA_BOOL(FugaSlots_hasByIndex(header->slots, index));
        } else {
            return FUGA_BOOL(FugaSlots_hasBySymbol(header->slots, name));
        }
    }

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_hasRawBy_) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_hasRawBy_(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_hasRawBy_(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_hasRawBy_(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_hasRawBy_(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRawBy_(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_hasRawBy_(a, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRawBy_(b, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_hasRawBy_(a, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRawBy_(b, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasRawBy_(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_hasRawBy_(a, a)));
    TEST( Fuga_isRaised(Fuga_hasRawBy_(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_hasRawBy_(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_hasRawBy_(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_hasRawBy_(a, Fuga_raise(FUGA_INT(10)))));


    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(Fuga_isTrue(Fuga_hasRawBy_(Fuga_lazy_(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_hasRawBy_(obj, Fuga_lazy_(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_hasRawBy_(a,Fuga_lazy_(FUGA_SYMBOL("a"),self))));
}
#endif

/**
 * Generic has.
 */ 
void* Fuga_hasBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_CHECK(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    void* result = Fuga_hasRawBy_(self, name);
    FUGA_CHECK(result);
    if (Fuga_isTrue(result))
        return result;
    if (!Fuga_isFalse(result))
        FUGA_RAISE(FUGA->TypeError,
            "hasBy_: Expected boolean from hasRawBy_"
        );

    if (FUGA_HEADER(self)->proto && Fuga_isSymbol(name))
        return Fuga_hasBy_(FUGA_HEADER(self)->proto, name);

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_hasBy_) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_hasBy_(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasBy_(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_INT(42))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_INT(-1))));

    TEST( Fuga_isTrue  (Fuga_hasBy_(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_hasBy_(a, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_STRING("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_hasBy_(a, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_MSG("a"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(b, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasBy_(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasBy_(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_hasBy_(a, a)));
    TEST( Fuga_isRaised(Fuga_hasBy_(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_hasBy_(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_hasBy_(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_hasBy_(a, Fuga_raise(FUGA_INT(10)))));

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(Fuga_isTrue(Fuga_hasBy_(Fuga_lazy_(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_hasBy_(obj, Fuga_lazy_(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_hasBy_(a,Fuga_lazy_(FUGA_SYMBOL("a"),self))));
}
#endif



/**
 * Get the value of a slot, without looking in proto.
 */
void* Fuga_getRawBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        return slot->value;

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "getRawBy_: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_getRawBy_) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, NULL, c)));

    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_getRawBy_(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_getRawBy_(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getRawBy_(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getRawBy_(b, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getRawBy_(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getRawBy_(b, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getRawBy_(c, FUGA_MSG("c"))));

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(prim == Fuga_getRawBy_(Fuga_lazy_(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_getRawBy_(obj, Fuga_lazy_(FUGA_INT(0), self)));
    TEST(a == Fuga_getRawBy_(a, Fuga_lazy_(FUGA_SYMBOL("a"), self)));
}
#endif

/**
 * Generic get.
 */
void* Fuga_getBy_(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        return slot->value;

    if (FUGA_HEADER(self)->proto && Fuga_isSymbol(name))
        return Fuga_getBy_(FUGA_HEADER(self)->proto, name);

    // FIXME: give better error message
    FUGA_RAISE(FUGA->SlotError,
        "getBy_: No slot with given name."
    );
}

#ifdef TESTING
TESTS(Fuga_getBy_) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(b, NULL, c)));

    TEST( a  ==        (Fuga_getBy_(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_getBy_(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_getBy_(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_getBy_(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getBy_(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_INT(42))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_INT(-1))));

    TEST( a  ==        (Fuga_getBy_(a, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getBy_(b, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getBy_(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getBy_(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getBy_(b, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getBy_(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_getBy_(a, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getBy_(b, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getBy_(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getBy_(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getBy_(b, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getBy_(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_getBy_(a, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getBy_(b, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getBy_(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getBy_(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getBy_(b, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getBy_(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getBy_(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getBy_(c, FUGA_MSG("c"))));

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(prim == Fuga_getBy_(Fuga_lazy_(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_getBy_(obj, Fuga_lazy_(FUGA_INT(0), self)));
    TEST(a == Fuga_getBy_(a, Fuga_lazy_(FUGA_SYMBOL("a"), self)));

    Fuga_quit(self);
}
#endif

void* Fuga_append_(void* self, void* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_NEED(self);
    FUGA_CHECK(value);
    FugaSlots* slots = Fuga_slots(self);
    FUGA_CHECK(slots);

    FugaSlot slot = {.name = NULL, .value = value, .doc = NULL};
    FugaSlots_append_(slots, slot);
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
void* Fuga_setBy_to_(void* self, void* name, void* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_NEED(self);
    FUGA_CHECK(value);
    FugaSlots* slots = Fuga_slots(self);
    FUGA_CHECK(slots);

    if (!name)
        return Fuga_append_(self, value);
    FUGA_NEED(name);
    if (Fuga_isNil(name))
        return Fuga_append_(self, value);
    name = Fuga_toName(name);
    FUGA_CHECK(name);

    FugaSlot slot = {.name = name, .value = value, .doc = NULL};

    if (Fuga_isInt(name)) {
        slot.name = NULL;
        long index = FugaInt_value(name);
        if (index > FugaSlots_length(slots))
            FUGA_RAISE(FUGA->ValueError,
                "setBy_to_: index out of bounds (too large)"
            );
        if (index < 0)
            FUGA_RAISE(FUGA->ValueError,
                "setBy_to_: index out of bounds (negative)"
            );
        FugaSlots_setByIndex(slots, index, slot);
    } else {
        FugaSlots_setBySymbol(slots, name, slot);
    }
    
    return FUGA->nil;
}

#ifdef TESTING
TESTS(Fuga_setBy_to_) {
    void* self = Fuga_init();
    void* a = Fuga_clone(FUGA->Object);

    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_INT(0), a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_INT(1), a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_setBy_to_(a, FUGA_INT(3), a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, NULL,        a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_INT(3), a)) );
    TEST( Fuga_isRaised (Fuga_setBy_to_(a, FUGA_INT(-1), a)) );

    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_SYMBOL("b"), a)) );
    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_SYMBOL("a"), a)) );

    TEST( Fuga_isNil    (Fuga_setBy_to_(a, FUGA_STRING("x"), a)) );
    TEST( Fuga_isRaised (Fuga_setBy_to_(a, FUGA_STRING(""), a)) );

    TEST( Fuga_isRaised (Fuga_setBy_to_(Fuga_raise(a), FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_setBy_to_(a, Fuga_raise(FUGA_INT(0)), a)) );
    TEST( Fuga_isRaised (Fuga_setBy_to_(a, FUGA_INT(0), Fuga_raise(a))) );

    Fuga_quit(self);
}
#endif


void* Fuga_has_         (void* self, const char* name)
    { return Fuga_hasBy_        (self, FUGA_SYMBOL(name));   }
void* Fuga_hasRaw_      (void* self, const char* name)
    { return Fuga_hasRawBy_     (self, FUGA_SYMBOL(name));   }
void* Fuga_hasDoc_      (void* self, const char* name)
    { return Fuga_hasDocBy_     (self, FUGA_SYMBOL(name));   }
void* Fuga_get_         (void* self, const char* name)
    { return Fuga_getBy_        (self, FUGA_SYMBOL(name));   }
void* Fuga_getRaw_      (void* self, const char* name)
    { return Fuga_getRawBy_     (self, FUGA_SYMBOL(name));   }
void* Fuga_getDoc_      (void* self, const char* name)
    { return Fuga_getDocBy_     (self, FUGA_SYMBOL(name));   }
void* Fuga_set_to_      (void* self, const char* name, void* value)
    { return Fuga_setBy_to_     (self, FUGA_SYMBOL(name), value);   }
void* Fuga_setDoc_to_   (void* self, const char* name, void* value)
    { return Fuga_setDocBy_to_  (self, FUGA_SYMBOL(name), value);   }


void* Fuga_hasAt_       (void* self, size_t index)
    { return Fuga_hasBy_        (self, FUGA_INT(index));   }
void* Fuga_hasNameAt_   (void* self, size_t index)
    { return Fuga_hasNameBy_    (self, FUGA_INT(index));   }
void* Fuga_hasDocAt_    (void* self, size_t index)
    { return Fuga_hasDocBy_     (self, FUGA_INT(index));   }
void* Fuga_getAt_       (void* self, size_t index)
    { return Fuga_getBy_        (self, FUGA_INT(index));   }
void* Fuga_getNameAt_   (void* self, size_t index)
    { return Fuga_getNameBy_    (self, FUGA_INT(index));   }
void* Fuga_getDocAt_    (void* self, size_t index)
    { return Fuga_getDocBy_     (self, FUGA_INT(index));   }
void* Fuga_setAt_to_    (void* self, size_t index, void* value)
    { return Fuga_setBy_to_     (self, FUGA_INT(index), value);   }
void* Fuga_setDocAt_to_ (void* self, size_t index, void* value)
    { return Fuga_setDocBy_to_  (self, FUGA_INT(index), value);   }


/**
 * Call / resolve a method.
 */
void* Fuga_call(void* self, void* recv, void* args)
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
void* Fuga_send(void* self, void* name, void* args)
{
    ALWAYS(self);    ALWAYS(name);    ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(name); FUGA_CHECK(args);

    void* method = Fuga_getBy_(self, name);
    return Fuga_call(method, self, args);
}

/**
 * Evaluate an object
 */
void* Fuga_eval(void* self, void* recv, void* scope)
{
    ALWAYS(self); ALWAYS(recv); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);

    if (Fuga_isInt(self) || Fuga_isString(self) || Fuga_isSymbol(self))
        return self;

    if (Fuga_isMsg(self))
        return FugaMsg_eval_in_(self, recv, scope);

    if (Fuga_isExpr(self))
        return Fuga_evalExpr(self, recv, scope);

    return Fuga_evalSlots(self, scope);
}

void* Fuga_evalSlot(void* self, void* scope, void* result)
{
    ALWAYS(self); ALWAYS(result); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(result); FUGA_NEED(scope);
    
    scope = Fuga_clone(scope);
    Fuga_setBy_to_(scope, FUGA_SYMBOL("this"), result);
    void* value = Fuga_eval(self, scope, scope);
    if (!Fuga_isNil(value))
        FUGA_CHECK(Fuga_append_(result, value));
    return value;
}

#ifdef TESTING
TESTS(Fuga_eval) {
    void* self = Fuga_init();

    void* prim = FUGA_INT(10);
    TEST(prim == Fuga_eval(prim, self, self));
    prim = FUGA_STRING("Hello World!");
    TEST(prim == Fuga_eval(prim, self, self));
    prim = FUGA_SYMBOL("hello");
    TEST(prim == Fuga_eval(prim, self, self));

    void* scope = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_setBy_to_(scope, FUGA_SYMBOL("hello"),prim)));
    TEST(Fuga_is_(prim, Fuga_eval(FUGA_MSG("hello"), scope, scope)));

    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(!Fuga_isRaised(Fuga_append_(obj, FUGA_MSG("hello"))));
    TEST(!Fuga_isRaised(Fuga_setBy_to_(obj, FUGA_SYMBOL("hi"), prim)));
    void* eobj = Fuga_eval(obj, scope, scope);
    TEST(!Fuga_isRaised(eobj));
    TEST(Fuga_hasLength_((eobj), 3));
    TEST(prim == Fuga_getBy_(eobj, FUGA_INT(0)));
    TEST(prim == Fuga_getBy_(eobj, FUGA_INT(1)));
    TEST(prim == Fuga_getBy_(eobj, FUGA_INT(2)));

    TEST(Fuga_isRaised(Fuga_eval(Fuga_raise(prim), scope, scope)));
    TEST(Fuga_isRaised(Fuga_eval(prim, Fuga_raise(scope), scope)));
    TEST(Fuga_isRaised(Fuga_eval(prim, scope, Fuga_raise(scope))));

    Fuga_quit(self);
}
#endif



void* Fuga_evalSlots(void* self, void* scope)
{
    ALWAYS(self); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(scope);
    void* result = Fuga_clone(FUGA->Object);
    long length = FugaInt_value(Fuga_length(self));
    ALWAYS(length >= 0);
    for (long i = 0; i < length; i++) {
        void* slot = Fuga_getBy_(self, FUGA_INT(i));
        FUGA_CHECK(Fuga_evalSlot(slot, scope, result));
    }
    return result;
}

void* Fuga_evalExpr(
    void* self,
    void* recv,
    void* scope
) {
    ALWAYS(self); ALWAYS(recv); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);
    long length = FugaInt_value(Fuga_length(self));
    if (length < 1) {
        FUGA_RAISE(FUGA->ValueError,
            "Expr eval: can't evaluate empty expression"
        );
    }
    for (long i = 0; i < length; i++) {
        void* part = Fuga_getBy_(self, FUGA_INT(i));
        recv = Fuga_eval(part, recv, scope);
        FUGA_CHECK(recv);
    }
    return recv;
}


void* Fuga_str(void* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    void* result = Fuga_send(self, FUGA_SYMBOL("str"),
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
bool Fuga_str_test(void* self, const char* str) {
    return FugaString_is_(Fuga_str(self), str);
}
#define FUGA_STR_TEST(a,b) TEST(Fuga_str_test(a, b))

TESTS(Fuga_str) {
    void* self = Fuga_init();

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

    FUGA_STR_TEST(Fuga_getBy_(FUGA->Int, FUGA_SYMBOL("str")),
                  "method(...)");

    // test names
    FUGA_STR_TEST(FUGA->True,  "true");
    FUGA_STR_TEST(FUGA->False, "false");
    FUGA_STR_TEST(FUGA->nil,   "nil");

    Fuga_quit(self);
}
#endif

void* Fuga_strSlots(void* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    long length = FugaInt_value(Fuga_length(self));

    void* result = FUGA_STRING("(");

    for (size_t slotNum = 0; slotNum < length; slotNum++) {
        void* fSlotNum = FUGA_INT(slotNum);
        if (Fuga_isTrue(Fuga_hasNameBy_(self, fSlotNum))) {
            void* name = Fuga_getNameBy_(self, fSlotNum);
            name = FugaSymbol_toString(name);
            FUGA_CHECK(name);
            result = FugaString_cat_(result, name);
            result = FugaString_cat_(result, FUGA_STRING("="));
            // FIXME: handle op msg edge case
        }
        FUGA_CHECK(result);
        void* slot = Fuga_getBy_(self, FUGA_INT(slotNum));
        void* str  = Fuga_str(slot);
        FUGA_NEED(str);
        result = FugaString_cat_(result, str);
        if (slotNum < length-1)
            result = FugaString_cat_(result, FUGA_STRING(", "));
    }
    result = FugaString_cat_(result, FUGA_STRING(")"));
    return result;
}

void Fuga_printException(void* self) 
{
    ALWAYS(self);
    if (Fuga_isRaised(self))
        self = Fuga_catch(self);
    void *msg = Fuga_getBy_(self, FUGA_SYMBOL("msg"));
    printf("Exception raised:\n\t");
    if (Fuga_isString(msg)) {
        FugaString_print(msg);
    } else {
        Fuga_print(msg);
    }
}

void* Fuga_print(void* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    void* str = Fuga_str(self);
    FUGA_CHECK(str);
    ALWAYS(Fuga_isString(str));
    FugaString_print(str);
    return FUGA->nil;
}

