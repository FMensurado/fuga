#ifndef FUGA_MSG_H
#define FUGA_MSG_H

#include "fuga.h"

struct FugaMsg {
    FugaSymbol* name;
};

void FugaMsg_init(void*);

#define FUGA_MSG(x) (FugaMsg_new_(self, (x)))
FugaMsg* FugaMsg_new_(void*, const char*);
FugaMsg* FugaMsg_fromSymbol(FugaSymbol*);
FugaSymbol* FugaMsg_toSymbol(FugaMsg*);
void* FugaMsg_eval_in_(FugaMsg* self, void* recv, void* scope);
void* FugaMsg_name(FugaMsg*);
void* FugaMsg_args(FugaMsg*);
void* FugaMsg_str(void*);
void* FugaMsg_match_(FugaMsg*, void*);

#endif

