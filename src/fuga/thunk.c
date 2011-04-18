
#include "thunk.h"

struct FugaThunk {
    int _unused;
};

void FugaThunk_init(void* self) {
    Fuga_setS(FUGA->Thunk, "str", FUGA_METHOD_STR(FugaThunk_str));
    Fuga_setS(FUGA->Thunk, "eval", FUGA_METHOD(FugaThunk_evalMethod));
    Fuga_setS(FUGA->Thunk, "lazy", FUGA_METHOD_0(FugaThunk_lazy));
}

FugaThunk* FugaThunk_new(void* self) {
    FUGA_CHECK(self);
    void* thunk = Fuga_clone(FUGA->Thunk);
    FUGA_CHECK(Fuga_setS(thunk, "code",  Fuga_lazyCode (self)));
    FUGA_CHECK(Fuga_setS(thunk, "scope", Fuga_lazyScope(self)));
    return thunk;
}

FugaThunk* FugaThunk_new_(void* self, void* scope) {
    FUGA_NEED(self); FUGA_NEED(scope);
    void* thunk = Fuga_clone(FUGA->Thunk);
    FUGA_CHECK(Fuga_setS(thunk, "code",  self));
    FUGA_CHECK(Fuga_setS(thunk, "scope", scope));
    return thunk;
}

void* FugaThunk_code(FugaThunk* self) {
    return Fuga_getS(self, "code");
}

void* FugaThunk_scope(FugaThunk* self) {
    return Fuga_getS(self, "scope");
}

void* FugaThunk_lazy(FugaThunk* self) {
    void* code  = FugaThunk_code(self);
    void* scope = FugaThunk_scope(self);
    return Fuga_lazy_(code, scope);
}

void* FugaThunk_eval(FugaThunk* self) {
    void* code  = FugaThunk_code(self);
    void* scope = FugaThunk_scope(self);
    return Fuga_eval(code, scope, scope);
}

void* FugaThunk_eval_(FugaThunk* self, void* scope) {
    void* code = FugaThunk_code(self);
    return Fuga_eval(code, scope, scope);
}

void* FugaThunk_evalMethod(FugaThunk* self, void* args) {
    FUGA_NEED(self);
    FUGA_NEED(args);
    if (Fuga_hasLength_(args, 0))
        return FugaThunk_eval(self);
    else if (Fuga_hasLength_(args, 1))
        return FugaThunk_eval_(self, Fuga_getI(args, 0));
    else
        FUGA_RAISE(FUGA->TypeError,
            "Thunk eval: expected 0 or 1 arguments"
        );
}

FugaString* FugaThunk_str(FugaThunk* self) {
    FUGA_NEED(self);
    FugaString* result;
    void* code = FugaThunk_code(self);
    result = FUGA_STRING("thunk(");
    result = FugaString_cat_(result, Fuga_str(code));
    result = FugaString_cat_(result, FUGA_STRING(", ...)"));
    return result;
}

