#include "prelude.h"
#include "method.h"
#include "test.h"
#include "loader.h"

void FugaPrelude_defOp(
    void* self,
    const char* name
) {
    Fuga_setS(FUGA->Prelude, name, FUGA_METHOD_OP(name));
}

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
    Fuga_setS(FUGA->Prelude, "Expr",        FUGA->Expr);
    Fuga_setS(FUGA->Prelude, "Path",        FUGA->Path);
    Fuga_setS(FUGA->Prelude, "Exception",   FUGA->Exception);
    Fuga_setS(FUGA->Prelude, "SyntaxError", FUGA->SyntaxError);
    Fuga_setS(FUGA->Prelude, "SlotError",   FUGA->SlotError);
    Fuga_setS(FUGA->Prelude, "TypeError",   FUGA->TypeError);
    Fuga_setS(FUGA->Prelude, "ValueError",  FUGA->ValueError);
    Fuga_setS(FUGA->Prelude, "TypeError",   FUGA->TypeError);
    Fuga_setS(FUGA->Prelude, "IOError",     FUGA->IOError);
    Fuga_setS(FUGA->Prelude, "Thunk",       FUGA->Thunk);
    Fuga_setS(FUGA->Prelude, "Path",        FUGA->Path);
    Fuga_setS(FUGA->Prelude, "Loader",      FugaLoader_new(self));

    Fuga_setS(FUGA->Prelude, "_name",   FUGA_STRING("Prelude"));
    Fuga_setS(FUGA->Prelude, ":=",      FUGA_METHOD(FugaPrelude_set));
    Fuga_setS(FUGA->Prelude, "=",       FUGA_METHOD(FugaPrelude_modify));
    Fuga_setS(FUGA->Prelude, "if",      FUGA_METHOD(FugaPrelude_if));
    Fuga_setS(FUGA->Prelude, "fn",      FUGA_METHOD(FugaPrelude_method));
    Fuga_setS(FUGA->Prelude, "print",  FUGA_METHOD(FugaPrelude_print));
    Fuga_setS(FUGA->Prelude, "import", FUGA_METHOD(FugaPrelude_import));
    Fuga_setS(FUGA->Prelude, "match",  FUGA_METHOD(FugaPrelude_match));
    Fuga_setS(FUGA->Prelude, "do",     FUGA_METHOD(FugaPrelude_do));
    Fuga_setS(FUGA->Prelude, "def",    FUGA_METHOD(FugaPrelude_def));
    Fuga_setS(FUGA->Prelude, "help",   FUGA_METHOD(FugaPrelude_help));
    Fuga_setS(FUGA->Prelude, "try",    FUGA_METHOD(FugaPrelude_try));

    Fuga_setS(FUGA->Prelude, "is?",    FUGA_METHOD_2(FugaPrelude_is));
    Fuga_setS(FUGA->Prelude, "isa?",   FUGA_METHOD_2(FugaPrelude_isa));

    Fuga_setS(FUGA->Prelude, "or",     FUGA_METHOD(FugaPrelude_orM));
    Fuga_setS(FUGA->Prelude, "and",    FUGA_METHOD(FugaPrelude_andM));
    Fuga_setS(FUGA->Prelude, "not",    FUGA_METHOD_1(FugaPrelude_notM));

    FugaPrelude_defOp(FUGA->Prelude, "==");
    FugaPrelude_defOp(FUGA->Prelude, "!=");
    FugaPrelude_defOp(FUGA->Prelude, "<");
    FugaPrelude_defOp(FUGA->Prelude, ">");
    FugaPrelude_defOp(FUGA->Prelude, "<=");
    FugaPrelude_defOp(FUGA->Prelude, ">=");

    FugaPrelude_defOp(FUGA->Prelude, "+");
    FugaPrelude_defOp(FUGA->Prelude, "-");
    FugaPrelude_defOp(FUGA->Prelude, "*");
    FugaPrelude_defOp(FUGA->Prelude, "/");
    FugaPrelude_defOp(FUGA->Prelude, "//");
    FugaPrelude_defOp(FUGA->Prelude, "%");
    FugaPrelude_defOp(FUGA->Prelude, "++");
}

void* FugaPrelude_is(void* self, void* a, void* b) {
    FUGA_NEED(a); FUGA_NEED(b);
    return FUGA_BOOL(Fuga_is_(a, b));
}

void* FugaPrelude_isa(void* self, void* a, void* b) {
    FUGA_NEED(a); FUGA_NEED(b);
    return FUGA_BOOL(Fuga_isa_(a, b));
}

