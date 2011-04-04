#include "prelude.h"
#include "method.h"
#include "test.h"
#include "loader.h"

void FugaPrelude_init(
    void* self
) {
    Fuga_setS(FUGA->Prelude, "Object",      FUGA->Object);
    Fuga_setS(FUGA->Prelude, "Prelude",     FUGA->Prelude);
    Fuga_setS(FUGA->Prelude, "Bool",        FUGA->Bool);
    Fuga_setS(FUGA->Prelude, "true",        FUGA->True);
    Fuga_setS(FUGA->Prelude, "false",       FUGA->False);
    Fuga_setS(FUGA->Prelude, "nil",         FUGA->nil);
    Fuga_setS(FUGA->Prelude, "Number",      FUGA->Number);
    Fuga_setS(FUGA->Prelude, "Int",         FUGA->Int);
    Fuga_setS(FUGA->Prelude, "String",      FUGA->String);
    Fuga_setS(FUGA->Prelude, "Symbol",      FUGA->Symbol);
    Fuga_setS(FUGA->Prelude, "Method",      FUGA->Method);
    Fuga_setS(FUGA->Prelude, "Msg",         FUGA->Msg);
    Fuga_setS(FUGA->Prelude, "Expr",        FUGA->Msg);
    Fuga_setS(FUGA->Prelude, "Exception",   FUGA->Exception);
    Fuga_setS(FUGA->Prelude, "SyntaxError", FUGA->SyntaxError);
    Fuga_setS(FUGA->Prelude, "SlotError",   FUGA->SlotError);
    Fuga_setS(FUGA->Prelude, "TypeError",   FUGA->TypeError);
    Fuga_setS(FUGA->Prelude, "ValueError",  FUGA->ValueError);
    Fuga_setS(FUGA->Prelude, "TypeError",   FUGA->TypeError);
    Fuga_setS(FUGA->Prelude, "IOError",     FUGA->IOError);
    Fuga_setS(FUGA->Prelude, "Loader",      FugaLoader_new(self));

    Fuga_setS(FUGA->Prelude, "name",   FUGA_STRING("Prelude"));
    Fuga_setS(FUGA->Prelude, "=",      FUGA_METHOD(FugaPrelude_equals));
    Fuga_setS(FUGA->Prelude, "if",     FUGA_METHOD(FugaPrelude_if));
    Fuga_setS(FUGA->Prelude, "method", FUGA_METHOD(FugaPrelude_method));
    Fuga_setS(FUGA->Prelude, "print",  FUGA_METHOD(FugaPrelude_print));
    Fuga_setS(FUGA->Prelude, "import", FUGA_METHOD(FugaPrelude_import));
    Fuga_setS(FUGA->Prelude, "match",  FUGA_METHOD(FugaPrelude_match));
    Fuga_setS(FUGA->Prelude, "do",     FUGA_METHOD(FugaPrelude_do));
    Fuga_setS(FUGA->Prelude, "def",    FUGA_METHOD(FugaPrelude_def));

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
    void* lhs = Fuga_get(code, FUGA_INT(0));
    void* rhs = Fuga_get(code, FUGA_INT(1));
    FUGA_NEED(lhs); FUGA_NEED(rhs);
    
    if (Fuga_isMsg(lhs) || Fuga_isInt(lhs)) {
        recv = Fuga_get(recv, FUGA_SYMBOL("this"));
    } else if (Fuga_isExpr(lhs)) {
        FUGA_FOR(i, slot, lhs) {
            if (i < length-1) {
                recv = Fuga_get(recv, slot);
                FUGA_CHECK(recv);
            } else {
                lhs = slot;
                FUGA_CHECK(lhs);
            }
        }
    }

    if (!(Fuga_isMsg(lhs) || Fuga_isInt(lhs))) {
        FUGA_RAISE(FUGA->TypeError,
            "=: left-hand side must be a msg or an int"
        );
    }

    FUGA_NEED(recv);

    rhs = Fuga_eval(rhs, scope, scope);
    FUGA_CHECK(rhs);
    FUGA_CHECK(Fuga_set(recv, lhs, rhs));
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

    Fuga_set(scope, FUGA_SYMBOL("this"), dest);
    Fuga_set(scope, FUGA_SYMBOL("foo"),  foo);

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_INT(0));
    Fuga_append_(args, FUGA_INT(10));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_INT(0)), 10));

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_MSG("a"));
    Fuga_append_(args, FUGA_INT(20));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_INT(1)), 20));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_SYMBOL("a")), 20));

    lhs = Fuga_clone(FUGA->Expr);
    Fuga_append_(lhs, FUGA_MSG("foo"));
    Fuga_append_(lhs, FUGA_MSG("a"));
    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, lhs);
    Fuga_append_(args, FUGA_INT(30));
    result = FugaPrelude_equals(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_get(foo, FUGA_INT(0)), 30));
    TEST(FugaInt_is_(Fuga_get(foo, FUGA_SYMBOL("a")), 30));

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
    void* cond = Fuga_get(args, FUGA_INT(0));
    FUGA_NEED(cond);
    if (Fuga_isTrue(cond))
        return Fuga_needOnce(Fuga_get(args, FUGA_INT(1)));
    if (Fuga_isFalse(cond))
        return Fuga_needOnce(Fuga_get(args, FUGA_INT(2)));
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
    void* formals = Fuga_get(code, FUGA_INT(0));
    void* body    = Fuga_get(code, FUGA_INT(1));

    return FugaMethod_method(scope, formals, body);
}


