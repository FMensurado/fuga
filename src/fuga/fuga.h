#ifndef FUGA_H
#define FUGA_H
/** 
*** # Fuga, a homoiconic object-oriented programming language.
**/

#include "common.h"

/**
*** ### FugaData
***
*** Represents the various types of primitive data. This is meant
*** to be a 64bit (8bytes) union. It is useful to remember that size
*** 
**/
typedef union FugaData {
    struct FugaObject* OBJECT;
    FugaIndex INT;
    uint64_t* LONG;
    double    REAL;
    char*     SYMBOL;
    char*     STRING;
    Fuga*     MSG;
    bool      BOOL;
    void*     data;
} FugaData;

/**
*** ### FUGA_TYPE 
***
*** This is an 8bit value that tells whether 
**/

typedef uint8_t FugaType;

#define FUGA_TYPE_NONE   0x00 // not a primitive
#define FUGA_TYPE_INT    0x01 // for smallprimitive ints
#define FUGA_TYPE_LONG   0x02 // for long primitive ints
#define FUGA_TYPE_REAL   0x03 // for primitive reals
#define FUGA_TYPE_STRING 0x04 // for primitive strings
#define FUGA_TYPE_SYMBOL 0x05 // for primitive symbols
#define FUGA_TYPE_MSG    0x06 // for primitive messages
#define FUGA_TYPE_METHOD 0x07 // for primitive methods

// primitive singletons
#define FUGA_TYPE_OBJECT 0x08  // type of Object (the base object)
#define FUGA_TYPE_NIL    0x09  // type of nil
#define FUGA_TYPE_TRUE   0x0A  // type of true
#define FUGA_TYPE_FALSE  0x0B  // type of false

// flags
#define FUGA_ERROR  0x80

/**
*** ### FugaID
***
*** Unique identifier for each 
**/

#define FUGA_ID_LAZY    ((FugaID)0)
#define FUGA_ID_NEEDED  ((FugaID)0xFFFFFFFF)
#define FUGA_ID_OBJECT  ((FugaID)1)

#define FUGA_READY(obj) (!(((obj)->id == FUGA_ID_LAZY  ) || \
                           ((obj)->id == FUGA_ID_NEEDED)))

/**
*** ### Fuga
***
*** The `Fuga` struct represents an object in the world of Fuga.
*** 
*** - Fields:
***
***     - Object Hierarchy:
***         - `Fuga* Object`: Points to the root object.
***         - `Fuga* proto`: This object's prototype. If this is NULL, this
***         object must be Object, the root object.
***
***     - Slots:
***         - `FugaSlots* slots`: this object's slots.
***         - `uint32_t size`: number of slots
***
***     - Primitive Data:
***         - `uint32_t type`: refers to the type of the `data` field. It
***         uses predefined values, such as `FUGA_TYPE_INT` or
***         `FUGA_TYPE_NONE`.
***         - `char data[]`: any data that can't be captured by the slots
***         themselves. Used for primitves such as numbers, strings, built 
***         in functions.
***
**/

struct Fuga {
    Fuga* Object;
    Fuga* proto;
    struct FugaSlots* slots;
    FugaID id;

    // primitive type
    FugaType type;
    uint32_t size:24;
    FugaData data;
};

/**
*** ### FugaObject
**/
typedef struct FugaObject {
    FugaID id;
    struct FugaSymbols *symbols;
    Fuga* Prelude;
    Fuga* Number;
    Fuga* Int;
    Fuga* Real;
    Fuga* String;
    Fuga* Symbol;
    Fuga* Msg;
    Fuga* Bool;
    Fuga* _true;
    Fuga* _false;
    Fuga* nil;
} FugaObject;

#define FUGA_Object  (self->Object)
#define FUGA_gc      (self->Object->data.OBJECT->gc)
#define FUGA_Prelude (self->Object->data.OBJECT->Prelude)
#define FUGA_Number  (self->Object->data.OBJECT->Number)
#define FUGA_Int     (self->Object->data.OBJECT->Int)
#define FUGA_Real    (self->Object->data.OBJECT->Real)
#define FUGA_String  (self->Object->data.OBJECT->String)
#define FUGA_Symbol  (self->Object->data.OBJECT->Symbol)
#define FUGA_Msg     (self->Object->data.OBJECT->Msg)
#define FUGA_Bool    (self->Object->data.OBJECT->Bool)
#define FUGA_true    (self->Object->data.OBJECT->_true)
#define FUGA_false   (self->Object->data.OBJECT->_false)
#define FUGA_nil     (self->Object->data.OBJECT->nil)

