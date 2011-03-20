#include "fuga.h"
#include "test.h"
#include <stdio.h>

struct FugaFile {
    FILE* fp;
};

void* FugaFile_exists_(
    void* self,
    FugaString* filename
) {
    ALWAYS(filename);
    FUGA_NEED(filename);
    if (!Fuga_isString(filename))
        FUGA_RAISE(FUGA->TypeError, "File exists: expected a string.");
    FILE* fp = fopen(filename->data, "r");
    if (fp) {
        fclose(fp);
        return FUGA->True;
    }
    return FUGA->False;
}

