#include "lazy.h"
#include "test.h"

const FugaType FugaLazy_type = {
    .name = "lazy"
};

void FugaLazy_mark(void* _self) {
    FugaLazy* self = _self;
    Fuga_mark_(self, self->code);
    Fuga_mark_(self, self->scope);
}

void* Fuga_lazy_(void* self, void* scope)
{
    ALWAYS(self); ALWAYS(scope);
    FUGA_NEED(self); FUGA_CHECK(scope);
    FugaLazy *thunk = Fuga_clone_(FUGA->Object, sizeof(FugaLazy));
    Fuga_type_(thunk, &FugaLazy_type);
    Fuga_onMark_(thunk, FugaLazy_mark);
    thunk->code  = self;
    thunk->scope = scope;
    return thunk;
}

/**
 * Evaluate a thunk. (If the result is a thunk, evaluate it again
 * and again and again...)
 */
void* Fuga_need(void* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    while (Fuga_isLazy(self)) {
        self = Fuga_needOnce(self);
        FUGA_CHECK(self);
    }
    return self;
}


/**
 * Evaluate a thunk, but if the result is a thunk, return it without
 * evaluating it.
 */
void* Fuga_needOnce(FugaLazy* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (Fuga_isLazy(self)) {
        if (self->scope) {
            void* res = Fuga_eval(self->code, self->scope, self->scope);
            FUGA_CHECK(res);
            self->code  = res;
            self->scope = NULL;
        }
        return self->code;
    }
    return self;
}

void* Fuga_lazyCode(FugaLazy* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isLazy(self))
        FUGA_RAISE(FUGA->TypeError, "c.thunkCode: not a thunk");
    return self->code;
}

void* Fuga_lazyScope(FugaLazy* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isLazy(self))
        FUGA_RAISE(FUGA->TypeError, "c.thunkScope: not a thunk");
    return self->scope;
}

void* Fuga_lazySlots(void* _self)
{
    FugaLazy* self = _self;
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isLazy(self))
        return Fuga_slots(self);
    if (!self->scope)
        return Fuga_lazySlots(self->code);
    void* result = Fuga_clone(FUGA->Object);
    void* code   = self->code;
    void* scope  = self->scope;
    FUGA_CHECK(code); FUGA_CHECK(scope);

    size_t numSlots = FugaInt_value(Fuga_length(code));
    for (size_t i = 0; i < numSlots; i++) {
        void* slot = Fuga_get(code, FUGA_INT(i));
        FUGA_CHECK(slot);
        // FIXME: handle Thunk ~
        FUGA_CHECK(Fuga_append_(result, Fuga_lazy_(slot, scope)));
    }

    return result;
}
