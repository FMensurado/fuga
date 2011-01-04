#include "symbols.h"
#include "test.h"
#include "gc.h"
#include <string.h>

struct FugaSymbols {
    FugaSymbols* tree[256];
};

void _FugaSymbols_mark(void* _self) 
{
    FugaSymbols* self = _self;
    for (size_t i = 0; i < 256; i++)
        FugaGC_mark(self, self->tree[i]);
}

FugaSymbols* FugaSymbols_new(void* gc)
{
    FugaSymbols* self = FugaGC_alloc(gc, sizeof(FugaSymbols));
    FugaGC_onMark(self, _FugaSymbols_mark);
    memset(self, sizeof(FugaSymbols), 0);
    return self;
}

Fuga* FugaSymbols_get(FugaSymbols* self, const char* name)
{
    ALWAYS(name);
    for (; *name; name++) {
        if (!self)
            return NULL;
        self = self->tree[(size_t)*name];
    }
    if (!self)
        return NULL;
    return (Fuga*)(self->tree[0]);
}

void FugaSymbols_set(
    FugaSymbols* self,
    const char* name,
    Fuga* value
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
    FugaGC *gc = FugaGC_start();
    FugaSymbols *self = FugaSymbols_new(gc);

    const char* str1 = "abc";
    const char* str2 = "def";
    const char* str3 = "";
    const char* str4 = "definitely";
    
    Fuga* sym1 = FugaGC_alloc(gc, 4);
    Fuga* sym2 = FugaGC_alloc(gc, 4);
    Fuga* sym3 = FugaGC_alloc(gc, 4);
    Fuga* sym4 = FugaGC_alloc(gc, 4);

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

    FugaGC_end(gc);
}
#endif

