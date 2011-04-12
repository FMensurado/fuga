#ifndef FUGA_PATH_H
#define FUGA_PATH_H

#include "fuga.h"
#include <stdio.h>

typedef struct FugaPath FugaPath;

void FugaPath_init (void* self);

FugaPath*   FugaPath_new         (FugaString* self);
void*       FugaPath_exists      (FugaPath* path);
void*       FugaPath_isRelative  (FugaPath* path);
void*       FugaPath_isAbsolute  (FugaPath* path);
FugaPath*   FugaPath_join_       (FugaPath* self, FugaPath* path);
FugaPath*   FugaPath_cat_        (FugaPath* self, FugaString* name);
FugaString* FugaPath_str         (FugaPath* path);

FILE* FugaPath_fopen_(FugaPath*, const char* mode);
void* FugaPath_load(FugaPath*);

FugaPath*   FugaPath_parent      (FugaPath* self);

void*       FugaPath_FUGAPATH    (void* self);

#endif

