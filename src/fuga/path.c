#include "test.h"
#include "path.h"
#include "platform.h"

struct FugaPath {
    unsigned unused;
};

void FugaPath_init(
    void* self
) {
    ALWAYS(self);

    Fuga_setS(FUGA->Path, "str",  FUGA_METHOD_STR(FugaPath_str));
    Fuga_setS(FUGA->Path, "join", FUGA_METHOD_1  (FugaPath_join_));
    Fuga_setS(FUGA->Path, "load", FUGA_METHOD_0  (FugaPath_load));
    Fuga_setS(FUGA->Path, "exists", FUGA_METHOD_0 (FugaPath_exists));
    Fuga_setS(FUGA->Path, "parent", FUGA_METHOD_0 (FugaPath_parent));
    Fuga_setS(FUGA->Path, "FUGAPATH", FUGA_METHOD_0 (FugaPath_FUGAPATH));
    Fuga_setS(FUGA->Path, "isAbsolute?",
         FUGA_METHOD_0(FugaPath_isAbsolute));
    Fuga_setS(FUGA->Path, "isRelative?",
         FUGA_METHOD_0(FugaPath_isRelative));
    Fuga_setS(FUGA->Path, "++", FUGA_METHOD_1  (FugaPath_cat_));
}

FugaPath* FugaPath_new(
    FugaString* self
) {
    FUGA_CHECK(self);
    FugaPath* path  = Fuga_clone(FUGA->Path);
    void*     parts = FugaString_split_(self,
        FUGA_STRING(FUGA_PLATFORM_PATHSEP)
    );
    FUGA_CHECK(parts);
    if (FugaString_is_(Fuga_getI(parts, -1), ""))
        FUGA_CHECK(Fuga_delI(parts, -1));
    FUGA_CHECK(Fuga_setS(path, "parts", parts));
    return path;
}

void* FugaPath_parts(
    FugaPath* self
) {
    void* args = Fuga_clone(FUGA->Object);
    return Fuga_send(self, FUGA_SYMBOL("parts"), args);
}

FugaString* FugaPath_str(FugaPath* self) {
    FUGA_NEED(self);
    void* parts = FugaPath_parts(self);
    return FugaString_join_(FUGA_STRING(FUGA_PLATFORM_PATHSEP), parts);
}

FILE* FugaPath_fopen_(FugaPath* self, const char* mode) {
    FugaString *path = FugaPath_str(self);
    if (!Fuga_isString(path)) return NULL;
    return fopen(path->data, mode);
}

void* FugaPath_load(FugaPath* self) {
    FUGA_NEED(self);
    FugaString* filename = FugaPath_str(self);
    FUGA_CHECK(filename);
    if (!Fuga_isString(filename))
        FUGA_RAISE(FUGA->TypeError,
            "Path load: expected Path str to return string"
        );
    return Fuga_load_(self, filename->data);
}

void* FugaPath_exists(FugaPath* self) {
    FILE* file = FugaPath_fopen_(self, "r");
    if (file) {
        fclose(file);
        return FUGA->True;
    }
    return FUGA->False;
}

void* FugaPath_isRelative(FugaPath* self) {
    void* parts = FugaPath_parts(self);
    FUGA_NEED(parts);
    return FUGA_BOOL(!FUGA_PLATFORM_ABSPATH(parts));
}
void* FugaPath_isAbsolute(FugaPath* self) {
    void* parts = FugaPath_parts(self);
    FUGA_NEED(parts);
    return FUGA_BOOL( FUGA_PLATFORM_ABSPATH(parts));
}

FugaPath* FugaPath_join_(FugaPath* self, FugaPath* path) {
    FUGA_NEED(self); FUGA_NEED(path);
    if (Fuga_isString(path))
        path = FugaPath_new((FugaString*)path);
    FUGA_IF(FugaPath_isAbsolute(path))
        FUGA_RAISE(FUGA->ValueError,
            "Path join: can't join an absolut path on the right"
        );
    void* result = Fuga_clone(FUGA->Path);
    void* parts  = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_extend_(parts, FugaPath_parts(self)));
    FUGA_CHECK(Fuga_extend_(parts, FugaPath_parts(path)));
    FUGA_CHECK(Fuga_setS(result, "parts", parts));
    return result;
}

FugaPath* FugaPath_cat_(
    FugaPath* self,
    FugaString* name
) {
    FUGA_NEED(self); FUGA_NEED(name);
    void* parts = FugaPath_parts(self);
    void* newparts = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_extend_(newparts, parts));

    if (Fuga_hasLength_(parts, 0)) {
        FUGA_CHECK(Fuga_append_(newparts, name));
    } else {
        void* laststr = Fuga_getI(parts, -1);
        laststr = FugaString_cat_(laststr, name);
        FUGA_CHECK(Fuga_setI(newparts, -1, laststr));
    }

    void* result = Fuga_clone(FUGA->Path);
    FUGA_CHECK(Fuga_setS(result, "parts", newparts));
    return result;
}

FugaPath* FugaPath_parent(
    FugaPath* self
) {
    FUGA_NEED(self);
    void* parts = FugaPath_parts(self);
    void* newparts = Fuga_clone(FUGA->Object);
    FUGA_CHECK(Fuga_extend_(newparts, parts));

    if (Fuga_hasLength_(parts, 0)
     || FugaString_is_(Fuga_getI(parts,-1), ".")
     || FugaString_is_(Fuga_getI(parts,-1), "..")) {
        FUGA_CHECK(Fuga_append_(newparts, FUGA_STRING("..")));
    } else {
        FUGA_CHECK(Fuga_delI(newparts, -1));
    }
    
    void* result = Fuga_clone(FUGA->Path);
    FUGA_CHECK(Fuga_setS(result, "parts", newparts));
    return result;
}

void* FugaPath_FUGAPATH(
    void* self
) {
    const char* FUGAPATH = FUGA_PLATFORM_FUGAPATH;
    void* result = Fuga_clone(FUGA->Object);
    if (!FUGAPATH) {
        void* path = FugaPath_new(FUGA_STRING("lib"));
        FUGA_CHECK(Fuga_append_(result, path));
    } else {
        void* path   = FUGA_STRING(FUGAPATH);
        void* paths  = FugaString_split_(path,
            FUGA_STRING(FUGA_PLATFORM_FUGAPATH_SEP)
        );

        FUGA_FOR(i, slot, paths) {
            if (!FugaString_is_(slot, ""))
                FUGA_CHECK(Fuga_append_(result, FugaPath_new(slot)));
        }
    }
    return result;
}

