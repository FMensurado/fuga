#ifndef FUGA_SYMBOL_H
#define FUGA_SYMBOL_H

#include "fuga.h"

void FugaSymbol_init(Fuga*);

#define FUGA_SYMBOL(x) FugaSymbol_new(self, (x))
Fuga* FugaSymbol_new(Fuga*, const char*);

bool  FugaSymbol_isValid(const char*);

Fuga* FugaSymbol_str(Fuga*);

#endif