void* FugaPrelude_print(
    void* self,
    void* args
) {
    FUGA_NEED(args);
    void* totalStr = FUGA_STRING("");
    FUGA_FOR(i, arg, args) {
        if (i > 0)
            totalStr = FugaString_cat_(totalStr, FUGA_STRING(" "));
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

void* FugaPrelude_import(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    FUGA_CHECK(self); FUGA_CHECK(args);
    args = Fuga_lazySlots(args);
    if (!Fuga_hasLength_(args, 1))
        FUGA_RAISE(FUGA->SyntaxError, "import: expected only 1 arg");
    void* arg = Fuga_lazyCode(Fuga_getI(args, 0));
    FUGA_CHECK(arg);
    FugaLoader* loader = Fuga_getS(self, "Loader");
    FUGA_CHECK(loader);

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, arg);
    void* module = Fuga_send(loader, FUGA_SYMBOL("import"), args);
    FUGA_CHECK(module);

    // FIXME: what to do with packages???????
    void* name = arg;
    FUGA_CHECK(Fuga_set(self, name, module));
    return FUGA->nil;
}

void* FugaPrelude_match(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    args = Fuga_lazySlots(args);
    FUGA_CHECK(self); FUGA_CHECK(args);
    long length = FugaInt_value(Fuga_length(args));
    if (length < 3) 
        FUGA_RAISE(FUGA->TypeError, "match: expected at least 3 arguments");
    if ((length - 1) % 2)
        FUGA_RAISE(FUGA->TypeError, "match: expected odd number of args");

    long numPatterns = (length-1) / 2;
    void* value = Fuga_getI(args, 0);
    for (long i = 0; i < numPatterns; i++) {
        void* matcher = Fuga_lazyCode(Fuga_getI(args, i*2 + 1));
        FUGA_CHECK(matcher);
        void* result = Fuga_match_(matcher, value);
        bool matched = true;
        FUGA_TRY(result) {
            FUGA_CATCH(FUGA->MatchError)
                matched = false;
            FUGA_RERAISE;
        }
        
        if (matched) {
            void* body = Fuga_getI(args, i*2 + 2);
            FUGA_CHECK(body);
            void* bodyCode = Fuga_lazyCode(body);
            void* bodyScope = Fuga_lazyScope(body);
            FUGA_CHECK(bodyScope = Fuga_clone(bodyScope));
            FUGA_CHECK(Fuga_update_(bodyScope, result));
            return Fuga_eval(bodyCode, bodyScope, bodyScope);
        }
    }
    FUGA_RAISE(FUGA->TypeError, "match: no patterns matched");
}

void* FugaPrelude_do(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    void* scope = Fuga_lazyScope(args);
    void* code  = Fuga_lazyCode(args);
    FUGA_CHECK(scope); FUGA_CHECK(code);
    scope = Fuga_clone(scope);
    FUGA_CHECK(Fuga_setS(scope, "this", scope));

    void* result = Fuga_evalIn(code, scope);
    FUGA_CHECK(result);
    long  length = FugaInt_value(Fuga_length(result));
    if (length == 0)
        return FUGA->nil;
    else
        return Fuga_getI(result, length-1);
}

void* FugaPrelude_def(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    void* scope = Fuga_lazyScope(args);
    void* code  = Fuga_lazyCode(args);
    FUGA_CHECK(scope); FUGA_CHECK(code);
    if (Fuga_hasLength_(code, 0))
        FUGA_RAISE(FUGA->TypeError, "def: expected at least one arg");

    // get arguments
    void* signature = Fuga_getI(code, 0);
    void* body;
    
    if (Fuga_hasLength_(code, 1)) {
        body = FUGA_MSG("nil");
    } else if (Fuga_hasLength_(code, 2)) {
        body = Fuga_getI(code, 1);
    } else {
        body = FUGA_MSG("do");
        FUGA_FOR(i, slot, code) {
            if (i > 0)
                Fuga_append_(body, slot);
        }
   }

    // break apart signature
    void* owner;
    void* name;
    if (Fuga_isMsg(signature)) {
        owner = Fuga_getS(scope, "this");
        name  = FugaMsg_name(signature);
        args  = FugaMsg_args(signature);
    } else if (Fuga_isExpr(signature)) {
        void* lastmsg;
        void* ownerexpr = Fuga_clone(FUGA->Expr);
        FUGA_FOR(i, slot, signature) {
            if (i < length-1)
                Fuga_append_(ownerexpr, slot);
            else
                lastmsg = slot;
        }
        owner = Fuga_eval(ownerexpr, scope, scope);
        name  = FugaMsg_name(lastmsg);
        args  = FugaMsg_args(lastmsg);
    } else {
        FUGA_RAISE(FUGA->TypeError,
            "def: unexpected signature type"
        );
    }
    FUGA_CHECK(owner); FUGA_CHECK(name); FUGA_CHECK(args);

    // create the method
    // FIXME: check to see if method already exists
    void* method = FugaMethod_method(scope, args, body);
    FUGA_CHECK(Fuga_set(owner, name, method));

    return FUGA->nil;
}