void* FugaPrelude_set(
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
    void* lhs  = Fuga_get(code, FUGA_INT(0));
    void* rhs  = Fuga_get(code, FUGA_INT(1));
    FUGA_NEED(lhs); FUGA_NEED(rhs);

    if (Fuga_isMsg(lhs) || Fuga_isInt(lhs)) {
        recv = Fuga_get(recv, FUGA_SYMBOL("_this"));
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
    FUGA_IF(Fuga_hasS(scope, "_doc"))
        FUGA_CHECK(Fuga_setDoc(recv, lhs, Fuga_getS(scope, "_doc")));
    return FUGA->nil;
}

void* FugaPrelude_modify(
    void* self,
    void* args
) {
    ALWAYS(self);
    ALWAYS(args);
    FUGA_NEED(self);
    if (!Fuga_isLazy(args))
        FUGA_RAISE(FUGA->TypeError, ":= : arguments must be a thunk");

    void* code  = Fuga_lazyCode(args);
    void* scope = Fuga_lazyScope(args);
    FUGA_NEED(code); FUGA_NEED(scope);

    if (!Fuga_hasLength_(code, 2))
        FUGA_RAISE(FUGA->TypeError, ":= : expected 2 arguments");

    void* recv = scope;
    void* lhs  = Fuga_get(code, FUGA_INT(0));
    void* rhs  = Fuga_get(code, FUGA_INT(1));
    FUGA_NEED(lhs); FUGA_NEED(rhs);

    if (Fuga_isMsg(lhs) || Fuga_isInt(lhs)) {
        recv = Fuga_get(recv, FUGA_SYMBOL("_this"));
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
            ":= : left-hand side must be a msg or an int"
        );
    }

    FUGA_NEED(recv);

    rhs = Fuga_eval(rhs, scope, scope);
    FUGA_CHECK(rhs);
    FUGA_CHECK(Fuga_modify(recv, lhs, rhs));
    FUGA_IF(Fuga_hasS(scope, "_doc"))
        FUGA_CHECK(Fuga_setDoc(recv, lhs, Fuga_getS(scope, "_doc")));
    return FUGA->nil;
}


#ifdef TESTING
#define FUGA_EQUALS_TEST(x,y,z)
TESTS(FugaPrelude_set) {
    void* self = Fuga_init();

    void* args;
    void* lhs;
    void* scope = Fuga_clone(FUGA->Object);
    void* dest  = Fuga_clone(FUGA->Object);
    void* foo   = Fuga_clone(FUGA->Object);
    void* result;

    Fuga_set(scope, FUGA_SYMBOL("_this"), dest);
    Fuga_set(scope, FUGA_SYMBOL("foo"),  foo);

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_INT(0));
    Fuga_append_(args, FUGA_INT(10));
    result = FugaPrelude_set(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_INT(0)), 10));

    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, FUGA_MSG("a"));
    Fuga_append_(args, FUGA_INT(20));
    result = FugaPrelude_set(self, Fuga_lazy_(args, scope));
    TEST(Fuga_isNil(result));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_INT(1)), 20));
    TEST(FugaInt_is_(Fuga_get(dest, FUGA_SYMBOL("a")), 20));

    lhs = Fuga_clone(FUGA->Expr);
    Fuga_append_(lhs, FUGA_MSG("foo"));
    Fuga_append_(lhs, FUGA_MSG("a"));
    args = Fuga_clone(FUGA->Object);
    Fuga_append_(args, lhs);
    Fuga_append_(args, FUGA_INT(30));
    result = FugaPrelude_set(self, Fuga_lazy_(args, scope));
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

    long length = Fuga_length(args);
    if (length < 2)
        FUGA_RAISE(FUGA->TypeError, "if: expected 2 or more arguments");
    for (long i = 0; i < length-1; i += 2) {
        void* cond = Fuga_getI(args, i);
        FUGA_NEED(cond);
        if (Fuga_isTrue(cond))
            return Fuga_needOnce(Fuga_getI(args, i+1));
        if (!Fuga_isFalse(cond))
            FUGA_RAISE(FUGA->TypeError,
                "if: expected condition to be boolean"
            );
    }
    if (length & 1)
        return Fuga_needOnce(Fuga_getI(args, length-1));
    else
        return FUGA->nil;
}

void* FugaPrelude_orM(void* self, void* args) {
	ALWAYS(self); ALWAYS(args);
	FUGA_NEED(self);
	args = Fuga_lazySlots(args);
	FUGA_CHECK(args);

	FUGA_FOR(i, arg, args) {
		FUGA_NEED(arg);
		if (Fuga_isTrue(arg))
			return FUGA->True;
		if (!Fuga_isFalse(arg))
			FUGA_RAISE(FUGA->TypeError, "or: expected only booleans");
	}

	return FUGA->False;
}

