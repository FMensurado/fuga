#ifndef FUGA_PRELUDE_H
#define FUGA_PRELUDE_H

#include "fuga.h"

void  FugaPrelude_init    (void*);
void* FugaPrelude_set  (void* self, void* args);
void* FugaPrelude_modify  (void* self, void* args);
void* FugaPrelude_if      (void* self, void* args);
void* FugaPrelude_method  (void* self, void* args);
void* FugaPrelude_print   (void* self, void* args);
void* FugaPrelude_import  (void* self, void* args);
void* FugaPrelude_match   (void* self, void* args);
void* FugaPrelude_do      (void* self, void* args);
void* FugaPrelude_def     (void* self, void* args);
void* FugaPrelude_help    (void* self, void* args);
void* FugaPrelude_try     (void* self, void* args);

void* FugaPrelude_is  (void* self, void* a, void* b);
void* FugaPrelude_isa (void* self, void* a, void* b);

void* FugaPrelude_orM     (void* self, void* args);
void* FugaPrelude_andM    (void* self, void* args);
void* FugaPrelude_notM    (void* self, void* arg);

#endif

