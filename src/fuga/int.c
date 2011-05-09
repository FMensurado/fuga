#include "test.h"
#include "int.h"
#include <stdio.h>

void FugaInt_init(void* self)
{
    Fuga_setS(FUGA->Int, "_name", FUGA_STRING("Int"));
    Fuga_setS(FUGA->Int, "str",   FUGA_METHOD_STR(FugaInt_str));
    Fuga_setS(FUGA->Int, "match", FUGA_METHOD_1(FugaInt_match_));
    Fuga_setS(FUGA->Int, "input", FUGA_METHOD(FugaInt_input));
    Fuga_setS(FUGA->Int, "+",     FUGA_METHOD(FugaInt_addMethod));
    Fuga_setS(FUGA->Int, "-",     FUGA_METHOD(FugaInt_subMethod));
    Fuga_setS(FUGA->Int, "*",     FUGA_METHOD_1(FugaInt_mul));
    Fuga_setS(FUGA->Int, "//",    FUGA_METHOD_1(FugaInt_fdiv));
    Fuga_setS(FUGA->Int, "%",     FUGA_METHOD_1(FugaInt_mod));
    Fuga_setS(FUGA->Int, "==",    FUGA_METHOD_1(FugaInt_eq));
    Fuga_setS(FUGA->Int, "!=",    FUGA_METHOD_1(FugaInt_neq));
    Fuga_setS(FUGA->Int, "<",     FUGA_METHOD_1(FugaInt_lt));
    Fuga_setS(FUGA->Int, ">",     FUGA_METHOD_1(FugaInt_gt));
    Fuga_setS(FUGA->Int, "<=",    FUGA_METHOD_1(FugaInt_le));
    Fuga_setS(FUGA->Int, ">=",    FUGA_METHOD_1(FugaInt_ge));
}

const FugaType FugaInt_type = {
    "Int"
};

FugaInt* FugaInt_new_(
    void* self,
    long value
) {
    ALWAYS(self);
    FugaInt* result = Fuga_clone_(FUGA->Int, sizeof(FugaInt));
    Fuga_type_(result, &FugaInt_type);
    result->value = value;
    return result;
}

long FugaInt_value(FugaInt* self)
{
    ALWAYS(self);
    ALWAYS(Fuga_isInt(self));
    return self->value;
}

bool FugaInt_is_(FugaInt* self, long value)
{
    ALWAYS(self);
    return Fuga_isInt(self) && (FugaInt_value(self) == value);
}

void* FugaInt_str(void* _self)
{
    FugaInt* self = _self;
    ALWAYS(self);
    FUGA_NEED(self);
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int str: expected primitive int");
    char revbuffer[1024] = {0};
    char buffer[1024] = {0};
    size_t i = 0;
    size_t j = 0;
    long value = self->value;
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

void* FugaInt_match_(FugaInt* self, FugaInt* other) 
{
    FUGA_NEED(self);
    FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->MatchError, "Int match: matches only on ints");
    if (self->value != other->value)
        FUGA_RAISE(FUGA->MatchError, "Int match: values don't match");
    return Fuga_clone(FUGA->Object);
}

void* FugaInt_addMethod(void* _self, void* args)
{
    FugaInt* self = _self;
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(args);
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive int");

    if (Fuga_hasLength_(args, 0)) {
        return self;
    } else if (Fuga_hasLength_(args, 1)) {
        FugaInt* other = Fuga_getI(args, 0);
        FUGA_NEED(other);
        if (!Fuga_isInt(other))
            FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive int");
        return FUGA_INT(FugaInt_value(self) + FugaInt_value(other));
    } else {
        FUGA_RAISE(FUGA->TypeError, "Int +: expected 0 or 1 args");
    }
}

void* FugaInt_subMethod(void* _self, void* args)
{
    FugaInt* self = _self;
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self); FUGA_NEED(args);
    if (!Fuga_isInt(self))
        FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive int");

    if (Fuga_hasLength_(args, 0)) {
        return FUGA_INT(-FugaInt_value(self));
    } else if (Fuga_hasLength_(args, 1)) {
        FugaInt* other = Fuga_getI(args, 0);
        FUGA_NEED(other);
        if (!Fuga_isInt(other))
            FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive int");
        return FUGA_INT(FugaInt_value(self) - FugaInt_value(other));
    } else {
        FUGA_RAISE(FUGA->TypeError, "Int -: expected 0 or 1 args");
    }
}

void* FugaInt_add(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int +: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) + FugaInt_value(other));
}

void* FugaInt_sub(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int -: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) - FugaInt_value(other));
}

void* FugaInt_mul(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int *: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) * FugaInt_value(other));
}

void* FugaInt_fdiv(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int //: expected primitive ints");
    if (FugaInt_value(other) == 0)
        FUGA_RAISE(FUGA->ValueError, "Int //: Division by zero.");
    return FUGA_INT(FugaInt_value(self) / FugaInt_value(other));
}

void* FugaInt_mod(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int %: expected primitive ints");
    return FUGA_INT(FugaInt_value(self) % FugaInt_value(other));
}

void* FugaInt_eq(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int ==: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) == FugaInt_value(other));
}

void* FugaInt_neq(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int !=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) != FugaInt_value(other));
}

void* FugaInt_lt(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int <: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) < FugaInt_value(other));
}

void* FugaInt_gt(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int >: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) > FugaInt_value(other));
}

void* FugaInt_le(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int <=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) <= FugaInt_value(other));
}

void* FugaInt_ge(void* _self, void* _other)
{
    FugaInt* self = _self;
    FugaInt* other = _other;
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
    if (!Fuga_isInt(self) || !Fuga_isInt(other))
        FUGA_RAISE(FUGA->TypeError, "Int >=: expected primitive ints");
    return FUGA_BOOL(FugaInt_value(self) >= FugaInt_value(other));
}

void* FugaInt_input(void* self, void* args) {
	FUGA_NEED(self); FUGA_NEED(args);
	long value;
	if (Fuga_hasLength_(args, 0)) {
		if(!scanf("%ld", &value))
            FUGA_RAISE(FUGA->IOError,
                "Int input: failed at reading int."
            );
		return FUGA_INT(value);
	} else if (Fuga_hasLength_(args, 1)) {
		void* arg = Fuga_getI(args, 0);
		if (Fuga_isString(arg)) {
			FugaString* str = arg;
			printf("%s", str->data);
		    if(!scanf("%ld", &value))
                FUGA_RAISE(FUGA->IOError,
                    "Int input: failed at reading int."
                );
			return FUGA_INT(value);
		} else {
			FUGA_RAISE(FUGA->TypeError,
				"Int input: argument must be a string"
			);
		}
	} else {
		FUGA_RAISE(FUGA->TypeError,
			"Int input: expected only 0 or 1 arguments"
		);
	}
}
