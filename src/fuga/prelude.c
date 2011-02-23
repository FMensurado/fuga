#include "prelude.h"
#include "method.h"
#include "test.h"

void FugaPrelude_init(
    Fuga* self
) {
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("="),
        FUGA_METHOD_ARG(FugaPrelude_equals));
}

Fuga* FugaPrelude_equals(
    Fuga* self,
    Fuga* args
) {
    ALWAYS(self);
    ALWAYS(args);
    FUGA_NEED(self);
    if (!Fuga_isThunk(args))
        FUGA_RAISE(FUGA->TypeError, "=: arguments must be a thunk");

    Fuga* code  = Fuga_thunkCode(args);
    Fuga* scope = Fuga_thunkScope(args);
    FUGA_NEED(code); FUGA_NEED(scope);

    if (!FugaInt_isEqualTo(Fuga_numSlots(code), 2))
        FUGA_RAISE(FUGA->TypeError, "=: expected 2 arguments");

    Fuga* recv = scope;
    Fuga* lhs = Fuga_getSlot(code, FUGA_INT(0));
    Fuga* rhs = Fuga_getSlot(code, FUGA_INT(1));
    FUGA_NEED(lhs); FUGA_NEED(rhs);
    
    if (Fuga_isMsg(lhs) || Fuga_isInt(lhs)) {
        recv = Fuga_getSlot(recv, FUGA_SYMBOL("this"));
    } else if (Fuga_isExpr(lhs)) {
        long numMsgs = FugaInt_value(Fuga_numSlots(lhs));
        for (long i = 0; i < numMsgs-1; i++) {
            Fuga* slot = Fuga_getSlot(lhs, FUGA_INT(i));
            FUGA_CHECK(slot);
            recv = Fuga_getSlot(recv, slot);
            FUGA_CHECK(recv);
        }
        lhs = Fuga_getSlot(lhs, FUGA_INT(numMsgs-1));
        FUGA_CHECK(lhs);
    }

    if (!(Fuga_isMsg(lhs) || Fuga_isInt(lhs))) {
        FUGA_RAISE(FUGA->TypeError,
            "=: left-hand side must be a msg or an int"
        );
    }

    FUGA_NEED(recv);

    rhs = Fuga_eval(rhs, scope, scope);
    FUGA_CHECK(rhs);
    FUGA_CHECK(Fuga_setSlot(recv, lhs, rhs));
    return FUGA->nil;
}

