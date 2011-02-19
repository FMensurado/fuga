#ifndef METHOD_H
#define METHOD_H

#include "fuga.h"


#define FUGA_METHOD(fn) (FugaMethod_new(self, (fn)))
Fuga* FugaMethod_new(Fuga* self, FugaMethod method);
Fuga* FugaMethod_call(Fuga* self, Fuga* recv, Fuga* args);

Fuga* FugaMethod_0arg(Fuga* self, Fuga* (*)(Fuga*));

Fuga* FugaMethod_strMethod(Fuga* self, Fuga* (*)(Fuga*));
#define FUGA_METHOD_STR(fn) (FugaMethod_strMethod(self, (fn)))

#endif

