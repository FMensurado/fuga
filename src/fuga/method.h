#ifndef METHOD_H
#define METHOD_H

#include "fuga.h"

void FugaMethod_init(Fuga* self);

#define FUGA_METHOD(fn) (FugaMethod_new(self, (fn)))
Fuga* FugaMethod_new(Fuga* self, FugaMethod method);
Fuga* FugaMethod_call(Fuga* self, Fuga* recv, Fuga* args);

Fuga* FugaMethod_arg(Fuga* self, Fuga* (*)(Fuga*, Fuga*));
#define FUGA_METHOD_ARG(fn) (FugaMethod_arg(self, (fn)))

Fuga* FugaMethod_0arg(Fuga* self, Fuga* (*)(Fuga*));
#define FUGA_METHOD_0ARG(fn) (FugaMethod_0arg(self, (fn)))

Fuga* FugaMethod_1arg(Fuga* self, Fuga* (*)(Fuga*, Fuga*));
#define FUGA_METHOD_1ARG(fn) (FugaMethod_1arg(self, (fn)))

Fuga* FugaMethod_2arg(Fuga* self, Fuga* (*)(Fuga*, Fuga*, Fuga*));
#define FUGA_METHOD_2ARG(fn) (FugaMethod_2arg(self, (fn)))

Fuga* FugaMethod_strMethod(Fuga* self, Fuga* (*)(Fuga*));
#define FUGA_METHOD_STR(fn) (FugaMethod_strMethod(self, (fn)))

#endif

