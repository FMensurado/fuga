#ifndef FUGA_FILE_H
#define FUGA_FILE_H

#include "fuga.h"

typedef struct FugaFile FugaFile;

void* FugaFile_exists_(
    void* self,
    FugaString* filename
);

#endif

