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

void* _FugaLoader_runBlock(
    void* self
) {
    FUGA_CHECK(self);
    void* block = self;
    self = Fuga_clone(FUGA->Prelude);
    void* scope = Fuga_clone(self);
    FUGA_CHECK(Fuga_setS(scope, "this", self));

    long numSlots = FugaInt_value(Fuga_length(block));
    for (long i = 0; i < numSlots; i++) {
        void* slot = Fuga_getI(block, i);
        FUGA_CHECK(slot);
        void* result = Fuga_eval(slot, scope, scope);
        FUGA_CHECK(result);
    }
    return self;
}

void* _FugaLoader_runFile_(
    void* self,
    const char* filename
) {
    FugaParser *parser = FugaParser_new(self);
    if (!FugaParser_readFile_(parser, filename)) 
        FUGA_RAISE(FUGA->IOError, "can't load module");
    void* block = FugaParser_block(parser);
    // FIXME: ensure EOF
    FUGA_CHECK(block = _FugaLoader_runBlock(block));
    return block;
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

    // FIXME: check that there isn't local / global conflict

    void* paths = FugaString_split_(
        Fuga_getS(self, "path"),
        FUGA_STRING(":")
    );
    FUGA_NEED(paths);
    long length = FugaInt_value(Fuga_length(paths));
    for (int i = 0; i < length; i++) {
        FugaString* path = Fuga_getI(paths, i); 
        FugaString* filepath = _FugaLoader_joinPath(path, filename);
        FUGA_CHECK(filepath);
        if (Fuga_isTrue(FugaFile_exists_(self, filepath)))
            return _FugaLoader_runFile_(self, filepath->data);
    }

    FUGA_RAISE(FUGA->IOError, "no such module");
}

void* FugaLoader_import_(
    FugaLoader* self,
    void*       arg
) {
    ALWAYS(self);       ALWAYS(arg);
    FUGA_CHECK(self);   FUGA_CHECK(arg);
    if (Fuga_isMsg(arg)) {
        arg = FugaMsg_toSymbol(arg);
        FUGA_CHECK(arg);
    }
    if (Fuga_isSymbol(arg)) {
        FugaString* str   = FugaSymbol_toString(arg);
        FugaString* fname = FugaString_cat_(str, FUGA_STRING(".fg"));
        FUGA_CHECK(fname);
        return FugaLoader_load_(self, fname);
    }
    FUGA_RAISE(FUGA->TypeError, "Loader import: unexpected type");
}

