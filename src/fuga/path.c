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
    FUGA->Path = Fuga_clone(FUGA->Object);
}

FugaPath* FugaPath_new(
    FugaString* self
) {
    FUGA_CHECK(self);
    FugaPath* path  = Fuga_clone(FUGA->Path);
    void*     parts = FugaString_split_(self, FUGA_STRING(FUGA_PATHSEP));
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

FugaString* FugaPath_toString(FugaPath* self) {
    FUGA_NEED(self);
    void* parts = FugaPath_parts(self);
    return FugaString_join_(FUGA_STRING(FUGA_PATHSEP), parts);
}

FILE* FugaPath_fopen_(FugaPath* self, const char* mode) {
    FugaString *path = FugaPath_toString(self);
    FUGA_NEED(path);
    return fopen(path->data, mode);
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
    return FUGA_BOOL(!FUGA_ISABS(parts));
}
void* FugaPath_isAbsolute(FugaPath* self) {
    void* parts = FugaPath_parts(self);
    FUGA_NEED(parts);
    return FUGA_BOOL(FUGA_ISABS(parts));
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

FugaString* FugaPath_str(FugaPath* path);
