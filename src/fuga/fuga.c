
#include "test.h"
#include "fuga.h"
#include "prelude.h"
#include "parser.h"
#include "path.h"
#include "thunk.h"
#include "loader.h"

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
    Fuga_mark_(self, FUGA->Path);
    Fuga_mark_(self, FUGA->Thunk);

    Fuga_mark_(self, FUGA->Exception);
    Fuga_mark_(self, FUGA->TypeError);
    Fuga_mark_(self, FUGA->SlotError);
    Fuga_mark_(self, FUGA->IOError);
    Fuga_mark_(self, FUGA->ValueError);
    Fuga_mark_(self, FUGA->SyntaxError);
    Fuga_mark_(self, FUGA->SyntaxUnfinished);
    Fuga_mark_(self, FUGA->MatchError);
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

    FUGA->Path  = Fuga_clone(FUGA->Object);
    FUGA->Thunk = Fuga_clone(FUGA->Object);

    FUGA->Exception         = Fuga_clone(FUGA->Object);
    FUGA->SlotError         = Fuga_clone(FUGA->Exception);
    FUGA->ValueError        = Fuga_clone(FUGA->Exception);
    FUGA->TypeError         = Fuga_clone(FUGA->Exception);
    FUGA->IOError           = Fuga_clone(FUGA->Exception);
    FUGA->SyntaxError       = Fuga_clone(FUGA->Exception);
    FUGA->SyntaxUnfinished  = Fuga_clone(FUGA->SyntaxError);
    FUGA->MatchError        = Fuga_clone(FUGA->Exception);

    FUGA->symbols = FugaSymbols_new(self);

    FugaPrelude_init(FUGA->Prelude);
    FugaInt_init(FUGA->Prelude);
    FugaString_init(FUGA->Prelude);
    FugaSymbol_init(FUGA->Prelude);
    FugaMsg_init(FUGA->Prelude);
    FugaMethod_init(FUGA->Prelude);
    FugaPath_init(FUGA->Prelude);
    FugaThunk_init(FUGA->Prelude);

    Fuga_initObject(FUGA->Prelude);
    Fuga_initBool(FUGA->Prelude);

    void* Loader  = Fuga_getS(FUGA->Prelude, "Loader");
    void* Prelude = FugaLoader_load_(Loader, FUGA_STRING("prelude.fg"));
    if (!Fuga_isRaised(Prelude)) {
        Fuga_delS(Prelude, "Loader");
        Fuga_update_(FUGA->Prelude, Prelude);
    } else {
        Fuga_printException(Prelude);
    }
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

void* Fuga_protoM(void* self) {
    FUGA_NEED(self);
    void* result = Fuga_proto(self);
    if (result)
        return result;
    FUGA_RAISE(FUGA->ValueError, "proto: Object has no proto");
}

void* Fuga_cloneM(void* self, void* args) {
    FUGA_NEED(self);
    FUGA_NEED(args);
    void* result = Fuga_clone(self);
    FUGA_CHECK(result);
    FUGA_HEADER(result)->slots = FUGA_HEADER(args)->slots;
    return result;
}

