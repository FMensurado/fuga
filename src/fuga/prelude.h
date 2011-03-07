#ifndef FUGA_PRELUDE_H
#define FUGA_PRELUDE_H

#include "fuga.h"

void  FugaPrelude_init    (Fuga*);
Fuga* FugaPrelude_equals  (Fuga* self, Fuga* args);
Fuga* FugaPrelude_if      (Fuga* self, Fuga* args);
Fuga* FugaPrelude_method  (Fuga* self, Fuga* args);

#endif

