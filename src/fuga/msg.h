#ifndef FUGA_MSG_H
#define FUGA_MSG_H

#include "fuga.h"

#define FUGA_MSG(x) (FugaMsg_new(self, (x)))
Fuga* FugaMsg_new(Fuga*, const char*);
Fuga* FugaMsg_fromSymbol(Fuga*);
Fuga* FugaMsg_toSymbol(Fuga*);
Fuga* FugaMsg_eval(Fuga* self, Fuga* recv, Fuga* scope);

#endif

