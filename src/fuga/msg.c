#include "msg.h"
#include "test.h"

void FugaMsg_init(Fuga* self)
{
    Fuga_setSlot(FUGA->Msg, FUGA_SYMBOL("str"),
        FUGA_METHOD_STR(FugaMsg_str));
}

Fuga* FugaMsg_new(Fuga* self, const char* name)
{
    ALWAYS(self); ALWAYS(name);
    return FugaMsg_fromSymbol(FUGA_SYMBOL(name));
}

Fuga* FugaMsg_fromSymbol(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isSymbol(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg toSymbol: expected primitive symbol"
        );

    Fuga* name = self;

    self = Fuga_clone(FUGA->Msg);
    self->type = FUGA_TYPE_MSG;
    self->size = 1;
    self->data = name;

    return self;
}

Fuga* FugaMsg_toSymbol(Fuga* self)
{
    Fuga* name = FugaMsg_name(self);
    FUGA_NEED(name);
    if (!Fuga_isSymbol(name))
        FUGA_RAISE(FUGA->TypeError,
            "Msg toSymbol: expected a symbol msg, not an int msg"
        );
    return name;
}

Fuga* FugaMsg_name(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg name: expected primitive msg"
        );
    return self->data;
}

Fuga* FugaMsg_args(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg args: expected primitive msg"
        );

    Fuga* args = Fuga_clone(FUGA->Object);
    args->slots = self->slots;
    return args;
}

Fuga* FugaMsg_eval(Fuga* self, Fuga* recv, Fuga* scope)
{
    ALWAYS(self);    ALWAYS(recv);    ALWAYS(scope);
    FUGA_NEED(self); FUGA_NEED(recv); FUGA_NEED(scope);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg eval: expected primitive msg"
        );

    Fuga* name = FugaMsg_name(self);
    Fuga* args = Fuga_thunk(FugaMsg_args(self), scope);
    return Fuga_send(recv, name, args);
}

Fuga* FugaMsg_str(Fuga* self)
{
    Fuga* name = FugaMsg_name(self);
    Fuga* namestr = Fuga_str(name);
    FUGA_NEED(namestr);
    if (Fuga_isSymbol(name))
        namestr = FugaString_sliceFrom(namestr, 1);

    if (FugaInt_isEqualTo(Fuga_numSlots(self), 0)) {
        return namestr;
    } else {
        Fuga* argsstr = Fuga_strSlots(self);
        return FugaString_cat(namestr, argsstr);
    }
}

