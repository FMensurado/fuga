#ifndef FUGA_H
#define FUGA_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t FugaIndex;
typedef struct FugaRoot   FugaRoot;
typedef struct FugaType   FugaType;
typedef struct FugaGCInfo FugaGCInfo;
typedef struct FugaHeader FugaHeader;
typedef struct FugaLazy   FugaLazy;
typedef struct FugaInt    FugaInt;
typedef struct FugaString FugaString;
typedef struct FugaSymbol FugaSymbol;
typedef struct FugaMethod FugaMethod;
typedef struct FugaMsg    FugaMsg;

#include "gclist.h"
#include "slots.h"
#include "symbols.h"

struct FugaRoot {
    // GC info
    FugaGCList white;
    FugaGCList grey;
    FugaGCList black;
    FugaGCList roots;

    // symbols
    FugaSymbols* symbols;

    // basic objects
    void* Object;
    void* Prelude;
    void* Number;
    void* Int;
    void* String;
    void* Symbol;
    void* Msg;
    void* Method;
    void* Expr;
    void* nil;
    void* Bool;
    void* True;     // "true"  is reserved in C++ :-/
    void* False;    // "false" is also reserved

    // Exceptions
    void* Exception;
    void* TypeError;
    void* SlotError;
    void* IOError;
    void* ValueError;
    void* SyntaxError;
};

struct FugaType {
    const char* name;
};

struct FugaGCInfo {
    FugaGCList  list;
    void        (*mark) (void*);
    void        (*free) (void*);
    unsigned    pass;
    bool        root;
};

struct FugaHeader {
    FugaGCInfo       gc;
    FugaRoot*        root;
    const FugaType*  type;
    FugaSlots*       slots;
    void*            proto;
};


#define FUGA_HEADER(self)   (((FugaHeader*)(self))-1)
#define FUGA_DATA(self)     ((void*)(((FugaHeader*)(self))+1))
#define FUGA                (FUGA_HEADER(self)->root)

//
void* Fuga_init(void);
void  Fuga_quit(void*);

void* Fuga_clone(void* proto);
void* Fuga_clone_(void* proto, size_t size);

// Garbage collection
void Fuga_onMark_(void* self, void (*markFn)(void*));
void Fuga_onFree_(void* self, void (*freeFn)(void*));
void Fuga_mark_(void* self, void* child);
void Fuga_collect(void* self);
void Fuga_root(void* self);
void Fuga_unroot(void* self);

// Primitives
const FugaType* Fuga_type   (void* self);
void            Fuga_type_  (void* self, const FugaType* type);

extern const FugaType FugaLazy_type;
extern const FugaType FugaInt_type;
extern const FugaType FugaString_type;
extern const FugaType FugaSymbol_type;
extern const FugaType FugaMsg_type;
extern const FugaType FugaExpr_type;
extern const FugaType FugaMethod_type;

bool Fuga_hasType_ (void* self, const FugaType* type);
bool Fuga_isLazy   (void* self);
bool Fuga_isInt    (void* self);
bool Fuga_isString (void* self);
bool Fuga_isSymbol (void* self);
bool Fuga_isMsg    (void* self);
bool Fuga_isMethod (void* self);


#include "lazy.h"
#include "int.h"
#include "string.h"
#include "symbol.h"
#include "string.h"
#include "method.h"
#include "msg.h"
#include "bool.h"

// Identity
bool Fuga_is_     (void* self, void* other);
bool Fuga_isa_    (void* self, void* other);
bool Fuga_isTrue  (void* self);
bool Fuga_isFalse (void* self);
bool Fuga_isNil   (void* self);
bool Fuga_isExpr  (void* self);

// Exception Handling
bool  Fuga_isRaised (void* self);
void* Fuga_raise(void* self);
void* Fuga_catch(void* self);

#define FUGA_RAISE(type, msg)                                           \
    do {                                                                \
        void* error##__LINE__ = Fuga_clone(type);                       \
        Fuga_set_to_(error##__LINE__, "msg", FUGA_STRING(msg));         \
        return Fuga_raise(error##__LINE__);                             \
    } while(0)

#define FUGA_CHECK(value)                                               \
    do {                                                                \
        void* error##__LINE__ = Fuga_catch(value);                      \
        if (error##__LINE__)                                            \
            return Fuga_raise(error##__LINE__);                         \
    } while(0)


// Slot Manipulation
void* Fuga_slots    (void* self);

FugaInt* Fuga_length       (void* self);
bool     Fuga_hasLength_   (void* self, long length);

void* Fuga_has_         (void* self, const char* name);
void* Fuga_hasRaw_      (void* self, const char* name);
void* Fuga_hasDoc_      (void* self, const char* name);
void* Fuga_get_         (void* self, const char* name);
void* Fuga_getRaw_      (void* self, const char* name);
void* Fuga_getDoc_      (void* self, const char* name);
void* Fuga_set_to_      (void* self, const char* name, void* value);
void* Fuga_setDoc_to_   (void* self, const char* name, void* value);

void* Fuga_hasBy_       (void* self, void* name);
void* Fuga_hasRawBy_    (void* self, void* name);
void* Fuga_hasNameBy_   (void* self, void* name);
void* Fuga_hasDocBy_    (void* self, void* name);
void* Fuga_getBy_       (void* self, void* name);
void* Fuga_getRawBy_    (void* self, void* name);
void* Fuga_getNameBy_   (void* self, void* name);
void* Fuga_getDocBy_    (void* self, void* name);
void* Fuga_setBy_to_    (void* self, void* name, void* value);
void* Fuga_setDocBy_to_ (void* self, void* name, void* value);

void* Fuga_hasAt_       (void* self, size_t index);
void* Fuga_hasNameAt_   (void* self, size_t index);
void* Fuga_hasDocAt_    (void* self, size_t index);
void* Fuga_getAt_       (void* self, size_t index);
void* Fuga_getNameAt_   (void* self, size_t index);
void* Fuga_getDocAt_    (void* self, size_t index);
void* Fuga_setAt_to_    (void* self, size_t index, void* value);
void* Fuga_setDocAt_to_ (void* self, size_t index, void* value);

void* Fuga_append_      (void* self, void* value);

// Calling & Sending
void* Fuga_call (void* self, void* recv, void* args);
void* Fuga_send (void* self, void* msg, void* args);

// Evaluation
void* Fuga_eval         (void* self, void* recv, void* scope);
void* Fuga_evalSlot     (void* self, void* scope, void* result);
void* Fuga_evalSlots    (void* self, void* scope);
void* Fuga_evalExpr     (void* self, void* recv, void* scope);

// Misc
void* Fuga_str(void* self);
void* Fuga_strSlots(void* self);

void  Fuga_printException(void* self);
void* Fuga_print(void* self);

#endif

