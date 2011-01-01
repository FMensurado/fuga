#include "symbols.h"
#include "test.h"
#include <string.h>

struct FugaSymbols {
    FugaSymbols* tree[256];
};

void _FugaSymbols_mark(void* _self, FugaGC *gc) 
{
    FugaSymbols* self = _self;
    for (size_t i = 0; i < 256; i++)
        FugaGC_mark(gc, self, self->tree[i]);
}

FugaSymbols* FugaSymbols_new(FugaGC* gc)
{
    FugaSymbols* self = FugaGC_alloc(gc, sizeof(FugaSymbols),
                                     NULL, _FugaSymbols_mark);
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
    FugaGC *gc,
    const char* name,
    Fuga* value
) {
    ALWAYS(name);
    ALWAYS(value);
    ALWAYS(self);

    for (; *name; name++) {
        if (!self->tree[(size_t)*name])
            self = self->tree[(size_t)*name] = FugaSymbols_new(gc);
        else
            self = self->tree[(size_t)*name];
    }
    self->tree[0] = (FugaSymbols*)value;
} TESTSUITE(FugaSymbols_set) {
    FugaGC *gc = FugaGC_start();
    FugaSymbols *self = FugaSymbols_new(gc);

    const char* str1 = "abc";
    const char* str2 = "def";
    const char* str3 = "";
    const char* str4 = "definitely";
    
    Fuga* sym1 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* sym2 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* sym3 = FugaGC_alloc(gc, 4, NULL, NULL);
    Fuga* sym4 = FugaGC_alloc(gc, 4, NULL, NULL);

    TEST(FugaSymbols_get(self, str1) == NULL,
         "shouldn't have symbol before it's set");
    FugaSymbols_set(self, gc, str1, sym1);
    TEST(FugaSymbols_get(self, str1) == sym1,
         "should have symbol after it's set");

    TEST(FugaSymbols_get(self, str2) == NULL,
         "shouldn't have symbol before it's set");
    FugaSymbols_set(self, gc, str2, sym2);
    TEST(FugaSymbols_get(self, str2) == sym2,
         "should have symbol after it's set");

    TEST(FugaSymbols_get(self, str3) == NULL,
         "shouldn't have symbol before it's set");
    FugaSymbols_set(self, gc, str3, sym3);
    TEST(FugaSymbols_get(self, str3) == sym3,
         "should have symbol after it's set");

    TEST(FugaSymbols_get(self, str4) == NULL,
         "shouldn't have symbol before it's set");
    FugaSymbols_set(self, gc, str4, sym4);
    TEST(FugaSymbols_get(self, str4) == sym4,
         "should have symbol after it's set");

    TEST(FugaSymbols_get(self, str1) == sym1,
         "should retain symbol after it's set");
    TEST(FugaSymbols_get(self, str2) == sym2,
         "should retain symbol after it's set");
    TEST(FugaSymbols_get(self, str3) == sym3,
         "should retain symbol after it's set");
    TEST(FugaSymbols_get(self, str4) == sym4,
         "should retain symbol after it's set");

    FugaGC_end(gc);
}