/**
*** ## Constructors and Destructors
*** ### Fuga_new
***
*** `Fuga_new` allocates and initializes a new Fuga environment. At the
*** moment there are no parameters, but there may end up being some soonish.
*** 
*** - Params: (none)
*** - Returns: The base Fuga object, `Object`.
**/
Fuga* Fuga_new();

/**
*** ### Fuga_free
***
*** `Fuga_free` deallocates the Fuga environment. This is useful when, e.g.,
*** you're done with your program, or a little Fuga "session" is done.
***
*** - Params:
***     - `Fuga* self`: any object in the Fuga environment.
*** - Returns: void.
**/
void Fuga_free(Fuga* self);

/**
*** ## Prototyping
*** ### Fuga_clone
***
*** `Fuga_clone` creates an object, using the old object as a prototype.
*** The new object will invoke the prototype through delegation, so changes
*** to the prototype can have an effect on the new object. This is often
*** desirable.
***
*** - Params:
***     - `Fuga* proto`: the new object's prototype.
*** - Returns: the new object
**/
Fuga* Fuga_clone(Fuga* proto);

/**
*** ## Properties
*** ### Fuga_length
***
*** Return the number of canonical slots in self. 
***
*** - Params:
***     - `Fuga* self`
*** - Returns: `
**/

/**
*** ### Fuga_type
***
*** Determine the primitive type of the object.
***
*** - Params:
***     - `Fuga* obj`
*** - Returns: the primitive type of `obj`
**/
#define Fuga_type(obj)  (((obj)->type) & ~FUGA_ERROR)

#define Fuga_data(obj)  ((obj)->data)

/**
*** ### Fuga_error
***
*** Determine whether an error was raised.
***
*** - Params:
***     - `Fuga* obj`
*** - Returns: true if `obj` is a raised error, false otherwise.
**/
#define Fuga_isError(obj) (((obj)->type) & FUGA_ERROR)
#define FUGA_SET(obj, expr) obj = expr; if(Fuga_isError(obj)) return obj
#define FUGA_DECL(obj, expr) Fuga* FUGA_SET(obj, expr)

/**
*** ### Fuga_is
***
*** Determine whether two objects are the same object.
**/
Fuga* Fuga_is(Fuga* self, Fuga* other);
#define Fuga_isTrue(self)  (Fuga_is(self, FUGA_true)  == FUGA_true)
#define Fuga_isFalse(self) (Fuga_is(self, FUGA_false) == FUGA_true)

/**
*** ### Fuga_isa
***
*** Determine whether self's parents include an object.
**/
Fuga* Fuga_isa(Fuga* self, Fuga* code);

/**
*** ## Primitives
**/
Fuga* Fuga_bool(Fuga* self, bool value);
Fuga* Fuga_int (Fuga* self, FugaIndex index);
Fuga* Fuga_real(Fuga* self, double);
Fuga* Fuga_string(Fuga* self, const char*);
Fuga* Fuga_symbol(Fuga* self, const char*);

#define FUGA_BOOL(x)    (Fuga_bool(self, (x)))
#define FUGA_INT(x)     (Fuga_int(self, (x)))
#define FUGA_REAL(x)    (Fuga_real(self, (x)))
#define FUGA_STRING(x)     (Fuga_string(self, (x)))
#define FUGA_SYMBOL(x)     (Fuga_symbol(self, (x)))

Fuga* Fuga_msg (Fuga* self);
Fuga* Fuga_msg1(Fuga* self, Fuga* arg0);
Fuga* Fuga_msg2(Fuga* self, Fuga* arg0, Fuga* arg1);
Fuga* Fuga_msg3(Fuga* self, Fuga* arg0, Fuga* arg1, Fuga* arg2);
Fuga* Fuga_msg4(Fuga* self, Fuga* arg0, Fuga* arg1, Fuga* arg2, Fuga* arg3);

#define FUGA_MSG(x)          (Fuga_msg(FUGA_SYMBOL((x))))
#define FUGA_MSG1(x,a)       (Fuga_msg1(FUGA_SYMBOL(x), (a)))
#define FUGA_MSG2(x,a,b)     (Fuga_msg2(FUGA_SYMBOL(x), (a), (b)))
#define FUGA_MSG3(x,a,b,c)   (Fuga_msg3(FUGA_SYMBOL(x), (a), (b), (c)))
#define FUGA_MSG4(x,a,b,c,d) (Fuga_msg4(FUGA_SYMBOL(x), (a), (b), (c), (d)))

