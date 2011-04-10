#pragma once
#ifndef FUGA_SLOTS_H
#define FUGA_SLOTS_H

/**
*** # FugaSlots
**/


/**
*** ## Types
*** ### FugaSlots
***
*** This is an ADT that handles the getting and setting of slots, in a
*** more-or-less low-level way. For every day use, I recommend you use
*** `Fuga_hasSlot`, `Fuga_getSlot`, `Fuga_setSlot`. The `Fuga_...` functions call
*** the respective `FugaSlots_...` functions.
***
*** Nothing is revealed about the implementation, because this is an
*** Abstract Data Type, meant to be use merely with its constructors.
**/
typedef struct FugaSlots FugaSlots;

#include "fuga.h"

/**
*** ### FugaSlot
***
*** Represents an individual slot. That is, a (name, value) pair.
**/
typedef struct FugaSlot FugaSlot;
struct FugaSlot {
    void* value;
    void* name;
    void* doc;
    FugaIndex index;
};

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
bool FugaSlots_hasByIndex(FugaSlots* slots, FugaIndex index);

/**
*** ### FugaSlots_hasBySymbol
***
*** Determine whether there is a slot with the give symbol.
**/
bool FugaSlots_hasBySymbol(FugaSlots* slots, void* symbol);

/**
*** ## Get
*** ### FugaSlots_getByIndex
***
*** Get the slot associated with a given index.
**/
FugaSlot* FugaSlots_getByIndex(FugaSlots* slots, FugaIndex index);

/**
*** ### FugaSlots_getBySymbol
***
*** Get the slot associated with a given symbol.
**/
FugaSlot* FugaSlots_getBySymbol(FugaSlots* slots, void* name);

void FugaSlots_append_(FugaSlots* slots, FugaSlot value);

/**
*** ## Set
*** ### FugaSlots_setByIndex
***
*** Set or update the slot associated with a given index.
**/
void FugaSlots_setByIndex(FugaSlots* slots, FugaIndex index, FugaSlot slot);

/**
*** ### FugaSlots_setBySymbol
***
*** Set or update the slot associated with a given symbol.
**/
void FugaSlots_setBySymbol(FugaSlots* slots, void* name, FugaSlot slot);

void FugaSlots_delByIndex  (FugaSlots* slots, FugaIndex index);
void FugaSlots_delBySymbol (FugaSlots* slots, void* name);

#endif