void Fuga_initObject(void* self) {
    Fuga_setS(FUGA->Object, "str", FUGA_METHOD_STR(Fuga_strSlots));
    Fuga_setS(FUGA->Object, "has", FUGA_METHOD_1(Fuga_has));
    Fuga_setS(FUGA->Object, "get", FUGA_METHOD_1(Fuga_get));
    Fuga_setS(FUGA->Object, "set", FUGA_METHOD_2(Fuga_set));
    Fuga_setS(FUGA->Object, "del", FUGA_METHOD_1(Fuga_del));
    Fuga_setS(FUGA->Object, "hasRaw", FUGA_METHOD_1(Fuga_hasRaw));
    Fuga_setS(FUGA->Object, "getRaw", FUGA_METHOD_1(Fuga_getRaw));
    Fuga_setS(FUGA->Object, "hasName", FUGA_METHOD_1(Fuga_hasName));
    Fuga_setS(FUGA->Object, "getName", FUGA_METHOD_1(Fuga_getName));
    Fuga_setS(FUGA->Object, "hasDoc", FUGA_METHOD_1(Fuga_hasDoc));
    Fuga_setS(FUGA->Object, "getDoc", FUGA_METHOD_1(Fuga_getDoc));
    Fuga_setS(FUGA->Object, "setDoc", FUGA_METHOD_2(Fuga_setDoc));
    Fuga_setS(FUGA->Object, "match",  FUGA_METHOD_1(FugaObject_match_));
    Fuga_setS(FUGA->Object, "len",    FUGA_METHOD_0(Fuga_length));
    Fuga_setS(FUGA->Object, "slots",  FUGA_METHOD_0(Fuga_slots));
    Fuga_setS(FUGA->Object, "dir",    FUGA_METHOD_0(Fuga_dir));
    Fuga_setS(FUGA->Object, "append", FUGA_METHOD_1(Fuga_append_));
    Fuga_setS(FUGA->Object, "extend", FUGA_METHOD_1(Fuga_extend_));
    Fuga_setS(FUGA->Object, "update", FUGA_METHOD_1(Fuga_update_));

    Fuga_setS(FUGA->Object, "eval",   FUGA_METHOD_2(Fuga_eval));
    Fuga_setS(FUGA->Object, "evalIn", FUGA_METHOD_1(Fuga_evalIn));

    Fuga_setS(FUGA->Object, "proto",  FUGA_METHOD_0(Fuga_protoM));
    Fuga_setS(FUGA->Object, "clone",  FUGA_METHOD(Fuga_cloneM));

    // *cough* sorry. this is here, but this location doesn't make sense
    Fuga_set(FUGA->nil,   FUGA_SYMBOL("name"), FUGA_STRING("nil"));

}

void Fuga_initBool(void* self) {
    Fuga_setS(FUGA->True,  "name", FUGA_STRING("true"));
    Fuga_setS(FUGA->False, "name", FUGA_STRING("false"));
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
long Fuga_length(void* self)
{
    ALWAYS(self);
    NEVER(Fuga_isRaised(self));
    NEVER(Fuga_isLazy(self));
    if (FUGA_HEADER(self)->slots)
        return FugaSlots_length(FUGA_HEADER(self)->slots);
    else
        return 0;
}

bool Fuga_hasLength_(void* self, long value)
{
    self = Fuga_need(self);
    if (Fuga_isRaised(self))
        return false;
    return Fuga_length(self) == value;
}

/**
 * Converts a primitive into either an int or a symbol (raising an
 * exception on failure).
 */ 
void* Fuga_toName(void* name, void* self)
{
    ALWAYS(name);
    FUGA_NEED(name);
    if (Fuga_isSymbol(name))
        return name;
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0) {
            name = FUGA_INT(FugaInt_value(name) + Fuga_length(self));
        }
        return name;
    }
    if (Fuga_isString(name))
        return FugaString_toSymbol(name);
    if (Fuga_isMsg(name))
       return FugaMsg_toSymbol(name);
    FUGA_RAISE(FUGA->TypeError,
        "toName: expected primitive int, symbol, string, or msg"
    );
}

#ifdef TESTING
TESTS(Fuga_toName) {
    void* self = Fuga_init();
    TEST(Fuga_isInt   (Fuga_toName(FUGA_INT   (10),  self)));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_SYMBOL("hello"), self)));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_STRING("hello"), self)));
    TEST(Fuga_isRaised(Fuga_toName(FUGA_STRING(""), self)));
    TEST(Fuga_isSymbol(Fuga_toName(FUGA_MSG("hello"), self)));
    TEST(Fuga_isRaised(Fuga_toName(FUGA->Int, self)));
    TEST(Fuga_isRaised(Fuga_toName(Fuga_raise(FUGA_INT(10)), self)));
    Fuga_quit(self);
}
#endif

void* Fuga_proto(
    void* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    return FUGA_HEADER(self)->proto;
}

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

