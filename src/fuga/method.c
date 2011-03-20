#include "method.h"
#include "test.h"

const FugaType FugaMethod_type = {
    "Method"
};

void FugaMethod_init(void* self)
{
    Fuga_setS(FUGA->Method, "str", FUGA_STRING("method(...)"));
}

struct FugaMethod {
    void* (*method)(void*, void*, void*);
};

void* FugaMethod_new_(void* self, void* (*method)(void*, void*, void*))
{
    ALWAYS(self); ALWAYS(method);
    FugaMethod* result = Fuga_clone_(FUGA->Method, sizeof(FugaMethod));
    Fuga_type_(result, &FugaMethod_type);
    result->method = method;
    return result;
}

void* FugaMethod_call(void* _self, void* recv, void* args)
{
    FugaMethod* self = _self;
    ALWAYS(self); ALWAYS(recv); ALWAYS(args);
    ALWAYS(Fuga_isMethod(self));
    FUGA_CHECK(recv); FUGA_CHECK(args);
    return self->method(self, recv, args);
}

// arg methods

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* (*method) (void*, void*);
} FugaMethodN;


void* FugaMethodN_call(void* _self, void* recv, void* args)
{
    FugaMethodN* self = _self;
    return self->method(recv, args);
}

void* FugaMethodN_new_(void* self, void* (*method)(void*, void*))
{
    FugaMethodN* result = Fuga_clone_(FUGA->Method, sizeof(FugaMethodN));
    Fuga_type_(result, &FugaMethod_type);
    result->call   = FugaMethodN_call;
    result->method = method;
    return result;
}

// str method

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* (*method) (void*);
} FugaMethodStr;

void* FugaMethodStr_call(void* _self, void* recv, void* args)
{
    FugaMethodStr* self = _self;
    FUGA_NEED(args);
    if (!Fuga_hasLength_(args, 0))
        FUGA_RAISE(FUGA->TypeError, "str: expected no arguments");
    if (Fuga_isTrue(Fuga_hasRawS(recv, "name")))
        return Fuga_getRawS(recv, "name");
    return self->method(recv);
}

void* FugaMethodStr_new_(void* self, void* (*method)(void*))
{
    FugaMethodStr* result = Fuga_clone_(FUGA->Method, sizeof(*result));
    Fuga_type_(result, &FugaMethod_type);
    result->call   = FugaMethodStr_call;
    result->method = method;
    return result;
}

// 0 arg methods 

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* (*method) (void*);
} FugaMethod0;

void* FugaMethod0_call(void* _self, void* recv, void* args)
{
    FugaMethod0* self = _self;
    FUGA_NEED(args);
    if (!Fuga_hasLength_(args, 0))
        FUGA_RAISE(FUGA->TypeError, "expected no arguments");
    return self->method(recv);
}

void* FugaMethod0_new_(void* self, void* (*method)(void*))
{
    FugaMethod0* result = Fuga_clone_(FUGA->Method, sizeof(*result));
    Fuga_type_(result, &FugaMethod_type);
    result->call   = FugaMethod0_call;
    result->method = method;
    return result;
}

// 1 arg methods 

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* (*method) (void*, void*);
} FugaMethod1;

void* FugaMethod1_call(void* _self, void* recv, void* args)
{
    FugaMethod1* self = _self;
    FUGA_NEED(args);
    if (!Fuga_hasLength_(args, 1))
        FUGA_RAISE(FUGA->TypeError, "expected 1 argument");
    void* arg0 = Fuga_getI(args, 0);
    return self->method(recv, arg0);
}

void* FugaMethod1_new_(void* self, void* (*method)(void*, void*))
{
    FugaMethod1* result = Fuga_clone_(FUGA->Method, sizeof(*result));
    Fuga_type_(result, &FugaMethod_type);
    result->call   = FugaMethod1_call;
    result->method = method;
    return result;
}

// 2 arg methods 

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* (*method) (void*, void*, void*);
} FugaMethod2;

void* FugaMethod2_call(void* _self, void* recv, void* args)
{
    FugaMethod2* self = _self;
    FUGA_NEED(args);
    if (!Fuga_hasLength_(args, 2))
        FUGA_RAISE(FUGA->TypeError, "expected 2 arguments");
    void* arg0 = Fuga_getI(args, 0);
    void* arg1 = Fuga_getI(args, 1);
    return self->method(recv, arg0, arg1);
}

void* FugaMethod2_new_(void* self, void* (*method)(void*, void*, void*))
{
    FugaMethod2* result = Fuga_clone_(FUGA->Method, sizeof(*result));
    Fuga_type_(result, &FugaMethod_type);
    result->call   = FugaMethod2_call;
    result->method = method;
    return result;
}

// fuga method

typedef struct {
    void* (*call)   (void*, void*, void*);
    void* scope;
    void* args;
    void* body;
} FugaMethodFuga;

void FugaMethodFuga_mark(void* _self)
{
    FugaMethodFuga *self = _self;
    Fuga_mark_(self, self->scope);
    Fuga_mark_(self, self->args);
    Fuga_mark_(self, self->body);
}

void* FugaMethodFuga_scope(void* self, void* formals, void* actuals)
{
    // FIXME: check to see if 'thunkSlots' is necessary
    FUGA_NEED(self); FUGA_NEED(formals); FUGA_NEED(actuals);
    self = Fuga_clone(self);
    void* numFormalsF = Fuga_length(formals);
    FUGA_CHECK(numFormalsF);
    void* numActualsF = Fuga_length(actuals);
    FUGA_CHECK(numActualsF);

    long numFormals = FugaInt_value(numFormalsF);
    long numActuals = FugaInt_value(numActualsF);
    if (numFormals != numActuals)
        FUGA_RAISE(FUGA->TypeError, "expected different number of args");
    for (long i = 0; i < numFormals; i++) {
        void* formal = Fuga_getI(formals, i);
        void* actual = Fuga_getI(actuals, i);
        FUGA_CHECK(formal);
        FUGA_CHECK(actual);
        FUGA_CHECK(Fuga_set(self, formal, actual));
    }
    // FIXME: handle thunks
    return self;
}


void* FugaMethodFuga_call(void* _self, void* recv, void* args)
{
    FugaMethodFuga* self = _self;
    FUGA_CHECK(self); FUGA_CHECK(recv); FUGA_CHECK(args);
    ALWAYS(Fuga_isMethod(self));
    void* scope = FugaMethodFuga_scope(self->scope, self->args, args);
    FUGA_CHECK(scope);
    FUGA_CHECK(Fuga_setS(scope, "self", recv));
    return Fuga_eval(self->body, scope, scope);
}

void* FugaMethod_method(void* self, void* args, void* body)
{
    FUGA_NEED(self);
    FUGA_NEED(args);
    FUGA_NEED(body);
    FugaMethodFuga* result = Fuga_clone_(FUGA->Method, sizeof *result);
    Fuga_type_(result, &FugaMethod_type);
    Fuga_onMark_(result, FugaMethodFuga_mark);
    result->call  = FugaMethodFuga_call;
    result->scope = self;
    result->args  = args;
    result->body  = body;
    return result;
}

