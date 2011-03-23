#ifndef FUGA_SYMBOL_H
#define FUGA_SYMBOL_H

#include "fuga.h"

struct FugaSymbol {
    size_t size;
    size_t length;
    char data[];
};

void FugaSymbol_init(void*);

bool FugaSymbol_is_(FugaSymbol* self, const char* value);
bool FugaSymbol_isValid(const char*);

#define FUGA_SYMBOL(x) FugaSymbol_new_(self, (x))

FugaSymbol* FugaSymbol_new_     (void*, const char*);
void* FugaSymbol_str      (void*);
void* FugaSymbol_toString (void*);
void* FugaSymbol_match_   (FugaSymbol*, FugaSymbol*);

#endif

