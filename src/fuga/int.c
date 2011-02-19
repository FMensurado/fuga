#include "test.h"
#include "int.h"

void FugaInt_init(Fuga* self)
{
    Fuga_set(FUGA->Int, FUGA_SYMBOL("str"),
        FugaMethod_strMethod(self, FugaInt_str));
}

Fuga* FugaInt_new(Fuga* self, long value)
{
    ALWAYS(self);
    self = Fuga_clone(self->root->Int);
    self->type = FUGA_TYPE_INT;
    self->data = (void*)value;
    return self;
}

long FugaInt_value(Fuga* self)
{
    ALWAYS(self);
    ALWAYS(Fuga_isInt(self));
    return (long)self->data;
}

bool FugaInt_isEqualTo(Fuga* self, long value)
{
    ALWAYS(self);
    return Fuga_isInt(self) && (FugaInt_value(self) == value);
}

Fuga* FugaInt_str(Fuga* self)
{
    ALWAYS(self);
    FUGA_NEED(self);
    if (Fuga_isTrue(Fuga_rawHas(self, FUGA_SYMBOL("name"))))
        return Fuga_rawGet(self, FUGA_SYMBOL("name"));
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int str: expected primitive int");
    char revbuffer[1024] = {0};
    char buffer[1024] = {0};
    size_t i = 0;
    size_t j = 0;
    long value = (long)self->data;
    if (value == 0)
        return FUGA_STRING("0");
    if (value < 0) {
        buffer[j++] = '-';
        value = -value;
    }
    while (value > 0) {
        revbuffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0)
        buffer[j++] = revbuffer[--i];
    buffer[j++] = 0;
    return FUGA_STRING(buffer);
}