void* FugaPrelude_andM    (void* self, void* args) {
	ALWAYS(self); ALWAYS(args);
	FUGA_NEED(self);
	args = Fuga_lazySlots(args);
	FUGA_CHECK(args);

	FUGA_FOR(i, arg, args) {
		FUGA_NEED(arg);
		if (Fuga_isFalse(arg))
			return FUGA->False;
		if (!Fuga_isTrue(arg))
			FUGA_RAISE(FUGA->TypeError, "and: expected only booleans");
	}

	return FUGA->True;
}

void* FugaPrelude_notM  (void* self, void* arg) {
	ALWAYS(self); ALWAYS(arg);
	FUGA_NEED(self); FUGA_NEED(arg);
	if (Fuga_isFalse(arg))
		return FUGA->True;
	if (Fuga_isTrue(arg))
		return FUGA->False;
	FUGA_RAISE(FUGA->TypeError, "not: expected only a boolean");
}

void* FugaPrelude_method(
    void* self,
    void* args
) {
    void* scope = Fuga_lazyScope(args);
    void* code  = Fuga_lazyCode(args);
    void* method = FugaMethod_empty(scope);
    FUGA_CHECK(scope); FUGA_CHECK(code); FUGA_CHECK(method);
    
    bool oscope = true;

    FUGA_FOR(i, body, code) {
        void* args;
        FUGA_IF(Fuga_hasNameI(code, i)) {
            args = Fuga_getNameI(code, i);
            if (Fuga_isSymbol(args)) {
                if (oscope) {
                    scope  = Fuga_clone(scope);
                    oscope = false;
                    FUGA_CHECK(Fuga_setS(method, "scope", scope));
                }
                body = Fuga_eval(body, scope, scope);
                FUGA_CHECK(Fuga_set(scope, args, body));
                continue;
            }
        } else {
            args = Fuga_clone(FUGA->Object);
        }
        FUGA_CHECK(args);
        FUGA_CHECK(FugaMethod_addPattern(method, args, body));
    }

    return method;
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
    void* target = Fuga_getS(Fuga_lazyScope(args), "_this");
    FUGA_CHECK(target);
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

    void* name = arg;
    if (Fuga_isExpr(name)) {
        // packages -- very ad hoc ish. Do they need their own loader?
        FUGA_FOR(i, slot, name) {
            if (i < length-1) {
                FUGA_IF(Fuga_has(target, slot)) {
                    target = Fuga_get(target, slot);
                    FUGA_NEED(self);
                } else {
                    void* package = Fuga_clone(FUGA->Object);
                    FUGA_CHECK(package);
                    FUGA_CHECK(Fuga_set(target, slot, package));
                    target = package;
                }
            } else {
                FUGA_CHECK(Fuga_set(target, slot, module));
            }
        }
    } else {
        FUGA_CHECK(Fuga_set(target, name, module));
    }
    return FUGA->nil;
}

void* FugaPrelude_match(
    void* self,
    void* args
) {
    ALWAYS(self); ALWAYS(args);
    args = Fuga_lazySlots(args);
    FUGA_CHECK(self); FUGA_CHECK(args);
    long length = Fuga_length(args);
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
            FUGA_CATCH(FUGA->MatchError) {
                matched = false;
                break;
            }
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
    return Fuga_evalIn(code, scope);
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
        owner = Fuga_getS(scope, "_this");
        name  = FugaMsg_name(signature);
        args  = FugaMsg_args(signature);
    } else if (Fuga_isExpr(signature)) {
        void* lastmsg = NULL;
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

    // create the method (or add pattern if method already exists)

    void* method;

    FUGA_IF(Fuga_hasRaw(owner, name)) {
        method = Fuga_get(owner, name);
        if (Fuga_isMethod(method)) {
            // FIXME: make patterns be scope aware
            FUGA_CHECK(FugaMethod_addPattern(method, args, body));
            return FUGA->nil;
        }
    }

    method = FugaMethod_method(scope, args, body);
    FUGA_CHECK(Fuga_set(owner, name, method));
  { FUGA_IF(Fuga_hasS(scope, "_doc"))
        FUGA_CHECK(Fuga_setDoc(owner, name, Fuga_getS(scope, "_doc"))); }
    return FUGA->nil;
}


