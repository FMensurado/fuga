#include "method.h"
#include "test.h"

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

