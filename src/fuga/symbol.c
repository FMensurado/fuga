#include <string.h>
#include "symbol.h"
#include "symbols.h"
#include "test.h"
#include "char.h"

const FugaType FugaSymbol_type = {
    "Symbol"
};

void FugaSymbol_init(
    void* self
) {
    Fuga_setS(FUGA->Symbol, "_name", FUGA_STRING("Symbol"));
    Fuga_setS(FUGA->Symbol, "str", FUGA_METHOD_STR(FugaSymbol_str));
    Fuga_setS(FUGA->Symbol, "match", FUGA_METHOD_1(FugaSymbol_match_));
}

bool FugaSymbol_is_(
    FugaSymbol* self,
    const char* value
) {
    ALWAYS(self);
    return !Fuga_isRaised(self)
        &&  Fuga_isSymbol(self)
        && (strcmp(self->data, value) == 0);
}

bool FugaSymbol_isValid(
    const char* value
) {
    ALWAYS(value);
    return *value;
    // FIXME: determine what makes a valid symbol.
}

FugaSymbol* FugaSymbol_new_(
    void* self,
    const char* value
) {
    ALWAYS(self); ALWAYS(value);
    FUGA_CHECK(self);
    ALWAYS(FugaSymbol_isValid(value));
    
    FugaSymbol* result = FugaSymbols_get(FUGA->symbols, value);
    if (result)
        return result;

    size_t size = strlen(value);
    result = Fuga_clone_(FUGA->Symbol, sizeof(FugaSymbol) + size + 1);
    Fuga_type_(result, &FugaSymbol_type);
    result->size   = size;
    result->length = size; // FIXME: determine actual length
    memcpy(result->data, value, size+1);
    FugaSymbols_set(FUGA->symbols, value, result);
    return result;
}

#ifdef TESTING
TESTS(FUGA_SYMBOL) {
    void* self = Fuga_init();
    TEST(Fuga_isSymbol(FUGA_SYMBOL("hello")));
    Fuga_quit(self);
}
#endif

void* FugaSymbol_str(void* _self) {
    FugaSymbol* self = _self;
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isSymbol(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "Symbol str: expected primitive symbol"
        );
    }
    
    char buffer[self->size+3];
    size_t index=0;
    buffer[index++] = ':';
    if (FugaChar_isOp(self->data))
        buffer[index++] = '\\';
    memcpy(buffer+index, self->data, self->size+1);
    return FUGA_STRING(buffer);
}

void* FugaSymbol_toString(void* self) {
    void* result = FugaSymbol_str(self);
    return FugaString_from_(result, 1);
}

void* FugaSymbol_match_(FugaSymbol* self, FugaSymbol* other) 
{
    FUGA_NEED(self);
    FUGA_NEED(other);
    if (!Fuga_isSymbol(self) || !Fuga_isSymbol(other))
        FUGA_RAISE(FUGA->MatchError, "Symbol match: matches only on syms");
    if (!Fuga_is_(self, other))
        FUGA_RAISE(FUGA->MatchError, "Symbol match: values don't match");
    return Fuga_clone(FUGA->Object);
}

