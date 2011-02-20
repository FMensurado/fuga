#ifndef FUGA_H
#define FUGA_H

#include "gc.h"
#include "common.h"
#include "symbols.h"
#include "slots.h"

typedef struct FugaRoot FugaRoot;
typedef const struct FugaType FugaType;
typedef struct FugaHeader FugaHeader;
typedef uint64_t FugaID;
typedef Fuga* (*FugaMethod)(Fuga* self, Fuga* recv, Fuga* args);

/**
 * A single Fuga object.
 */
struct Fuga {
    FugaRoot*  root;
    FugaType*  type;
    Fuga*      proto;
    FugaSlots* slots;
    FugaID     id;
    size_t     size;
    void*      data;
    FugaMethod method;
};

/**
 * Holds references to important Fuga objects and things.
 */
struct FugaRoot {
    FugaSymbols* symbols;
    FugaID lastID;

    // Important protos
    Fuga* Object;
    Fuga* Prelude;

    // singletons
    Fuga* Bool;
    Fuga* True;
    Fuga* False;
    Fuga* nil;

    // Primitive protos
    Fuga* Number;
    Fuga* Int;
    Fuga* String;
    Fuga* Symbol;
    Fuga* Msg;
    Fuga* Method;
    Fuga* Expr;

    // Exception protos
    Fuga* Exception;
    Fuga* SyntaxError;
    Fuga* SlotError;
    Fuga* MutableError;
    Fuga* TypeError;
    Fuga* ValueError;
    Fuga* IOError;
};

#define FUGA_ID 0

#define FUGA (self->root)

/**
 * Defines how a Fuga object acts.
 */
struct FugaType {
    Fuga* (*proto)(Fuga*);
    Fuga* (*has)(Fuga*, Fuga*);
    Fuga* (*get)(Fuga*, Fuga*);
    Fuga* (*set)(Fuga*, Fuga*);
    Fuga* (*call)(Fuga*, Fuga*, Fuga*);
};

// Basic types
#include "int.h"
#include "string.h"
#include "symbol.h"
#include "msg.h"
#include "method.h"

#define FUGA_TYPE_MASK    7

#define FUGA_TYPE_INT     ((FugaType*)1)
#define FUGA_TYPE_STRING  ((FugaType*)2)
#define FUGA_TYPE_SYMBOL  ((FugaType*)3)
#define FUGA_TYPE_MSG     ((FugaType*)4)
#define FUGA_TYPE_METHOD  ((FugaType*)5)

#define FUGA_BOOL(x) ((x) ? FUGA->True : FUGA->False)

// Managing Fuga environments
Fuga* Fuga_init(void);
void  Fuga_quit(Fuga*);
void  Fuga_mark(void*);

void Fuga_initObject(Fuga* self);
void Fuga_initBool(Fuga* self);

// General Functions
Fuga* Fuga_clone(Fuga*);
Fuga* Fuga_raise(Fuga*);
Fuga* Fuga_is(Fuga*, Fuga*);
Fuga* Fuga_isa(Fuga*, Fuga*);
Fuga* Fuga_proto(Fuga*);

bool Fuga_isRaised(Fuga*);
bool Fuga_isTrue  (Fuga*);
bool Fuga_isFalse (Fuga*);
bool Fuga_isNil   (Fuga*);
bool Fuga_isInt   (Fuga*);
bool Fuga_isString(Fuga*);
bool Fuga_isSymbol(Fuga*);
bool Fuga_isMsg   (Fuga*);
bool Fuga_isMethod(Fuga*);
bool Fuga_isExpr  (Fuga*);

// Error handling
Fuga* Fuga_raise(Fuga*);
#define FUGA_RAISE(type, msg) do{                                      \
        Fuga* error##__LINE__ = Fuga_clone(type);                      \
        Fuga_set(error##__LINE__, FUGA_SYMBOL("msg"),FUGA_STRING(msg));\
        return Fuga_raise(error##__LINE__);                            \
    } while(0)
#define FUGA_RERAISE(error) return Fuga_raise(error)
#define FUGA_RAISEP(type, msg) do{                                 \
        printf("%s:%d: %s, %s\n", __FILE__, __LINE__, #type, msg); \
        FUGA_RAISE(type, msg);                                     \
    } while(0)

Fuga* Fuga_catch(Fuga*);
#define FUGA_CHECK(result) do{                          \
        Fuga* error##__LINE__ = result;                 \
        if (Fuga_isRaised(error##__LINE__))             \
            return error##__LINE__;                     \
    } while(0)



// Slot manipulation
Fuga* Fuga_length(Fuga*);
Fuga* Fuga_rawHas(Fuga*, Fuga*);
Fuga* Fuga_rawGet(Fuga*, Fuga*);
Fuga* Fuga_has(Fuga*, Fuga*);
Fuga* Fuga_get(Fuga*, Fuga*);
Fuga* Fuga_append(Fuga*, Fuga*);
Fuga* Fuga_set(Fuga*, Fuga*, Fuga*);
Fuga* Fuga_hasDoc(Fuga*, Fuga*);
Fuga* Fuga_getDoc(Fuga*, Fuga*);
Fuga* Fuga_setDoc(Fuga*, Fuga*, Fuga*);

// Thunks
Fuga* Fuga_thunk(Fuga* self, Fuga* scope);
Fuga* Fuga_need(Fuga*);
#define FUGA_NEED(result) FUGA_CHECK(Fuga_need(result))
Fuga* Fuga_needOnce(Fuga*);

// Eval
Fuga* Fuga_eval(Fuga* self, Fuga* recv, Fuga* scope);
Fuga* Fuga_evalSlots(Fuga* self, Fuga* scope);
Fuga* Fuga_evalSlotsIn(Fuga* self, Fuga* scope);
Fuga* Fuga_evalExpr(Fuga* self, Fuga* recv, Fuga* scope);

// Call
Fuga* Fuga_send(Fuga* self, Fuga* name, Fuga* args);
Fuga* Fuga_call(Fuga* self, Fuga* recv, Fuga* args);

// Utility
Fuga* Fuga_str(Fuga* self);
Fuga* Fuga_strSlots(Fuga* self);

void Fuga_printException(Fuga* self);

#endif

