#pragma once
#ifndef FUGA_SYMBOLS_H
#define FUGA_SYMBOLS_H

typedef struct FugaSymbols FugaSymbols;

#include "fuga.h"

FugaSymbols* FugaSymbols_new(void* self);

FugaSymbol* FugaSymbols_get(
    FugaSymbols* self,
    const char* name
);

void FugaSymbols_set(
    FugaSymbols* self,
    const char* name,
    FugaSymbol* value
);

#endif

