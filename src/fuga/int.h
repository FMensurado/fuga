#ifndef FUGA_INT_H
#define FUGA_INT_H

#include "fuga.h"

void FugaInt_init(Fuga*);

#define FUGA_INT(x) FugaInt_new(self, (x))
Fuga* FugaInt_new(Fuga*, long);
long FugaInt_value(Fuga*);
bool FugaInt_isEqualTo(Fuga*, long);

Fuga* FugaInt_str(Fuga*);

// arithmetic
Fuga* FugaInt_add(Fuga*, Fuga*);
Fuga* FugaInt_sub(Fuga*, Fuga*);
Fuga* FugaInt_mul(Fuga*, Fuga*);
Fuga* FugaInt_fdiv(Fuga*, Fuga*);
Fuga* FugaInt_mod(Fuga*, Fuga*);

// comparison
Fuga* FugaInt_eq(Fuga*, Fuga*);
Fuga* FugaInt_neq(Fuga*, Fuga*);
Fuga* FugaInt_gt(Fuga*, Fuga*);
Fuga* FugaInt_lt(Fuga*, Fuga*);
Fuga* FugaInt_ge(Fuga*, Fuga*);
Fuga* FugaInt_le(Fuga*, Fuga*);

// operators that are binary and unary
Fuga* FugaInt_addMethod(Fuga* self, Fuga* args);
Fuga* FugaInt_subMethod(Fuga* self, Fuga* args);

#endif