/**
*** ### Fuga_raise
***
*** Raise an error.
**/
Fuga* Fuga_raise(Fuga*);

#define FUGA_RAISE(f) return Fuga_raise(f)

/**
*** ## Slot Manipulation
*** ### Fuga_has
***
*** Determine whether a slot exists inside an object, with the given
*** name or index. If there is such a slot, `Fuga_get` is guaranteed to
*** succeed.
***
*** - Params:
***     - `Fuga* self`: object to look in
***     - `name` or `index`: name or index of the slot to look for 
*** - Returns: true if there is such a slot, false otherwise.
**/

bool  Fuga_rawHasByIndex  (Fuga* self, FugaIndex index);
bool  Fuga_rawHasByString (Fuga* self, const char* name);
bool  Fuga_rawHasBySymbol (Fuga* self, Fuga* name);
Fuga* Fuga_rawHas         (Fuga* self, Fuga* name);

bool  Fuga_hasByIndex     (Fuga* self, FugaIndex index);
bool  Fuga_hasByString    (Fuga* self, const char* name);
bool  Fuga_hasBySymbol    (Fuga* self, Fuga* name);
Fuga* Fuga_has            (Fuga* self, Fuga* name);

/**
*** ### Fuga_get
*** 
*** Return the value associated with a given name in a given object. If
*** there is no such slot, raise a SlotError.
***
*** - Params:
***     - `Fuga* self`: object to look in
***     - `name` or `index`: name or index of the slot to look for
*** - Returns: the value of the slot, or SlotError.
**/

Fuga* Fuga_rawGetByIndex  (Fuga* self, FugaIndex index);
Fuga* Fuga_rawGetByString (Fuga* self, const char* name);
Fuga* Fuga_rawGetBySymbol (Fuga* self, Fuga* name);
Fuga* Fuga_rawGet         (Fuga* self, Fuga* name);

Fuga* Fuga_getByIndex     (Fuga* self, FugaIndex index);
Fuga* Fuga_getByString    (Fuga* self, const char* name);
Fuga* Fuga_getBySymbol    (Fuga* self, Fuga* name);
Fuga* Fuga_get            (Fuga* self, Fuga* name);

/**
*** ### Fuga_set
***
*** Associate a value with a name in an object. If there is already such a
*** slot, or the index is too large, raise a SlotError.
***
*** - Params:
***     - `Fuga* self`: object to modify
***     - `name` or `index`: name of slot to add. If NULL, will insert
***     value in the first available index.
***     - `value`: the value of the slot to add.
*** - Returns: NULL on success, SlotError on failure.
**/

void  Fuga_setByIndex   (Fuga* self, FugaIndex index,  Fuga* value);
void  Fuga_setByString  (Fuga* self, const char* name, Fuga* value);
void  Fuga_setBySymbol  (Fuga* self, Fuga* name,       Fuga* value);
Fuga* Fuga_set          (Fuga* self, Fuga* name,       Fuga* value);

/**
*** ## Thunks
*** ### Fuga_thunk
***
*** Avoids evaluating code until it's necessary.
**/

Fuga* Fuga_thunk(Fuga* self, Fuga* receiver, Fuga* scope);

/**
*** ### Fuga_need
***
*** Force evaluation of a thunk. You only need to call this if you need
*** to access any of the self's data directly, or if you want to force
*** evaluation. Return self if self evaluates succesfully. If self raises
*** an error, return the error and reset the thunk.
**/
Fuga* Fuga_need(Fuga* self);
#define FUGA_NEED(self) FUGA_SET(self, Fuga_need(self))

/**
*** ### Fuga_slots
***
*** Return the slots associated with the object or thunk. If self is a
*** thunk, does not evaluate the slots -- returns the slots as thunks.
**/
Fuga* Fuga_slots(Fuga* self);

/**
*** ## Evaluation
*** ### Fuga_eval
***
*** Evaluate code. This does not generate thunks -- it generates evaluated
*** code. Usually, scope and receiver will be the same. If scope is NULL,
*** receiver is used. Return the evaluated object.
**/

Fuga* Fuga_eval(Fuga* self, Fuga* receiver, Fuga* scope);

/**
*** ### Fuga_evalSlots
***
*** Evaluate code according to its slots, not according to its type.
**/

#endif

