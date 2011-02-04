
#include <string.h>
#include "string.h"
#include "test.h"
#include "symbol.h"

Fuga* FugaString_new(Fuga* self, const char* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_CHECK(self);
    self = Fuga_clone(FUGA->String);
    self->type = FUGA_TYPE_STRING;
    self->size = strlen(value)+1;
    self->data = FugaGC_alloc(self, self->size);
    strcpy(self->data, value);
    return self;
}

#ifdef TESTING
TESTS(FUGA_STRING) {
    Fuga* self = Fuga_init();
    TEST(Fuga_isString(FUGA_STRING("")));
    TEST(Fuga_isString(FUGA_STRING("hello")));
    Fuga_quit(self);
}
#endif


Fuga* FugaString_toSymbol(Fuga* self)
{
    ALWAYS(self);
    ALWAYS(Fuga_isString(self)); // better to return

    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String toSymbol: expected primitive string"
        );
    }

    if (!FugaSymbol_isValid(self->data)) {
        FUGA_RAISE(FUGA->TypeError,
            "String toSymbol: invalid name"
        );
    }

    return FUGA_SYMBOL(self->data);
}

