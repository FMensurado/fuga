#include "test.h"
#include "int.h"

void FugaInt_init(Fuga* self)
{
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("str"),
        FUGA_METHOD_STR(FugaInt_str));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("+"),
        FUGA_METHOD(FugaInt_addMethod));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("-"),
        FUGA_METHOD(FugaInt_subMethod));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("*"),
        FUGA_METHOD_1ARG(FugaInt_mul));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("//"),
        FUGA_METHOD_1ARG(FugaInt_fdiv));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("%"),
        FUGA_METHOD_1ARG(FugaInt_mod));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("=="),
        FUGA_METHOD_1ARG(FugaInt_eq));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("!="),
        FUGA_METHOD_1ARG(FugaInt_neq));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("<"),
        FUGA_METHOD_1ARG(FugaInt_lt));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL(">"),
        FUGA_METHOD_1ARG(FugaInt_gt));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL("<="),
        FUGA_METHOD_1ARG(FugaInt_le));
    Fuga_setSlot(FUGA->Int, FUGA_SYMBOL(">="),
        FUGA_METHOD_1ARG(FugaInt_ge));
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

Fuga* FugaInt_addMethod(Fuga* self, Fuga* args)
{
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(args);
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive int");

    Fuga* numSlots = Fuga_numSlots(args);
    if (FugaInt_isEqualTo(numSlots, 0)) {
        return self;
    } else if (FugaInt_isEqualTo(numSlots, 1)) {
        Fuga* other = Fuga_getSlot(args, FUGA_INT(0));
        if (!Fuga_isInt(other))
            FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive int");
        return FUGA_INT(FugaInt_value(self) + FugaInt_value(other));
    } else {
        FUGA_RAISE(FUGA->TypeError, "Int +: expected 0 or 1 args");
    }
}

Fuga* FugaInt_subMethod(Fuga* self, Fuga* args)
{
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(args);
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive int");

    Fuga* numSlots = Fuga_numSlots(args);
    if (FugaInt_isEqualTo(numSlots, 0)) {
        return FUGA_INT(-FugaInt_value(self));
    } else if (FugaInt_isEqualTo(numSlots, 1)) {
        Fuga* other = Fuga_getSlot(args, FUGA_INT(0));
        FUGA_NEED(other);
        if (!Fuga_isInt(other))
            FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive int");
        return FUGA_INT(FugaInt_value(self) - FugaInt_value(other));
    } else {
        FUGA_RAISE(FUGA->TypeError, "Int -: expected 0 or 1 args");
    }
}

Fuga* FugaInt_add(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) + FugaInt_value(other));
}

Fuga* FugaInt_sub(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) - FugaInt_value(other));
}

Fuga* FugaInt_mul(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int *: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) * FugaInt_value(other));
}

Fuga* FugaInt_fdiv(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int //: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) / FugaInt_value(other));
}

Fuga* FugaInt_mod(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int %: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) % FugaInt_value(other));
}

Fuga* FugaInt_eq(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int ==: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) == FugaInt_value(other));
}

Fuga* FugaInt_neq(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int !=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) != FugaInt_value(other));
}

Fuga* FugaInt_lt(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int <: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) < FugaInt_value(other));
}

Fuga* FugaInt_gt(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int >: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) > FugaInt_value(other));
}

Fuga* FugaInt_le(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int <=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) <= FugaInt_value(other));
}

Fuga* FugaInt_ge(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int >=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) >= FugaInt_value(other));
}

