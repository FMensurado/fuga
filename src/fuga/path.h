#ifndef FUGA_PATH_H
#define FUGA_PATH_H

#include "fuga.h"
#include <stdio.h>

typedef struct FugaPath FugaPath;

void FugaPath_init (void* self);

FugaPath* FugaPath_new        (FugaString* self);
void*     FugaPath_exists     (FugaPath* path);
void*     FugaPath_isRelative (FugaPath* path);
void*     FugaPath_isAbsolute (FugaPath* path);
FugaPath* FugaPath_join_      (FugaPath* self, FugaPath* path);

FugaString* FugaPath_toString (FugaPath* path);
FugaString* FugaPath_str      (FugaPath* path);

/**
 * Open the file associated with a path, returning a normal
 * C FILE on success. On failure, return a normal Fuga exception.
 * Make sure to check before calling other file functions!
 *
 * For a Fuga-centric version, implement Fuga_open_.
 */
FILE* FugaPath_fopen_(FugaPath*, const char* mode);

#endif

