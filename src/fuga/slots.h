#pragma once
#ifndef FUGA_SLOTS_H
#define FUGA_SLOTS_H

/**
*** # FugaSlots
**/

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "gc.h"

/**
*** ## Types
*** ### FugaSlots
***
*** This is an ADT that handles the getting and setting of slots, in a
*** more-or-less low-level way. For every day use, I recommend you use
*** `Fuga_has`, `Fuga_get`, `Fuga_set`. The `Fuga_...` functions call
*** the respective `FugaSlots_...` functions.
***
*** Nothing is revealed about the implementation, because this is an
*** Abstract Data Type, meant to be use merely with its constructors.
**/
typedef struct FugaSlots FugaSlots;

/**
*** ### FugaSlot
***
*** Represents an individual slot. That is, a (name, value) pair.
**/
typedef struct FugaSlot FugaSlot;
#ifndef FUGA_FUGA_TYPEDEF
#define FUGA_FUGA_TYPEDEF
typedef struct Fuga Fuga;
#endif
struct FugaSlot {
    Fuga* name;
    Fuga* value;
};

/**
*** ### FugaSlotsIndex
***
*** Represents an index of a FugaSlots object. A FugaSlotsIndex can be
*** positive OR negative. For the sake of practicality, we restrict 
*** FugaSlotsIndex's to 64bit integers. We might choose to change this
*** later, so it's important to abstract out the functionality of
*** FugaSlotsIndex that we're going to need.
**/

typedef int64_t FugaSlotsIndex;

#define FugaSlotsIndex_fromInt(n) ((FugaSlotsIndex)(n))
#define FugaSlotsIndex_fromPtr(p) ((FugaSlotsIndex)(p))
#define FugaSlotsIndex_eq(a,b) ((a) == (b))
#define FugaSlotsIndex_lt(a,b) ((a) <  (b))

/**
*** ## Constructors
*** ### FugaSlots_new
***
*** Create an empty FugaSlots.
**/
FugaSlots* FugaSlots_new(void* gc);

/**
*** ## Properties
*** ### FugaSlots_length
***
*** Return the number of canonical slots. In other words, return the number
*** of contiguous indexed slots starting from 0.
**/
size_t FugaSlots_length(FugaSlots* slots);

/**
*** ## Has
*** ### FugaSlots_hasByIndex
***
*** Determine whether there is a slot with the given index.
**/
bool FugaSlots_hasByIndex(FugaSlots* slots, FugaSlotsIndex index);

/**
*** ### FugaSlots_hasBySymbol
***
*** Determine whether there is a slot with the give symbol.
**/
bool FugaSlots_hasBySymbol(FugaSlots* slots, Fuga* symbol);

/**
*** ## Get
*** ### FugaSlots_getByIndex
***
*** Get the slot associated with a given index.
**/
FugaSlot* FugaSlots_getByIndex  (FugaSlots* slots, FugaSlotsIndex index);

/**
*** ### FugaSlots_getBySymbol
***
*** Get the slot associated with a given symbol.
**/
FugaSlot* FugaSlots_getBySymbol (FugaSlots* slots, Fuga* name);

/**
*** ## Set
*** ### FugaSlots_setByIndex
***
*** Set or update the slot associated with a given index. You still
*** need to pass in a Fuga* object for the name because FugaSlots is
*** completely Fuga-agnostic. It doesn't know anything about Fuga
*** objects other than their addresses, and the fact that they need to
*** be garbage-collected, so FugaSlots can't fabricate its own Fuga
*** object for the name.
**/
void FugaSlots_setByIndex(
    FugaSlots* slots,
    Fuga* name,
    Fuga* value,
    FugaSlotsIndex index
);

/**
*** ### FugaSlots_setBySymbol
***
*** Set or update the slot associated with a given symbol.
**/
void FugaSlots_setBySymbol(
    FugaSlots* slots,
    Fuga* name,
    Fuga* value
);

#endif

