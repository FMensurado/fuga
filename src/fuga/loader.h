#pragma once
#ifndef FUGA_LOADER_H
#define FUGA_LOADER_H

#include "fuga.h"

typedef struct FugaLoader FugaLoader;

FugaLoader* FugaLoader_new(void*);
void*       FugaLoader_setPaths_(FugaLoader*, void*);
void*       FugaLoader_setLocal_(FugaLoader*, void*);
void*       FugaLoader_load_(FugaLoader*,   void*);
void*       FugaLoader_import_(FugaLoader*, void*);

#endif

