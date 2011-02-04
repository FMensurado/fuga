#include "msg.h"
#include "test.h"

Fuga* FugaMsg_new(Fuga* self, const char* name)
{
    ALWAYS(self); ALWAYS(name);
    return FugaMsg_fromSymbol(FUGA_SYMBOL(name));
}

Fuga* FugaMsg_fromSymbol(Fuga* self)
{
    ALWAYS(self);
    FUGA_CHECK(self);
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
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isMsg(self))
        FUGA_RAISE(FUGA->TypeError,
            "Msg toSymbol: expected primitive msg"
        );

    return self->data;
}

