#include "method.h"
#include "test.h"

void FugaMethod_init(Fuga* self)
{
    Fuga_set(FUGA->Method, FUGA_SYMBOL("str"),
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

struct _FugaMethod_strMethod {
    Fuga* (*fp)(Fuga*);
};

Fuga* _FugaMethod_strMethodCall(Fuga* self, Fuga* recv, Fuga* args)
{
    struct _FugaMethod_strMethod* method = self->data;
    FUGA_NEED(args);
    if (!FugaInt_isEqualTo(Fuga_length(args), 0)) {
        FUGA_RAISE(FUGA->ValueError, "str: expected no arguments");
    }
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
    return self;
}


