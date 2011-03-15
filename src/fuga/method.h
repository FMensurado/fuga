#ifndef METHOD_H
#define METHOD_H

#include "fuga.h"

void FugaMethod_init(void* self);

void* FugaMethod_new_(void* self, void* (*)(void*, void*, void*));
void* FugaMethod_call(void* self, void* recv, void* args);
void* FugaMethodN_new_(void* self, void* (*)(void*, void*));
void* FugaMethod0_new_(void* self, void* (*)(void*));
void* FugaMethod1_new_(void* self, void* (*)(void*, void*));
void* FugaMethod2_new_(void* self, void* (*)(void*, void*, void*));
void* FugaMethodStr_new_(void* self, void* (*)(void*));
void* FugaMethod_method(void* scope, void* args, void* body);

#define FUGA_METHOD(fn) (FugaMethodN_new_(self, (fn)))
#define FUGA_METHOD_0ARG(fn) (FugaMethod0_new_(self, (fn)))
#define FUGA_METHOD_1ARG(fn) (FugaMethod1_new_(self, (fn)))
#define FUGA_METHOD_2ARG(fn) (FugaMethod2_new_(self, (fn)))
#define FUGA_METHOD_STR(fn) (FugaMethodStr_new_(self, (fn)))


#endif

