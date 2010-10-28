#ifndef FUGA_SLOTS_H
#define FUGA_SLOTS_H

#include "../gc/gc.h"
#include "fuga.h"

/*
** 
*/
typedef struct _fuga_slots_t *fuga_slots_t;

/*
** ## Constructors
**
** Use `fuga_slots_new` to create a new table.
*/
fuga_slots_t fuga_slots_new (fuga_env_t env);

/*
** Fuga tables are intended to be 100% garbage collected (and this module
** depends on the gc package), so there aren't any public destructors.
** Sorry. (Not really.)
*/

/*
** ## Existence
**
** The `has` family of functions returns whether a given slot can be
** found in the table. They return TRUE (non-0) if so, and FALSE (0)
** otherwise.
*/
int fuga_slots_has  (fuga_slots_t, fuga_t);
int fuga_slots_hass (fuga_slots_t, fuga_str_t);
int fuga_slots_hassc(fuga_slots_t, const char*);
int fuga_slots_hasi (fuga_slots_t, fuga_int_t);
int fuga_slots_hasic(fuga_slots_t, size_t);

/*
** ## Getting
*/
fuga_t fuga_slots_get  (fuga_slots_t, fuga_t);
fuga_t fuga_slots_gets (fuga_slots_t, fuga_str_t);
fuga_t fuga_slots_getsc(fuga_slots_t, const char*);
fuga_t fuga_slots_geti (fuga_slots_t, fuga_int_t);
fuga_t fuga_slots_getic(fuga_slots_t, size_t);

/*
** ## Setting
*/
void fuga_slots_set  (fuga_slots_t, fuga_t,      fuga_t);
void fuga_slots_sets (fuga_slots_t, fuga_str_t,  fuga_t);
void fuga_slots_setsc(fuga_slots_t, const char*, fuga_t);
void fuga_slots_add  (fuga_slots_t,              fuga_t);

#endif

