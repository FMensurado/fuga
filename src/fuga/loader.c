#include "loader.h"
#include "test.h"
#include "parser.h"
#include "file.h"

#include <stdlib.h>

struct FugaLoader { int _unused; };

FugaString* _FugaLoader_getPath(
        void* self
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    char* path = getenv("FUGAPATH");
    if (!path)
        return FUGA_STRING("");
    else
        return FUGA_STRING(path);
}

FugaLoader* FugaLoader_new(
    void* self
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    FugaLoader* loader = Fuga_clone(FUGA->Object);

    Fuga_setS(loader, "setPath",  FUGA_METHOD_1(FugaLoader_setPath_));
    Fuga_setS(loader, "setLocal", FUGA_METHOD_1(FugaLoader_setLocal_));
    Fuga_setS(loader, "load",     FUGA_METHOD_1(FugaLoader_load_));
    Fuga_setS(loader, "import",   FUGA_METHOD_1(FugaLoader_import_));

    FugaLoader_setPath_(loader, _FugaLoader_getPath(loader));
    FugaLoader_setLocal_(loader, FUGA_STRING("."));

    return loader;
}


void* FugaLoader_setPath_(
    FugaLoader* self,
    FugaString* path
) {
    ALWAYS(self);     ALWAYS(path);
    FUGA_CHECK(self); FUGA_NEED(path);
    if (!Fuga_isString(path))
        FUGA_RAISE(FUGA->TypeError, "Loader setPath: expected a string");
    return Fuga_setS(self, "path", path);
}

void* FugaLoader_setLocal_(
    FugaLoader* self,
    FugaString* local
) {
    ALWAYS(self);     ALWAYS(local);
    FUGA_CHECK(self); FUGA_NEED(local);
    if (!Fuga_isString(local))
        FUGA_RAISE(FUGA->TypeError,
            "Loader setLocal: expected a string"
        );
    return Fuga_setS(self, "local", local);
}


FugaString* _FugaLoader_joinPath(
    FugaString* self,
    FugaString* filename
) {
    // FIXME: don't duplicate separator
    // FIXME: use portable separator (/ on unix, \ on windows)
    self = FugaString_cat_(self, FUGA_STRING("/"));
    return FugaString_cat_(self, filename);
}


void* FugaLoader_load_(
    FugaLoader* self,
    FugaString* filename
) {
    ALWAYS(self);     ALWAYS(filename);
    FUGA_CHECK(self); FUGA_NEED(filename);
    if (!Fuga_isString(filename))
        FUGA_RAISE(FUGA->TypeError, "Loader load: expected a string");

    FugaString* local = Fuga_getS(self, "local");
    local = _FugaLoader_joinPath(local, filename);
    FUGA_NEED(local);
    bool isLocal = Fuga_isTrue(FugaFile_exists_(self, local));

    void* paths = FugaString_split_(
        Fuga_getS(self, "path"),
        FUGA_STRING(":")
    );
    FUGA_NEED(paths);
    FUGA_FOR(i, path, paths) {
        FugaString* filepath = _FugaLoader_joinPath(path, filename);
        FUGA_CHECK(filepath);
        if (Fuga_isTrue(FugaFile_exists_(self, filepath))) {
            if (isLocal)
                FUGA_RAISE(FUGA->IOError,
                    "local/global import conflict"
                );
            return Fuga_load_(self, filepath->data);
        }
    }

    if (isLocal)
        return Fuga_load_(self, local->data);

    FUGA_RAISE(FUGA->IOError, "no such module");
}

FugaString* _FugaLoader_importName(
    void* self
) {
    ALWAYS(self);
    FUGA_NEED(self);
    if (Fuga_isMsg(self))
        self = FugaMsg_toSymbol(self);
    if (Fuga_isSymbol(self))
        self = FugaSymbol_toString(self);
    if (Fuga_isString(self))
        return self;
    if (Fuga_isExpr(self)) {
        FugaString* result;
        FUGA_FOR(i, slot, self) {
            FugaString* name = _FugaLoader_importName(slot);
            if (i == 0)
                result = name;
            else
                result = _FugaLoader_joinPath(result, name);
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
    FugaString* fname = _FugaLoader_importName(arg);
    fname = FugaString_cat_(fname, FUGA_STRING(".fg"));
    FUGA_CHECK(fname);
    return FugaLoader_load_(self, fname);
}

