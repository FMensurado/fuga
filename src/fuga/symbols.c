#include "symbols.h"
#include "test.h"
#include <string.h>

struct FugaSymbols {
    FugaSymbols* tree[256];
};

void FugaSymbols_mark(void* _self) 
{
    FugaSymbols* self = _self;
    for (size_t i = 0; i < 256; i++)
        Fuga_mark_(self, self->tree[i]);
}

FugaSymbols* FugaSymbols_new(void* self)
{
    FugaSymbols* syms = Fuga_clone_(FUGA->Object, sizeof(FugaSymbols));
    Fuga_onMark_(syms, FugaSymbols_mark);
    return syms;
}

FugaSymbol* FugaSymbols_get(FugaSymbols* self, const char* name)
{
    ALWAYS(name);
    for (; *name; name++) {
        if (!self)
            return NULL;
        self = self->tree[(size_t)*name];
    }
    if (!self)
        return NULL;
    return (FugaSymbol*)(self->tree[0]);
}

void FugaSymbols_set(
    FugaSymbols* self,
    const char* name,
    FugaSymbol* value
) {
    ALWAYS(name);
    ALWAYS(value);
    ALWAYS(self);

    for (; *name; name++) {
        if (!self->tree[(size_t)*name])
            self = self->tree[(size_t)*name] = FugaSymbols_new(self);
        else
            self = self->tree[(size_t)*name];
    }
    self->tree[0] = (FugaSymbols*)value;
}

#ifdef TESTING
TESTS(FugaSymbols) {
    void *gc = Fuga_init();
    FugaSymbols *self = FugaSymbols_new(gc);

    const char* str1 = "abc";
    const char* str2 = "def";
    const char* str3 = "";
    const char* str4 = "definitely";
    
    FugaSymbol* sym1 = (FugaSymbol*)FUGA_STRING("do");
    FugaSymbol* sym2 = (FugaSymbol*)FUGA_STRING("re");
    FugaSymbol* sym3 = (FugaSymbol*)FUGA_STRING("mi");
    FugaSymbol* sym4 = (FugaSymbol*)FUGA_STRING("fa");

    TEST(FugaSymbols_get(self, str1) == NULL);
    FugaSymbols_set(self, str1, sym1);
    TEST(FugaSymbols_get(self, str1) == sym1);

    TEST(FugaSymbols_get(self, str2) == NULL);
    FugaSymbols_set(self, str2, sym2);
    TEST(FugaSymbols_get(self, str2) == sym2);

    TEST(FugaSymbols_get(self, str3) == NULL);
    FugaSymbols_set(self, str3, sym3);
    TEST(FugaSymbols_get(self, str3) == sym3);

    TEST(FugaSymbols_get(self, str4) == NULL);
    FugaSymbols_set(self, str4, sym4);
    TEST(FugaSymbols_get(self, str4) == sym4);

    TEST(FugaSymbols_get(self, str1) == sym1);
    TEST(FugaSymbols_get(self, str2) == sym2);
    TEST(FugaSymbols_get(self, str3) == sym3);
    TEST(FugaSymbols_get(self, str4) == sym4);

    Fuga_quit(gc);
}
#endif

