#include <string.h>
#include "symbol.h"
#include "symbols.h"
#include "test.h"

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

    self = Fuga_clone(FUGA->String);
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
