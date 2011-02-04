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
    Fuga* SlotError;
    Fuga* MutableError;
    Fuga* TypeError;
    Fuga* ValueError;
    Fuga* IOError;
};

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

// General Functions
Fuga* Fuga_clone(Fuga*);
Fuga* Fuga_raise(Fuga*);
Fuga* Fuga_is(Fuga*, Fuga*);

uint64_t Fuga_newID(Fuga*);

#define Fuga_isTrue(self)   (!Fuga_isRaised(self) && \
                             (self)->id == FUGA->True->id)
#define Fuga_isFalse(self)  (!Fuga_isRaised(self) && \
                             (self)->id == FUGA->False->id)
#define Fuga_isNil(self)    (!Fuga_isRaised(self) && \
                             (self)->id == FUGA->nil->id)
#define Fuga_isInt(self)    (!Fuga_isRaised(self) && \
                             (self)->type == FUGA_TYPE_INT)
#define Fuga_isString(self) (!Fuga_isRaised(self) && \
                             (self)->type == FUGA_TYPE_STRING)
#define Fuga_isSymbol(self) (!Fuga_isRaised(self) && \
                             (self)->type == FUGA_TYPE_SYMBOL)
#define Fuga_isMsg(self)    (!Fuga_isRaised(self) && \
                             (self)->type == FUGA_TYPE_MSG)
#define Fuga_isMethod(self) (!Fuga_isRaised(self) && \
                             (self)->type == FUGA_TYPE_METHOD)
#define Fuga_isRaised(self)  (((size_t)(self)) & 0x01)

// Error handling
Fuga* Fuga_raise(Fuga*);
#define FUGA_RAISE(type, msg) do{                                      \
        Fuga* error##__LINE__ = Fuga_clone(type);                      \
        Fuga_set(error##__LINE__, FUGA_SYMBOL("msg"),FUGA_STRING(msg));\
        return Fuga_raise(error##__LINE__);                            \
    } while(0)
#define FUGA_RERAISE(error) return Fuga_raise(error)

Fuga* Fuga_catch(Fuga*);
#define FUGA_CHECK(result) do{                          \
        Fuga* error##__LINE__ = result;                 \
        if (Fuga_isRaised(error##__LINE__))             \
            return error##__LINE__;                     \
    } while(0)

// Generic Functions
Fuga* Fuga_proto(Fuga*);
Fuga* Fuga_rawHas(Fuga*, Fuga*);
Fuga* Fuga_rawGet(Fuga*, Fuga*);
Fuga* Fuga_has(Fuga*, Fuga*);
Fuga* Fuga_get(Fuga*, Fuga*);
Fuga* Fuga_set(Fuga*, Fuga*, Fuga*);
Fuga* Fuga_hasDoc(Fuga*, Fuga*);
Fuga* Fuga_getDoc(Fuga*, Fuga*);
Fuga* Fuga_setDoc(Fuga*, Fuga*, Fuga*);
Fuga* Fuga_call(Fuga*, Fuga*, Fuga*);

// Derived Functions
Fuga* Fuga_need(Fuga*);
Fuga* Fuga_send(Fuga*, const char*, Fuga*);
Fuga* Fuga_isa(Fuga*, Fuga*);


#endif

