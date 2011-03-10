#ifndef FUGA_H
#define FUGA_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "gclist.h"

struct FugaRoot {
    // GC info
    FugaGCList white;
    FugaGCList grey;
    FugaGCList black;

    // basic objects
    void* Object;
    void* Prelude;
    void* Number;
    void* Int;
    void* String;
    void* Symbol;
    void* Msg;
    void* Method;
    void* Name;
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
};

struct FugaGCInfo {
    FugaGCList list;
    unsigned   pass;
    bool       root;
};

struct FugaHeader {
    FugaGCInfo  gc;
    FugaRoot*   root;
    FugaType*   type;
    FugaSlots*  slots;
    void*       proto;
};

#define FUGA_HEADER(self)   (((FugaHeader*)(self))-1)
#define FUGA_DATA(self)     (((FugaHeader*)(self))+1)

#define FUGA (FUGA_HEADER(self)->root)

//
void* Fuga_init(void);
void  Fuga_quit(void*);

void* Fuga_clone(void* proto);
void* Fuga_clone_(void* proto, size_t size);

// Garbage collection
void* Fuga_onMark_(void* self, void markFn(void*));
void* Fuga_onFree_(void* self, void freeFn(void*));
void  Fuga_mark_(void* self, void* child);

// Primitives
FugaType Fuga_type   (void* self);
void     Fuga_type_  (void* self, FugaType* type);

extern static const FugaType FugaInt_type;
extern static const FugaType FugaString_type;
extern static const FugaType FugaSymbol_type;
extern static const FugaType FugaMsg_type;
extern static const FugaType FugaExpr_type;
extern static Inconst FugaType FugaMethod_type;

bool Fuga_isType_  (void* self, FugaType* type);
bool Fuga_isInt    (void* self);
bool Fuga_isString (void* self);
bool Fuga_isSymbol (void* self);
bool Fuga_isMsg    (void* self);
bool Fuga_isExpr   (void* self);
bool Fuga_isMethod (void* self);

// Identity
bool Fuga_is      (void* self, void* other);  // XXX: returning bool?
bool Fuga_isa     (void* self, void* other);  // XXX: returning bool?
bool Fuga_isTrue  (void* self);
bool Fuga_isFalse (void* self);
bool Fuga_isNil   (void* self);

// Exception Handling
bool  Fuga_isRaised (void* self);
void* Fuga_raise(void* self);
void* Fuga_catch(void* self);

#define FUGA_RAISE(,)
#define FUGA_CHECK

// Thunks

// Slot Manipulation
bool  Fuga_has_         (void* self, void* name);
bool  Fuga_hasRaw_      (void* self, void* name);
bool  Fuga_hasName_     (void* self, void* name);
bool  Fuga_hasDoc_      (void* self, void* name);
void* Fuga_get_         (void* self, void* name);
void* Fuga_getRaw_      (void* self, void* name);
void* Fuga_getName_     (void* self, void* name);
void* Fuga_getDoc_      (void* self, void* name);
void* Fuga_set_to_      (void* self, void* name, void* value);
void* Fuga_setDoc_to_   (void* self, void* name, void* value);

bool  Fuga_hasAt_       (void* self, size_t index);
bool  Fuga_hasNameAt_   (void* self, size_t index);
bool  Fuga_hasDocAt_    (void* self, size_t index);
void* Fuga_getAt_       (void* self, size_t index);
void* Fuga_getNameAt_   (void* self, size_t index);
void* Fuga_getDocAt_    (void* self, size_t index);
void* Fuga_setAt_to_    (void* self, size_t index, void* value);
void* Fuga_setDocAt_to_ (void* self, size_t index, void* value);

void* Fuga_append_      (void* self, void* value);

#endif

