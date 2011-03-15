#include "prelude.h"
#include "method.h"
#include "test.h"

void FugaPrelude_init(
    void* self
) {
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Object"),  FUGA->Object);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Prelude"), FUGA->Prelude);

    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Bool"),    FUGA->Bool);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("true"),    FUGA->True);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("false"),   FUGA->False);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("nil"),     FUGA->nil);

    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Number"),  FUGA->Number);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Int"),     FUGA->Int);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("String"),  FUGA->String);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Symbol"),  FUGA->Symbol);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Method"),  FUGA->Method);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Msg"),     FUGA->Msg);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Expr"),    FUGA->Msg);

    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("Exception"),
                                       FUGA->Exception);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("SyntaxError"),
                                       FUGA->SyntaxError);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("SlotError"),
                                       FUGA->SlotError);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("TypeError"),
                                       FUGA->TypeError);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("ValueError"),
                                       FUGA->ValueError);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("TypeError"),
                                       FUGA->TypeError);
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("IOError"),
                                       FUGA->IOError);

    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("name"),
                                FUGA_STRING("Prelude"));

    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("="),
                                FUGA_METHOD(FugaPrelude_equals));
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("if"),
                                FUGA_METHOD(FugaPrelude_if));
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("method"),
                                FUGA_METHOD(FugaPrelude_method));
    Fuga_setBy_to_(FUGA->Prelude, FUGA_SYMBOL("print"),
                                FUGA_METHOD(FugaPrelude_print));
}

void* FugaPrelude_equals(
    void* self,
    void* args
) {
    ALWAYS(self);
    ALWAYS(args);
    FUGA_NEED(self);
    if (!Fuga_isLazy(args))
        FUGA_RAISE(FUGA->TypeError, "=: arguments must be a thunk");

    void* code  = Fuga_lazyCode(args);
    void* scope = Fuga_lazyScope(args);
    FUGA_NEED(code); FUGA_NEED(scope);

    if (!Fuga_hasLength_(code, 2))
        FUGA_RAISE(FUGA->TypeError, "=: expected 2 arguments");

    void* recv = scope;
    void* lhs = Fuga_getBy_(code, FUGA_INT(0));
    void* rhs = Fuga_getBy_(code, FUGA_INT(1));
    FUGA_NEED(lhs); FUGA_NEED(rhs);
    
    if (Fuga_isMsg(lhs) || Fuga_isInt(lhs)) {
        recv = Fuga_getBy_(recv, FUGA_SYMBOL("this"));
    } else if (Fuga_isExpr(lhs)) {
        long numMsgs = FugaInt_value(Fuga_length(lhs));
        for (long i = 0; i < numMsgs-1; i++) {
            void* slot = Fuga_getBy_(lhs, FUGA_INT(i));
            FUGA_CHECK(slot);
            recv = Fuga_getBy_(recv, slot);
            FUGA_CHECK(recv);
        }
        lhs = Fuga_getBy_(lhs, FUGA_INT(numMsgs-1));
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
    FUGA_CHECK(Fuga_setBy_to_(recv, lhs, rhs));
    return FUGA->nil;
}


#ifdef TESTING
#define FUGA_EQUALS_TEST(x,y,z)
TESTS(FugaPrelude_equals) {
    void* self = Fuga_init();

    void* args;
    void* lhs;
    void* scope = Fuga_clone(FUGA->Object);
    void* dest  = Fuga_clone(FUGA->Object);
    void* foo   = Fuga_clone(FUGA->Object);
    void* result;

    Fuga_setBy_to_(scope, FUGA_SYMBOL("this"), dest);
    Fuga_setBy_to_(scope, FUGA_SYMBOL("foo"),  foo);

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_INT(0));
    Fuga_append_(args, FUGA_INT(10));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_getBy_(dest, FUGA_INT(0)), 10));

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_MSG("a"));
    Fuga_append_(args, FUGA_INT(20));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_getBy_(dest, FUGA_INT(1)), 20));
    TEST(FugaInt_is_(Fuga_getBy_(dest, FUGA_SYMBOL("a")), 20));

    lhs = Fuga_clone(FUGA->Expr);
    Fuga_append_(lhs, FUGA_MSG("foo"));
    Fuga_append_(lhs, FUGA_MSG("a"));
    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, lhs);
    Fuga_append_(args, FUGA_INT(30));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_getBy_(foo, FUGA_INT(0)), 30));
    TEST(FugaInt_is_(Fuga_getBy_(foo, FUGA_SYMBOL("a")), 30));

    Fuga_quit(self);
}
#endif

void* FugaPrelude_if(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    FUGA_NEED(self);
    args = Fuga_lazySlots(args);
    FUGA_CHECK(args);
    // FIXME: allow multiple arguments (2 or more).
    if (!Fuga_hasLength_(args, 3))
        FUGA_RAISE(FUGA->TypeError, "if: expected 3 arguments");
    void* cond = Fuga_getBy_(args, FUGA_INT(0));
    FUGA_NEED(cond);
    if (Fuga_isTrue(cond))
        return Fuga_needOnce(Fuga_getBy_(args, FUGA_INT(1)));
    if (Fuga_isFalse(cond))
        return Fuga_needOnce(Fuga_getBy_(args, FUGA_INT(2)));
    FUGA_RAISE(FUGA->TypeError,
        "if: condition must be a boolean"
    );
}

void* FugaPrelude_method(
    void* self,
    void* args
) {
    void* scope = Fuga_lazyScope(args);
    void* code  = Fuga_lazyCode(args);
    if (!Fuga_hasLength_(code, 2))
        FUGA_RAISE(FUGA->TypeError, "method: expected 2 arguments");
    void* formals = Fuga_getBy_(code, FUGA_INT(0));
    void* body    = Fuga_getBy_(code, FUGA_INT(1));

    return FugaMethod_method(scope, formals, body);
}


void* FugaPrelude_print(
    void* self,
    void* args
) {
    FUGA_NEED(args);
    size_t numSlots = FugaInt_value(Fuga_length(args));
    // FIXME: use a string builder
    void* totalStr = FUGA_STRING("");
    for (size_t i = 0; i < numSlots; i++) {
        if (i > 0) totalStr = FugaString_cat_(totalStr, FUGA_STRING(" "));
        void* arg = Fuga_getBy_(args, FUGA_INT(i));
        FUGA_NEED(totalStr);
        if (!Fuga_isString(arg)) {
            arg = Fuga_str(arg);
            FUGA_NEED(arg);
        }
        totalStr = FugaString_cat_(totalStr, arg);
        FUGA_CHECK(totalStr);
    }
    FugaString_print(totalStr);
    return FUGA->nil;
}

