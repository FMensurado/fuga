#include "prelude.h"
#include "method.h"
#include "test.h"

void FugaPrelude_init(
    Fuga* self
) {
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Object"),  FUGA->Object);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Prelude"), FUGA->Prelude);

    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Bool"),    FUGA->Bool);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("true"),    FUGA->True);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("false"),   FUGA->False);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("nil"),     FUGA->nil);

    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Number"),  FUGA->Number);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Int"),     FUGA->Int);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("String"),  FUGA->String);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Symbol"),  FUGA->Symbol);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Method"),  FUGA->Method);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Msg"),     FUGA->Msg);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Expr"),    FUGA->Msg);

    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("Exception"),
                                       FUGA->Exception);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("SyntaxError"),
                                       FUGA->SyntaxError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("SlotError"),
                                       FUGA->SlotError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("MutableError"),
                                       FUGA->MutableError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("TypeError"),
                                       FUGA->TypeError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("ValueError"),
                                       FUGA->ValueError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("TypeError"),
                                       FUGA->TypeError);
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("IOError"),
                                       FUGA->IOError);

    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("name"),
                                FUGA_STRING("Prelude"));

    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("="),
                                FUGA_METHOD(FugaPrelude_equals));
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("if"),
                                FUGA_METHOD(FugaPrelude_if));
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("method"),
                                FUGA_METHOD(FugaPrelude_method));
    Fuga_setSlot(FUGA->Prelude, FUGA_SYMBOL("print"),
                                FUGA_METHOD(FugaPrelude_print));
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

    if (!Fuga_hasNumSlots((code), 2))
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


#ifdef TESTING
#define FUGA_EQUALS_TEST(x,y,z)
TESTS(FugaPrelude_equals) {
    Fuga* self = Fuga_init();

    Fuga* args;
    Fuga* lhs;
    Fuga* scope = Fuga_clone(FUGA->Object);
    Fuga* dest  = Fuga_clone(FUGA->Object);
    Fuga* foo   = Fuga_clone(FUGA->Object);
    Fuga* result;

    Fuga_setSlot(scope, FUGA_SYMBOL("this"), dest);
    Fuga_setSlot(scope, FUGA_SYMBOL("foo"),  foo);

    args = Fuga_clone(FUGA->Object);
    Fuga_append(args, FUGA_INT(0));
    Fuga_append(args, FUGA_INT(10));
    result = FugaPrelude_equals(self, Fuga_thunk(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_isEqualTo(Fuga_getSlot(dest, FUGA_INT(0)), 10));

    args = Fuga_clone(FUGA->Object);
    Fuga_append(args, FUGA_MSG("a"));
    Fuga_append(args, FUGA_INT(20));
    result = FugaPrelude_equals(self, Fuga_thunk(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_isEqualTo(Fuga_getSlot(dest, FUGA_INT(1)), 20));
    TEST(FugaInt_isEqualTo(Fuga_getSlot(dest, FUGA_SYMBOL("a")), 20));

    lhs = Fuga_clone(FUGA->Expr);
    Fuga_append(lhs, FUGA_MSG("foo"));
    Fuga_append(lhs, FUGA_MSG("a"));
    args = Fuga_clone(FUGA->Object);
    Fuga_append(args, lhs);
    Fuga_append(args, FUGA_INT(30));
    result = FugaPrelude_equals(self, Fuga_thunk(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_isEqualTo(Fuga_getSlot(foo, FUGA_INT(0)), 30));
    TEST(FugaInt_isEqualTo(Fuga_getSlot(foo, FUGA_SYMBOL("a")), 30));

    Fuga_quit(self);
}
#endif

Fuga* FugaPrelude_if(
    Fuga* self,
    Fuga* args
) {
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self);
    args = Fuga_thunkSlots(args);
    FUGA_CHECK(args);
    // FIXME: allow multiple arguments (2 or more).
    if (!Fuga_hasNumSlots(args, 3))
        FUGA_RAISE(FUGA->TypeError, "if: expected 3 arguments");
    Fuga* cond = Fuga_getSlot(args, FUGA_INT(0));
    FUGA_NEED(cond);
    if (Fuga_isTrue(cond))
        return Fuga_needOnce(Fuga_getSlot(args, FUGA_INT(1)));
    if (Fuga_isFalse(cond))
        return Fuga_needOnce(Fuga_getSlot(args, FUGA_INT(2)));
    FUGA_RAISE(FUGA->TypeError,
        "if: condition must be a boolean"
    );
}

Fuga* FugaPrelude_method(
    Fuga* self,
    Fuga* args
) {
    Fuga* scope = Fuga_thunkScope(args);
    Fuga* code  = Fuga_thunkCode(args);
    if (!Fuga_hasNumSlots(code, 2))
        FUGA_RAISE(FUGA->TypeError, "method: expected 2 arguments");
    Fuga* formals = Fuga_getSlot(code, FUGA_INT(0));
    Fuga* body    = Fuga_getSlot(code, FUGA_INT(1));

    return FugaMethod_method(scope, formals, body);
}


Fuga* FugaPrelude_print(
    Fuga* self,
    Fuga* args
) {
    FUGA_NEED(args);
    size_t numSlots = FugaInt_value(Fuga_numSlots(args));
    // FIXME: use a string builder
    Fuga* totalStr = FUGA_STRING("");
    for (size_t i = 0; i < numSlots; i++) {
        if (i > 0) totalStr = FugaString_cat(totalStr, FUGA_STRING(" "));
        Fuga* arg = Fuga_getSlot(args, FUGA_INT(i));
        FUGA_NEED(totalStr);
        if (!Fuga_isString(arg)) {
            arg = Fuga_str(arg);
            FUGA_NEED(arg);
        }
        totalStr = FugaString_cat(totalStr, arg);
        FUGA_CHECK(totalStr);
    }
    FugaString_print(totalStr);
    return FUGA->nil;
}