void* Fuga_dir(
    void* self
) {
    FUGA_NEED(self);
    void* dir = Fuga_clone(FUGA->Object);
    long length = Fuga_length(self);
    for (long i = 0; i < length; i++) {
        FUGA_IF(Fuga_hasNameI(self, i))
            FUGA_CHECK(Fuga_append_(dir, Fuga_getNameI(self,i)));
    }
    void* proto = Fuga_proto(self);
    if (proto && !Fuga_isRaised(proto))
        FUGA_CHECK(Fuga_extend_(dir, Fuga_dir(proto)));
    // FIXME: remove duplicates
    return dir;
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
void* Fuga_hasName(void* self, void* name)
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
    if (FugaInt_value(name) >= Fuga_length(self))
        FUGA_RAISE(FUGA->ValueError,
            "hasName_: index >= numSlots"
        );

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot->name)
        return FUGA->True;
    else
        return FUGA->False;
}

void* Fuga_hasDoc(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name, self);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "hasDoc: index < 0"
            );
        if (FugaInt_value(name) >= Fuga_length(self))
            FUGA_RAISE(FUGA->ValueError,
                "hasDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot && slot->doc) {
        return FUGA->True;
    } else if (FUGA_HEADER(self)->proto) {
        return Fuga_hasDoc(FUGA_HEADER(self)->proto, name);
    } else {
        return FUGA->False;
    }
}

/**
 * Does a slot have a name for a given index?
 */
void* Fuga_getName(void* self, void* name)
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
    if (FugaInt_value(name) >= Fuga_length(self))
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

void* Fuga_getDoc(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name, self);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "getDoc: index < 0"
            );
        if (FugaInt_value(name) >= Fuga_length(self))
            FUGA_RAISE(FUGA->ValueError,
                "getDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot && slot->doc)
        return slot->doc;
    if (FUGA_HEADER(self)->proto)
        return Fuga_getDoc(FUGA_HEADER(self)->proto, name);
    FUGA_RAISE(FUGA->SlotError,
        "getDoc: no slot with name"
    );
}
void* Fuga_setDoc(void* self, void* name, void* value)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self); FUGA_NEED(name);
    name = Fuga_toName(name, self);
    FUGA_NEED(name);
    if (Fuga_isInt(name)) {
        if (FugaInt_value(name) < 0)
            FUGA_RAISE(FUGA->ValueError,
                "setDoc: index < 0"
            );
        if (FugaInt_value(name) >= Fuga_length(self))
            FUGA_RAISE(FUGA->ValueError,
                "setDoc: index >= numSlots"
            );
    }

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        slot->doc = value;
    else if (!Fuga_isInt(name) && Fuga_proto(self))
        return Fuga_setDoc(Fuga_proto(self), name, value);
    else 
        FUGA_RAISE(FUGA->SlotError,
            "setDoc: no slot with name"
        );
    return FUGA->nil;
}

/**
 * Look in an object's slots.
 */
