#ifndef FUGA_PRELUDE_H
#define FUGA_PRELUDE_H

#include "fuga.h"

void  FugaPrelude_init    (void*);
void* FugaPrelude_equals  (void* self, void* args);
void* FugaPrelude_if      (void* self, void* args);
void* FugaPrelude_method  (void* self, void* args);
void* FugaPrelude_print   (void* self, void* args);
void* FugaPrelude_import  (void* self, void* args);
void* FugaPrelude_match   (void* self, void* args);

#endif

