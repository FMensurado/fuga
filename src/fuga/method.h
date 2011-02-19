#ifndef METHOD_H
#define METHOD_H

#include "fuga.h"


#define FUGA_METHOD(fn) (FugaMethod_new(self, (fn)))
Fuga* FugaMethod_new(Fuga* self, FugaMethod method);
Fuga* FugaMethod_call(Fuga* self, Fuga* recv, Fuga* args);

Fuga* FugaMethod_noArgs(Fuga* self, Fuga* (*)(Fuga*));
Fuga* FugaMethod_strMethod(Fuga* self, Fuga* (*)(Fuga*));

#endif

