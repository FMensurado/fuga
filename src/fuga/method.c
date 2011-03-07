#include "method.h"
#include "test.h"

void FugaMethod_init(Fuga* self)
{
    Fuga_setSlot(FUGA->Method, FUGA_SYMBOL("str"),
        FUGA_STRING("method(...)"));
}

Fuga* FugaMethod_new(Fuga* self, FugaMethod method)
{
    ALWAYS(self); ALWAYS(method);
    self = Fuga_clone(FUGA->Method);
    self->type = FUGA_TYPE_METHOD;
    self->method = method;
    return self;
}

Fuga* FugaMethod_call(Fuga* self, Fuga* recv, Fuga* args)
{
    ALWAYS(self); ALWAYS(recv); ALWAYS(args);
    ALWAYS(Fuga_isMethod(self));
    FUGA_CHECK(recv); FUGA_CHECK(args);
    return self->method(self, recv, args);
}

// arg methods

struct _FugaMethod_argMethod {
    Fuga* (*fp)(Fuga*, Fuga*);
};

Fuga* _FugaMethod_argMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_argMethod* method = self->data;
    return method->fp(recv, args);
}

Fuga* FugaMethod_arg(Fuga* self, Fuga* (*fp)(Fuga*, Fuga*))
{
    self = FugaMethod_new(self, _FugaMethod_argMethodCall);
    struct _FugaMethod_argMethod* method = FugaGC_alloc(
        self,
        sizeof(struct _FugaMethod_argMethod)
    );
    method->fp = fp;
    self->data = method;
    self->size = 1;
    return self;
}

// str method

struct _FugaMethod_strMethod {
    Fuga* (*fp)(Fuga*);
};

Fuga* _FugaMethod_strMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_strMethod* method = self->data;
    FUGA_NEED(args);
    if (!Fuga_hasNumSlots(args, 0)) {
        FUGA_RAISE(FUGA->ValueError, "str: expected no arguments");
    }
    if (Fuga_isTrue(Fuga_hasSlotRaw(recv, FUGA_SYMBOL("name"))))
        return Fuga_getSlotRaw(recv, FUGA_SYMBOL("name"));
    return method->fp(recv);
}

Fuga* FugaMethod_strMethod(Fuga* self, Fuga* (*fp)(Fuga*))
{
    self = FugaMethod_new(self, _FugaMethod_strMethodCall);
    struct _FugaMethod_strMethod* method = FugaGC_alloc(
        self,
        sizeof(struct _FugaMethod_strMethod)
    );
    method->fp = fp;
    self->data = method;
    self->size = 1;
    return self;
}

// 0 arg methods 

struct _FugaMethod_0argMethod {
    Fuga* (*fp)(Fuga*);
};

Fuga* _FugaMethod_0argMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_0argMethod* method = self->data;
    FUGA_NEED(args);
    if (!Fuga_hasNumSlots(args, 0)) {
        FUGA_RAISE(FUGA->ValueError, "expected no arguments");
    }
    return method->fp(recv);
}

Fuga* FugaMethod_0arg(Fuga* self, Fuga* (*fp)(Fuga*))
{
    self = FugaMethod_new(self, _FugaMethod_0argMethodCall);
    struct _FugaMethod_0argMethod* method = FugaGC_alloc(
        self,
        sizeof(struct _FugaMethod_0argMethod)
    );
    method->fp = fp;
    self->data = method;
    self->size = 1;
    return self;
}

// 1 arg methods

struct _FugaMethod_1argMethod {
    Fuga* (*fp)(Fuga*, Fuga*);
};

Fuga* _FugaMethod_1argMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_1argMethod* method = self->data;
    FUGA_NEED(args);
    if (!Fuga_hasNumSlots(args, 1)) {
        FUGA_RAISE(FUGA->ValueError, "expected 1 argument");
    }
    Fuga* arg0 = Fuga_getSlot(args, FUGA_INT(0));
    FUGA_CHECK(arg0);
    return method->fp(recv, arg0);
}

