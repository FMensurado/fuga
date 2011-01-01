#pragma once
#ifndef FUGA_SYMBOLS_H
#define FUGA_SYMBOLS_H

#include "gc.h"

#ifndef FUGA_FUGA_TYPEDEF
typedef struct Fuga Fuga;
#endif

typedef struct FugaSymbols FugaSymbols;
Fuga* FugaSymbols_get(FugaSymbols* self, const char* name);
void FugaSymbols_set(FugaSymbols* self, FugaGC* gc,
    const char* name,
    Fuga* value
);

#endif

