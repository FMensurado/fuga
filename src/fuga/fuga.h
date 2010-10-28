#ifndef FUGA_FUGA_H
#define FUGA_FUGA_H

typedef struct _fuga_t* fuga_t;

#include "env.h"
#include "slots.h"

/*
** The `value` is a context-specific value, and it depends itself.
** 
*/
struct _fuga_t {
    fuga_env_t   env;
    fuga_t       proto;
    fuga_slots_t slots;
    void*        value;
};

#endif