void* FugaPrelude_help(
    void* self,
    void* args
) {
    FUGA_NEED(self);
    args = Fuga_lazySlots(args);
    FUGA_CHECK(args);
    if (!Fuga_hasLength_(args, 1))
        FUGA_RAISE(FUGA->TypeError, "doc: expected 1 argument");

    void* arg   = Fuga_getI(args, 0);
    void* code  = Fuga_lazyCode(arg);
    void* scope = Fuga_lazyScope(arg);
    FUGA_CHECK(code); FUGA_CHECK(scope);

    void* recv;
    void* name;
    void* value;
    bool  bare = false;
    if (Fuga_isMsg(code)) {
        recv = scope;
        name = code;
    } else if (Fuga_isExpr(code)) {
        name = Fuga_getI(code, -1);
        recv = Fuga_clone(FUGA->Expr);
        FUGA_CHECK(Fuga_extend_(recv, code));
        FUGA_CHECK(Fuga_delI(recv, -1));
        recv = Fuga_eval(recv, scope, scope);
    } else {
        bare  = true;
        value = Fuga_eval(code, scope, scope);
    }


    void* doc = NULL;
    if (!bare) {
        FUGA_NEED(name);
        FUGA_NEED(recv);
        FUGA_IF(Fuga_has(recv, name)) {
            FUGA_IF(Fuga_hasDoc(recv, name)) {
                doc = Fuga_getDoc(recv, name);
            } else {
                bare = true;
            }
            value = Fuga_get(recv, name);
        } else {
            printf("No such slot.\n");
            return FUGA->nil;
        }
    }

    FUGA_CHECK(Fuga_print(code));

    if (doc) {
        FUGA_NEED(doc);
        if (!Fuga_isString(doc))
            FUGA_CHECK(doc = Fuga_str(doc));
        void* lines = FugaString_split_(doc, FUGA_STRING("\n"));
        FUGA_NEED(lines);
        FUGA_FOR(i, line, lines) {
            printf("    ");
            FugaString_print(line);
        }
    }

    void* slots = value;
    printf("    Slots:\n");
    while (slots) {
        FUGA_NEED(slots);

        long length = Fuga_length(slots);
        for (long i = 0; i < length; i++) {
            if (!Fuga_isTrue(Fuga_hasNameI(slots, i)))
                continue;
            FugaSymbol* name = Fuga_getNameI(slots, i);
            if (name->data[0] == '_')
                continue;

            void* doc = FUGA_STRING("");
          { FUGA_IF(Fuga_hasDocI(slots, i))
                FUGA_CHECK(doc = Fuga_getDocI(slots, i)); }
            FUGA_NEED(doc);
            if (!Fuga_isString(doc)) doc = FUGA_STRING("");
            doc = Fuga_getI(FugaString_split_(doc, FUGA_STRING("\n")), 0);
            FUGA_CHECK(doc);

            void* line = FUGA_STRING("        ");
            line = FugaString_cat_(line, FugaSymbol_toString(name));
            line = FugaString_cat_(line, FUGA_STRING("\t\t"));
            line = FugaString_cat_(line, doc);
            FUGA_CHECK(line);
            FugaString_print(line);
        }
        slots = Fuga_proto(slots);
        if (!slots) break;
        FUGA_IF(Fuga_hasRawS(slots, "_name")) {
            FugaString* name = Fuga_getRawS(slots, "_name");
            if (Fuga_isString(name)) {
                printf("    Slots from %s:\n", name->data);
            } else {
                printf("    Slots from next proto:\n");
            }
        } else {
            printf("    Slots from next proto:\n");
        }
    }

    return FUGA->nil;
}


void* FugaPrelude_try(void* self, void* args) {
    FUGA_CHECK(self);
    args = Fuga_lazySlots(args);
    FUGA_CHECK(args);

    long length = Fuga_length(args);
    void* value = Fuga_need(Fuga_getI(args, 0));
    if (Fuga_isRaised(value)) {
        void* error = Fuga_catch(value);
        for (int i = 1; i < length-1; i+=2) {
            void* proto = Fuga_getI(args, i);
            FUGA_NEED(proto);
            if (Fuga_isa_(error, proto)) {
                value = Fuga_getI(args, i+1);
                void* scope = Fuga_clone(Fuga_lazyScope(value));
                FUGA_CHECK(Fuga_setS(scope, "exception", error));
                value = Fuga_eval(Fuga_lazyCode(value), scope, scope);
                break;
            }
        }
    }

    if (!(length & 1)) {
        void *finally = Fuga_getI(args, length-1);
        FUGA_NEED(finally);
    }
    return value;
}

