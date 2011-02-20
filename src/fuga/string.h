#ifndef FUGA_STRING_H
#define FUGA_STRING_H

#include "fuga.h"

#define FUGA_STRING(x) FugaString_new(self, (x))
void  FugaString_init(Fuga*);
Fuga* FugaString_new(Fuga*, const char*);
Fuga* FugaString_toSymbol(Fuga*);
void  FugaString_print(Fuga*);
bool  FugaString_isEqualTo(Fuga* self, const char* str);
Fuga* FugaString_str(Fuga*);

Fuga* FugaString_sliceFrom(Fuga*, long start);
Fuga* FugaString_sliceFromTo(Fuga*, long start, long end);
Fuga* FugaString_sliceTo(Fuga*, long end);

Fuga* FugaString_cat(Fuga*, Fuga*);

#endif

