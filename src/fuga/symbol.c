#include <string.h>
#include "symbol.h"
#include "symbols.h"
#include "test.h"
#include "char.h"

void FugaSymbol_init(Fuga* self)
{
    Fuga_setSlot(FUGA->Symbol, FUGA_SYMBOL("str"),
        FUGA_METHOD_STR(FugaSymbol_str));
}

bool FugaSymbol_isValid(const char* value)
{
    ALWAYS(value);
    return *value;
    // FIXME: determine what makes a valid symbol.
}

Fuga* FugaSymbol_new(Fuga* self, const char* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_CHECK(self);
    ALWAYS(FugaSymbol_isValid(value));
    
    Fuga* result = FugaSymbols_get(FUGA->symbols, value);
    if (result) return result;

    self = Fuga_clone(FUGA->Symbol);
    self->type = FUGA_TYPE_SYMBOL;
    self->size = strlen(value)+1;
    self->data = FugaGC_alloc(self, self->size);
    strcpy(self->data, value);
    FugaSymbols_set(FUGA->symbols, value, self);
    return self;
}

#ifdef TESTING
TESTS(FUGA_SYMBOL) {
    Fuga* self = Fuga_init();
    TEST(Fuga_isSymbol(FUGA_SYMBOL("hello")));
    Fuga_quit(self);
}
#endif

Fuga* FugaSymbol_str(Fuga* self) {
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isSymbol(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "Symbol str: expected primitive symbol"
        );
    }
    
    char buffer[self->size+2];
    size_t index=0;
    buffer[index++] = ':';
    if (FugaChar_isOp(self->data))
        buffer[index++] = '\\';
    memcpy(buffer+index, self->data, self->size);
    return FUGA_STRING(buffer);
}

Fuga* FugaSymbol_toString(Fuga* self) {
    Fuga* result = FugaSymbol_str(self);
    return FugaString_sliceFrom(result, 1);
}

