#ifndef FUGA_STRING_H
#define FUGA_STRING_H

#include "fuga.h"

#define FUGA_STRING(x) FugaString_new(self, (x))
Fuga* FugaString_new(Fuga*, const char*);
Fuga* FugaString_toSymbol(Fuga*);
void  FugaString_print(Fuga*);

#endif