Fuga* FugaMethod_1arg(Fuga* self, Fuga* (*fp)(Fuga*, Fuga*))
{
    self = FugaMethod_new(self, _FugaMethod_1argMethodCall);
    struct _FugaMethod_1argMethod* method = FugaGC_alloc(
        self,
        sizeof(struct _FugaMethod_1argMethod)
    );
    method->fp = fp;
    self->data = method;
    self->size = 1;
    return self;
}

// 2 arg methods

struct _FugaMethod_2argMethod {
    Fuga* (*fp)(Fuga*, Fuga*, Fuga*);
};

Fuga* _FugaMethod_2argMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_2argMethod* method = self->data;
    FUGA_NEED(args);
    if (!Fuga_hasNumSlots(args, 2)) {
        FUGA_RAISE(FUGA->ValueError, "expected 2 arguments");
    }
    Fuga* arg0 = Fuga_getSlot(args, FUGA_INT(0));
    Fuga* arg1 = Fuga_getSlot(args, FUGA_INT(1));
    FUGA_CHECK(arg0); FUGA_CHECK(arg1);
    return method->fp(recv, arg0, arg1);
}

Fuga* FugaMethod_2arg(Fuga* self, Fuga* (*fp)(Fuga*, Fuga*, Fuga*))
{
    self = FugaMethod_new(self, _FugaMethod_2argMethodCall);
    struct _FugaMethod_2argMethod* method = FugaGC_alloc(
        self,
        sizeof(struct _FugaMethod_2argMethod)
    );
    method->fp = fp;
    self->data = method;
    self->size = 1;
    return self;
}

// fuga method

struct _FugaMethod_method {
    Fuga* scope;
    Fuga* args;
    Fuga* body;
};

Fuga* _FugaMethod_getScope(Fuga* self, Fuga* formals, Fuga* actuals)
{
    FUGA_NEED(self); FUGA_NEED(formals); FUGA_NEED(actuals);
    self = Fuga_clone(self);
    Fuga* numFormalsF = Fuga_numSlots(formals);
    FUGA_CHECK(numFormalsF);
    Fuga* numActualsF = Fuga_numSlots(actuals);
    FUGA_CHECK(numActualsF);

    size_t numFormals = FugaInt_value(numFormalsF);
    size_t numActuals = FugaInt_value(numActualsF);
    if (numFormals != numActuals)
        FUGA_RAISE(FUGA->TypeError, "expected different number of args");
    for (size_t i = 0; i < numFormals; i++) {
        Fuga* index  = FUGA_INT(i);
        Fuga* formal = Fuga_getSlot(formals, index);
        Fuga* actual = Fuga_getSlot(actuals, index);
        FUGA_CHECK(formal);
        FUGA_CHECK(actual);
        FUGA_CHECK(Fuga_setSlot(self, formal, actual));
    }
    // FIXME: handle thunks
    return self;
}

void _FugaMethod_methodMark(void* _method)
{
    struct _FugaMethod_method *method = _method;
    FugaGC_mark(method, method->scope);
    FugaGC_mark(method, method->args);
    FugaGC_mark(method, method->body);
}

Fuga* _FugaMethod_methodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    // FIXME: check to see if 'thunkSlots' is necessary
    FUGA_CHECK(self); FUGA_CHECK(recv); FUGA_CHECK(args);
    ALWAYS(Fuga_isMethod(self));
    struct _FugaMethod_method* method = self->data;
    Fuga* scope = _FugaMethod_getScope(method->scope, method->args,args);
    FUGA_CHECK(scope);
    FUGA_CHECK(Fuga_setSlot(scope, FUGA_SYMBOL("self"), recv));
    return Fuga_eval(method->body, scope, scope);
}

Fuga* FugaMethod_method(Fuga* scope, Fuga* args, Fuga* body)
{
    FUGA_NEED(scope);
    FUGA_NEED(args);
    FUGA_NEED(body);
    Fuga* self = FugaMethod_new(args, _FugaMethod_methodCall);
    FUGA_NEED(self);
    struct _FugaMethod_method* method = FugaGC_alloc(self,
                            sizeof(struct _FugaMethod_method));
    FugaGC_onMark(method, _FugaMethod_methodMark);
    method->scope = scope;
    method->args  = args;
    method->body  = body;
    self->data = method;
    self->size = 1;
    return self;
}

