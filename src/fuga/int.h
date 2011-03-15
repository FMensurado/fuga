#ifndef FUGA_INT_H
#define FUGA_INT_H

#include "fuga.h"

struct FugaInt {
    long value;
};

void FugaInt_init(void*);

#define FUGA_INT(x) FugaInt_new_(self, (x))
FugaInt* FugaInt_new_(void*, long);
long FugaInt_value(FugaInt*);
bool FugaInt_is_(FugaInt*, long);

void* FugaInt_str(void*);

// arithmetic
void* FugaInt_add(void*, void*);
void* FugaInt_sub(void*, void*);
void* FugaInt_mul(void*, void*);
void* FugaInt_fdiv(void*, void*);
void* FugaInt_mod(void*, void*);

// comparison
void* FugaInt_eq    (void*, void*);
void* FugaInt_neq   (void*, void*);
void* FugaInt_gt    (void*, void*);
void* FugaInt_lt    (void*, void*);
void* FugaInt_ge    (void*, void*);
void* FugaInt_le    (void*, void*);

// operators that are binary and unary
void* FugaInt_addMethod(void* self, void* args);
void* FugaInt_subMethod(void* self, void* args);

#endif

