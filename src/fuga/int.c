#include "test.h"
#include "int.h"

Fuga* FugaInt_proto(Fuga* self) {
    return self->root->Int;
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
    return Fuga_isInt(self) && (FugaInt_value(self) == value);
}

