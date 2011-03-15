#ifndef FUGA_LAZY_H
#define FUGA_LAZY_H

#include "fuga.h"

struct FugaLazy {
    void* code;
    void* scope;
};

// Lazy Evaluation
void* Fuga_need         (void* self);
void* Fuga_needOnce     (FugaLazy* self);
void* Fuga_lazy_        (void* self, void* scope);
void* Fuga_lazyCode     (FugaLazy* self);
void* Fuga_lazyScope    (FugaLazy* self);
void* Fuga_lazySlots    (void* self);
void* Fuga_needSlots    (FugaLazy* self);

#define FUGA_NEED(value)                                                \
    do {                                                                \
        value = Fuga_need(value);                                       \
        FUGA_CHECK(value);                                              \
    } while(0)

#endif

