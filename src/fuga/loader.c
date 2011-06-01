#include "loader.h"
#include "test.h"
#include "parser.h"
#include "path.h"

#include <stdlib.h>

struct FugaLoader { int _unused; };

FugaLoader* FugaLoader_new(
    void* self
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    FugaLoader* loader = Fuga_clone(FUGA->Object);

    Fuga_setS(loader, "_name", FUGA_STRING("Loader"));

    Fuga_setS(loader, "setPaths", FUGA_METHOD_1(FugaLoader_setPaths_));
    Fuga_setS(loader, "setLocal", FUGA_METHOD_1(FugaLoader_setLocal_));
    Fuga_setS(loader, "load",     FUGA_METHOD_1(FugaLoader_load_));
    Fuga_setS(loader, "import",   FUGA_METHOD_1(FugaLoader_import_));

    FugaLoader_setPaths_(loader, FugaPath_FUGAPATH(loader));
    FugaLoader_setLocal_(loader, FUGA_STRING("."));

    return loader;
}


void* FugaLoader_setPaths_(
    FugaLoader* self,
    void* paths
) {
    ALWAYS(self);     ALWAYS(paths);
    FUGA_CHECK(self); FUGA_NEED(paths);
    return Fuga_setS(self, "paths", paths);
}

void* FugaLoader_setLocal_(
    FugaLoader* self,
    void*       local
) {
    ALWAYS(self);     ALWAYS(local);
    FUGA_CHECK(self); FUGA_NEED(local);
    if (Fuga_isString(local))
        local = FugaPath_new(local);
    return Fuga_setS(self, "local", local);
}


void* FugaLoader_load_(
    FugaLoader* self,
    void* filename
) {
    ALWAYS(self);     ALWAYS(filename);
    FUGA_CHECK(self); FUGA_NEED(filename);
    if (Fuga_isString(filename))
        filename = FugaPath_new(filename);

    FugaPath* local = FugaPath_join_(Fuga_getS(self, "local"), filename);
    FUGA_CHECK(local);
    bool isLocal = Fuga_isTrue(FugaPath_exists(local));

    void* paths = Fuga_getS(self, "paths");
    FUGA_NEED(paths);
    FUGA_FOR(i, path, paths) {
        FugaPath* filepath = FugaPath_join_(path, filename);
        FUGA_CHECK(filepath);
        FUGA_IF(FugaPath_exists(filepath)) { 
            if (isLocal)
                FUGA_RAISE(FUGA->IOError,
                    "local/global import conflict"
                );
            return FugaPath_load(filepath);
        }
    }

    if (isLocal)
        return FugaPath_load(local);

    FUGA_RAISE(FUGA->IOError, "no such module");
}

FugaPath* _FugaLoader_importName(
    void* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (Fuga_isMsg(self))
        self = FugaMsg_toSymbol(self);
    if (Fuga_isSymbol(self))
        self = FugaSymbol_toString(self);
    if (Fuga_isString(self))
        return FugaPath_new(FugaString_lower(self));
    if (Fuga_isExpr(self)) {
        FugaPath* result = NULL;
        FUGA_FOR(i, slot, self) {
            FugaPath* name = _FugaLoader_importName(slot);
            if (i == 0)
                result = name;
            else
                result = FugaPath_join_(result, name);
            FUGA_CHECK(result);
        }
        return result;
    }
    FUGA_CHECK(self);
    FUGA_RAISE(FUGA->TypeError, "Loader import: unexpected type");
}


void* FugaLoader_import_(
    FugaLoader* self,
    void*       arg
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    FugaPath* fname = _FugaLoader_importName(arg);
    fname = FugaPath_cat_(fname, FUGA_STRING(".fg"));
    FUGA_CHECK(fname);
    return FugaLoader_load_(self, fname);
}

