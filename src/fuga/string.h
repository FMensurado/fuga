#ifndef FUGA_STRING_H
#define FUGA_STRING_H

#include "fuga.h"

struct FugaString {
    size_t size;
    size_t length;
    char data[];
};

#define FUGA_STRING(x) FugaString_new(self, (x))
void        FugaString_init     (void*);
FugaString* FugaString_new      (void*, const char*);
FugaSymbol* FugaString_toSymbol (FugaString*);
void        FugaString_print    (FugaString*);
bool        FugaString_is_      (FugaString* self, const char* str);
void*       FugaString_str      (void*);
void*       FugaString_match_   (FugaString* self, FugaString* other);
FugaString* FugaString_from_    (FugaString*, long start);
FugaString* FugaString_from_to_ (FugaString*, long start, long end);
FugaString* FugaString_to_      (FugaString*, long end);
void*       FugaString_cat_     (void*, void*);
void*       FugaString_split_   (FugaString*, FugaString*);

#endif