void* Fuga_hasRaw(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name, self);
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
TESTS(Fuga_hasRaw) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, a)));

    TEST( Fuga_isTrue  (Fuga_hasRaw(a, FUGA_INT(0))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(b, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_INT(0))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_INT(1))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(b, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_INT(1))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_INT(2))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_INT(42))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(a, FUGA_INT(-1))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_INT(-2))));

    TEST( Fuga_isTrue  (Fuga_hasRaw(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_SYMBOL("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_SYMBOL("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_SYMBOL("c"))));

    TEST( Fuga_isTrue  (Fuga_hasRaw(a, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_STRING("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_STRING("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(b, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_STRING("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_STRING("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_STRING("c"))));

    TEST( Fuga_isTrue  (Fuga_hasRaw(a, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_MSG("a"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_MSG("b"))));
    TEST( Fuga_isTrue  (Fuga_hasRaw(b, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_MSG("b"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(a, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(b, FUGA_MSG("c"))));
    TEST( Fuga_isFalse (Fuga_hasRaw(c, FUGA_MSG("c"))));

    TEST( Fuga_isRaised(Fuga_hasRaw(a, a)));
    TEST( Fuga_isRaised(Fuga_hasRaw(a, FUGA->Int)));
    TEST( Fuga_isRaised(Fuga_hasRaw(a, FUGA_STRING(""))));
    TEST( Fuga_isRaised(Fuga_hasRaw(Fuga_raise(a), FUGA_INT(10))));
    TEST( Fuga_isRaised(Fuga_hasRaw(a, Fuga_raise(FUGA_INT(10)))));


    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(Fuga_isTrue(Fuga_hasRaw(Fuga_lazy_(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_hasRaw(obj, Fuga_lazy_(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_hasRaw(a,Fuga_lazy_(FUGA_SYMBOL("a"),self))));
}
#endif

/**
 * Generic has.
 */ 
void* Fuga_has(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_CHECK(self);

    void* result = Fuga_hasRaw(self, name);
    FUGA_CHECK(result);
    if (Fuga_isTrue(result))
        return result;
    if (!Fuga_isFalse(result))
        FUGA_RAISE(FUGA->TypeError,
            "hasBy_: Expected boolean from hasRawBy_"
        );

    name = Fuga_toName(name, self);
    if (FUGA_HEADER(self)->proto && Fuga_isSymbol(name))
        return Fuga_has(FUGA_HEADER(self)->proto, name);

    return FUGA->False;
}

#ifdef TESTING
TESTS(Fuga_has) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

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
    TEST( Fuga_isTrue  (Fuga_has(a, FUGA_INT(-1))));
    TEST( Fuga_isFalse (Fuga_has(a, FUGA_INT(-2))));

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

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(Fuga_isTrue(Fuga_has(Fuga_lazy_(obj, self), FUGA_INT(0))));
    TEST(Fuga_isTrue(Fuga_has(obj, Fuga_lazy_(FUGA_INT(0), self))));
    TEST(Fuga_isTrue(Fuga_has(a,Fuga_lazy_(FUGA_SYMBOL("a"),self))));
}
#endif



/**
 * Get the value of a slot, without looking in proto.
 */
void* Fuga_getRaw(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name, self);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        return slot->value;

    // raise SlotError
    FugaString *msg = FUGA_STRING("getRaw: no slot named '");
    if (Fuga_isSymbol(name))
        msg = FugaString_cat_(msg, FugaSymbol_toString(name));
    else
        msg = FugaString_cat_(msg, Fuga_str(name));
    msg = FugaString_cat_(msg, FUGA_STRING("'"));
    void* error = Fuga_clone(FUGA->SlotError);
    FUGA_CHECK(Fuga_setS(error, "msg", msg));
    return Fuga_raise(error);
}

#ifdef TESTING
TESTS(Fuga_getRaw) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("a"), a)));
    TEST(!Fuga_isRaised(Fuga_set(a, FUGA_SYMBOL("b"), a)));
    TEST(!Fuga_isRaised(Fuga_set(b, FUGA_SYMBOL("b"), b)));
    TEST(!Fuga_isRaised(Fuga_set(b, NULL, c)));

    TEST( a  ==        (Fuga_getRaw(a, FUGA_INT(0))));
    TEST( b  ==        (Fuga_getRaw(b, FUGA_INT(0))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_INT(0))));
    TEST( a  ==        (Fuga_getRaw(a, FUGA_INT(1))));
    TEST( c  ==        (Fuga_getRaw(b, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_INT(1))));
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_INT(2))));
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_INT(42))));
    TEST( a  ==        (Fuga_getRaw(a, FUGA_INT(-1))))
    TEST( a  ==        (Fuga_getRaw(a, FUGA_INT(-2))))
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_INT(-3))));

    TEST( a  ==        (Fuga_getRaw(a, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_SYMBOL("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_SYMBOL("a"))));
    TEST( a  ==        (Fuga_getRaw(a, FUGA_SYMBOL("b"))));
    TEST( b  ==        (Fuga_getRaw(b, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_SYMBOL("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_SYMBOL("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_SYMBOL("c"))));

    TEST( a  ==        (Fuga_getRaw(a, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_STRING("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_STRING("a"))));
    TEST( a  ==        (Fuga_getRaw(a, FUGA_STRING("b"))));
    TEST( b  ==        (Fuga_getRaw(b, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_STRING("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_STRING("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_STRING("c"))));

    TEST( a  ==        (Fuga_getRaw(a, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_MSG("a"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_MSG("a"))));
    TEST( a  ==        (Fuga_getRaw(a, FUGA_MSG("b"))));
    TEST( b  ==        (Fuga_getRaw(b, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_MSG("b"))));
    TEST( Fuga_isRaised(Fuga_getRaw(a, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(b, FUGA_MSG("c"))));
    TEST( Fuga_isRaised(Fuga_getRaw(c, FUGA_MSG("c"))));

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(prim == Fuga_getRaw(Fuga_lazy_(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_getRaw(obj, Fuga_lazy_(FUGA_INT(0), self)));
    TEST(a == Fuga_getRaw(a, Fuga_lazy_(FUGA_SYMBOL("a"), self)));
}
#endif

/**
 * Generic get.
 */
void* Fuga_get(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name, self);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        return slot->value;

    if (FUGA_HEADER(self)->proto && Fuga_isSymbol(name))
        return Fuga_get(FUGA_HEADER(self)->proto, name);

    // raise SlotError
    FugaString *msg = FUGA_STRING("get: no slot named '");
    if (Fuga_isSymbol(name))
        msg = FugaString_cat_(msg, FugaSymbol_toString(name));
    else
        msg = FugaString_cat_(msg, Fuga_str(name));
    msg = FugaString_cat_(msg, FUGA_STRING("'"));
    void* error = Fuga_clone(FUGA->SlotError);
    FUGA_CHECK(Fuga_setS(error, "msg", msg));
    return Fuga_raise(error);
}

#ifdef TESTING
TESTS(Fuga_get) {
    void* self = Fuga_init();
    
    void* a = Fuga_clone(FUGA->Object);
    void* b = Fuga_clone(a);
    void* c = Fuga_clone(b);

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
    TEST( a  ==        (Fuga_get(a, FUGA_INT(-1))));
    TEST( a  ==        (Fuga_get(a, FUGA_INT(-2))));
    TEST( Fuga_isRaised(Fuga_get(a, FUGA_INT(-3))));

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

    void* prim = FUGA_INT(10);
    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(prim == Fuga_get(Fuga_lazy_(obj, self), FUGA_INT(0)));
    TEST(prim == Fuga_get(obj, Fuga_lazy_(FUGA_INT(0), self)));
    TEST(a == Fuga_get(a, Fuga_lazy_(FUGA_SYMBOL("a"), self)));

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
void* Fuga_set(void* self, void* name, void* value)
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
    name = Fuga_toName(name, self);
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
TESTS(Fuga_set) {
    void* self = Fuga_init();
    void* a = Fuga_clone(FUGA->Object);

    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(0), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(1), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(0), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_INT(3), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, NULL,        a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(3), a)) );
    TEST( Fuga_isNil    (Fuga_set(a, FUGA_INT(-1), a)) );
    TEST( Fuga_isRaised (Fuga_set(a, FUGA_INT(-5), a)) );

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

void* Fuga_del(void* self, void* name)
{
    ALWAYS(self); ALWAYS(name);
    FUGA_NEED(self);
    name = Fuga_toName(name, self);
    FUGA_CHECK(name);

    FugaSlot* slot = Fuga_getSlot_(self, name);
    if (slot)
        FugaSlots_delByIndex(FUGA_HEADER(self)->slots, slot->index);
    return FUGA->nil;
}

void* Fuga_hasS         (void* self, const char* name)
    { return Fuga_has        (self, FUGA_SYMBOL(name));   }
void* Fuga_hasRawS      (void* self, const char* name)
    { return Fuga_hasRaw     (self, FUGA_SYMBOL(name));   }
void* Fuga_hasDocS      (void* self, const char* name)
    { return Fuga_hasDoc     (self, FUGA_SYMBOL(name));   }
void* Fuga_getS         (void* self, const char* name)
    { return Fuga_get        (self, FUGA_SYMBOL(name));   }
void* Fuga_getRawS      (void* self, const char* name)
    { return Fuga_getRaw     (self, FUGA_SYMBOL(name));   }
void* Fuga_getDocS      (void* self, const char* name)
    { return Fuga_getDoc     (self, FUGA_SYMBOL(name));   }
void* Fuga_setS      (void* self, const char* name, void* value)
    { return Fuga_set     (self, FUGA_SYMBOL(name), value);   }
void* Fuga_setDocS   (void* self, const char* name, void* value)
    { return Fuga_setDoc  (self, FUGA_SYMBOL(name), value);   }
void* Fuga_delS         (void* self, const char* name)
    { return Fuga_del        (self, FUGA_SYMBOL(name));   }


void* Fuga_hasI       (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_has        (self, FUGA_INT(index));   }
void* Fuga_hasNameI   (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_hasName    (self, FUGA_INT(index));   }
void* Fuga_hasDocI    (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_hasDoc     (self, FUGA_INT(index));   }
void* Fuga_getI       (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_get        (self, FUGA_INT(index));   }
void* Fuga_getNameI   (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_getName    (self, FUGA_INT(index));   }
void* Fuga_getDocI    (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_getDoc     (self, FUGA_INT(index));   }
void* Fuga_setI    (void* self, long index, void* value)
    { FUGA_CHECK(self);
      return Fuga_set     (self, FUGA_INT(index), value);   }
void* Fuga_setDocI (void* self, long index, void* value)
    { FUGA_CHECK(self);
      return Fuga_setDoc  (self, FUGA_INT(index), value);   }
void* Fuga_delI       (void* self, long index)
    { FUGA_CHECK(self);
      return Fuga_del        (self, FUGA_INT(index));   }

void* Fuga_extend_(
    void* self,
    void* other
) {
    ALWAYS(self);       ALWAYS(other);
    FUGA_NEED(self);    FUGA_NEED(other);
    FUGA_FOR(i, slot, other) {
        FUGA_CHECK(Fuga_append_(self, slot));
    }
    return FUGA->nil;
}

void* Fuga_update_(
    void* self,
    void* other
) {
    ALWAYS(self);       ALWAYS(other);
    FUGA_NEED(self);    FUGA_NEED(other);
    FUGA_FOR(i, slot, other) {
        if (Fuga_isTrue(Fuga_hasNameI(other, i))) {
            void* name = Fuga_getNameI(other, i);
            FUGA_CHECK(Fuga_set(self, name, slot));
        }
    }
    return FUGA->nil;
}


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
    if (Fuga_length(args))
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

    void* method = Fuga_get(self, name);
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
    TEST(!Fuga_isRaised(Fuga_set(scope, FUGA_SYMBOL("hello"),prim)));
    TEST(Fuga_is_(prim, Fuga_eval(FUGA_MSG("hello"), scope, scope)));

    void* obj = Fuga_clone(FUGA->Object);
    TEST(!Fuga_isRaised(Fuga_append_(obj, prim)));
    TEST(!Fuga_isRaised(Fuga_append_(obj, FUGA_MSG("hello"))));
    TEST(!Fuga_isRaised(Fuga_set(obj, FUGA_SYMBOL("hi"), prim)));
    void* eobj = Fuga_eval(obj, scope, scope);
    TEST(!Fuga_isRaised(eobj));
    TEST(Fuga_hasLength_((eobj), 3));
    TEST(prim == Fuga_get(eobj, FUGA_INT(0)));
    TEST(prim == Fuga_get(eobj, FUGA_INT(1)));
    TEST(prim == Fuga_get(eobj, FUGA_INT(2)));

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
    void* escope = Fuga_clone(scope);
    void* result = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_setS(escope, "_this", result));
    FUGA_FOR(i, slot, self) { 
        FUGA_IF(Fuga_hasDocI(self, i)) {
            void* doc = Fuga_getDocI(self, i);
            FUGA_CHECK(Fuga_setS(escope, "_doc", doc));
        } else {
            FUGA_CHECK(Fuga_delS(escope, "_doc"));
        }
        void* value = Fuga_eval(slot, escope, escope);
        FUGA_CHECK(value);
        if (!Fuga_isNil(value))
            FUGA_CHECK(Fuga_append_(result, value));
    }
    return result;
}

void* Fuga_evalIn(void* self, void* scope)
{
    ALWAYS(self); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(scope);
    void* escope = Fuga_clone(scope);
    FUGA_CHECK(Fuga_setS(escope, "_this", scope));
    void* value = FUGA->nil;
    FUGA_FOR(i, slot, self) {
        FUGA_IF(Fuga_hasDocI(self, i)) {
            void* doc = Fuga_getDocI(self, i);
            FUGA_CHECK(Fuga_setS(escope, "_doc", doc));
        } else {
            FUGA_CHECK(Fuga_delS(escope, "_doc"));
        }
        FUGA_CHECK(value = Fuga_eval(slot, escope, escope));
    }
    return value;
}

void* Fuga_evalExpr(
    void* self,
    void* recv,
    void* scope
) {
    ALWAYS(self); ALWAYS(recv); ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);
    long length = Fuga_length(self);
    if (length < 1) {
        FUGA_RAISE(FUGA->ValueError,
            "Expr eval: can't evaluate empty expression"
        );
    }
    for (long i = 0; i < length; i++) {
        void* part = Fuga_get(self, FUGA_INT(i));
        recv = Fuga_eval(part, recv, scope);
        FUGA_CHECK(recv);
    }
    return recv;
}

void* Fuga_evalModule(
    void* self
) {
    return Fuga_evalModule_(self, NULL);
}

void* Fuga_evalModule_(
    void* self,
    const char* filename
) {
    ALWAYS(self); FUGA_NEED(self);
    void* module = Fuga_clone(FUGA->Prelude);

    if (filename) {
        void* loader = Fuga_clone(Fuga_getS(FUGA->Prelude, "Loader"));
        FugaPath* local = FugaPath_new(FUGA_STRING(filename));
        local = FugaPath_parent(local);
        FUGA_CHECK(loader);
        FUGA_CHECK(local);
        FUGA_CHECK(FugaLoader_setLocal_(loader, local));
        FUGA_CHECK(Fuga_setS(module, "Loader", loader));
    }

    FUGA_CHECK(Fuga_evalIn(self, module));
    return module;
}

void* Fuga_load_(
    void* self,
    const char* filename
) {
    FugaParser *parser = FugaParser_new(self);
    if (!FugaParser_readFile_(parser, filename)) 
        FUGA_RAISE(FUGA->IOError, "can't load module");
    void* block  = FugaParser_block(parser);
    // FIXME: ensure EOF
    return Fuga_evalModule_(block, filename);

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

    FUGA_STR_TEST(Fuga_get(FUGA->Int, FUGA_SYMBOL("str")),
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
    long length = Fuga_length(self);
    void* result = FUGA_STRING("(");
    for (size_t slotNum = 0; slotNum < length; slotNum++) {
        void* fSlotNum = FUGA_INT(slotNum);
        if (Fuga_isTrue(Fuga_hasName(self, fSlotNum))) {
            void* name = Fuga_getName(self, fSlotNum);
            name = FugaSymbol_toString(name);
            FUGA_CHECK(name);
            result = FugaString_cat_(result, name);
            result = FugaString_cat_(result, FUGA_STRING(" = "));
        }
        FUGA_CHECK(result);
        void* slot = Fuga_get(self, FUGA_INT(slotNum));
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
    void *msg = Fuga_get(self, FUGA_SYMBOL("msg"));
    printf("EXCEPTION:\n\t");
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

void* Fuga_match_(void* self, void* attempt)
{
    FUGA_NEED(self);
    FUGA_CHECK(attempt);
    void* arg = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_append_(arg, attempt));
    return Fuga_send(self, FUGA_SYMBOL("match"), arg);
}

void* FugaObject_match_(void* self, void* attempt)
{
    FUGA_NEED(self);
    FUGA_CHECK(attempt);

    // check for lazySlots
    FUGA_FOR(i, slot1, self) {
        if (FugaMsg_is_(slot1, "~")) {
            attempt = Fuga_lazySlots(attempt);
            break;
        }
    }

    FUGA_NEED(attempt);
    // FIXME: allow varargs
    if (!Fuga_hasLength_(self, Fuga_length(attempt)))
        FUGA_RAISE(FUGA->MatchError, "different lengths");
    void* result = Fuga_clone(FUGA->Object);
    FUGA_FOR(i, slot, self) {
        void* resultSlot = Fuga_match_(slot, Fuga_getI(attempt, i));
        FUGA_CHECK(resultSlot);
        FUGA_CHECK(Fuga_update_(result, resultSlot));
    }
    return result;
}

