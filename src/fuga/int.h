#ifndef FUGA_INT_H
#define FUGA_INT_H

#include "fuga.h"

void FugaInt_init(Fuga*);

#define FUGA_INT(x) FugaInt_new(self, (x))
Fuga* FugaInt_new(Fuga*, long);
long FugaInt_value(Fuga*);
bool FugaInt_isEqualTo(Fuga*, long);

Fuga* FugaInt_str(Fuga*);

// binary operators
Fuga* FugaInt_add(Fuga*, Fuga*);
Fuga* FugaInt_sub(Fuga*, Fuga*);
Fuga* FugaInt_mul(Fuga*, Fuga*);
Fuga* FugaInt_fdiv(Fuga*, Fuga*);
Fuga* FugaInt_mod(Fuga*, Fuga*);

// support unary
Fuga* FugaInt_addMethod(Fuga* self, Fuga* args);
Fuga* FugaInt_subMethod(Fuga* self, Fuga* args);

#endif

