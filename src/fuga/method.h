#ifndef METHOD_H
#define METHOD_H

#include "fuga.h"

void FugaMethod_init(void* self);

typedef void* (*FugaMethodFn)    (void*, void*, void*);
typedef void* (*FugaMethodFnN)   (void*, void*);
typedef void* (*FugaMethodFn0)   (void*);
typedef void* (*FugaMethodFn1)   (void*, void*);
typedef void* (*FugaMethodFn2)   (void*, void*, void*);

void* FugaMethod_new_(void* self, FugaMethodFn);
void* FugaMethodN_new_(void* self, FugaMethodFnN);
void* FugaMethod0_new_(void* self, FugaMethodFn0);
void* FugaMethod1_new_(void* self, FugaMethodFn1);
void* FugaMethod2_new_(void* self, FugaMethodFn2);
void* FugaMethodStr_new_(void* self, FugaMethodFn0);
void* FugaMethodOp_new_(void* self, const char* name);
void* FugaMethod_method(void* scope, void* args, void* body);
void* FugaMethod_addPattern(void* scope, void* args, void* body);
void* FugaMethod_call(void* self, void* recv, void* args);

#define FUGA_METHOD(fn) (FugaMethodN_new_(self, (FugaMethodFnN)(fn)))
#define FUGA_METHOD_0(fn) (FugaMethod0_new_(self,(FugaMethodFn0)(fn)))
#define FUGA_METHOD_1(fn) (FugaMethod1_new_(self,(FugaMethodFn1)(fn)))
#define FUGA_METHOD_2(fn) (FugaMethod2_new_(self,(FugaMethodFn2)(fn)))
#define FUGA_METHOD_STR(fn) (FugaMethodStr_new_(self,(FugaMethodFn0)fn))
#define FUGA_METHOD_OP(name) (FugaMethodOp_new_(self, name))


#endif

