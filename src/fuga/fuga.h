#ifndef FUGA_H
#define FUGA_H
/** 
*** # Fuga, a homoiconic object-oriented programming language.
**/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
*** ### FugaData
***
*** Represents the various types of primitive data. This is meant
*** to be a 64bit (8bytes) union. It is useful to remember that size
*** 
**/
typedef union FugaData {
    struct FugaObject OBJECT;
    int64_t   INT;
    uint64_t* LONG;
    double    REAL;
    char*     SYMBOL;
    char*     STRING;
    bool      BOOL;
    void*     data;
} FugaData;

/**
*** ### FUGA_TYPE 
***
*** This is an 8bit value that tells whether 
***
*** - Values:
***     - `FUGA_TYPE_NONE`: for objects that aren't primitive
***     - `FUGA_TYPE_INT`: for small primitive ints
***     - `FUGA_TYPE_LONG`: for big primitive ints
***     - `FUGA_TYPE_REAL`: for floating point numbers
***     - `FUGA_TYPE_STRING`: for primitive strings
***     - `FUGA_TYPE_SYMBOL`: for primitive symbols
***     - `FUGA_TYPE_METHOD`: for primitive methods
***     - `FUGA_TYPE_OBJECT`: for the root object (Object)
***     - `FUGA_TYPE_NIL`: for the `nil` object (indicating a lack of value)
***     - `FUGA_TYPE_TRUE`: for the `true` object
***     - `FUGA_TYPE_FALSE`: for the `false` object
*** - Flags:
***     - `FUGA_FLAG_ERROR`: a raised error
**/

typedef uint8_t FugaType

#define FUGA_TYPE_NONE   0x00 // not a primitive
#define FUGA_TYPE_INT    0x01 // for smallprimitive ints
#define FUGA_TYPE_LONG   0x02 // for long primitive ints
#define FUGA_TYPE_REAL   0x03 // for primitive reals
#define FUGA_TYPE_STRING 0x04 // for primitive strings
#define FUGA_TYPE_SYMBOL 0x05 // for primitive symbols
#define FUGA_TYPE_METHOD 0x06 // for primitive methods

// primitive singletons
#define FUGA_TYPE_OBJECT 0x08  // type of Object (the base object)
#define FUGA_TYPE_NIL    0x09  // type of nil
#define FUGA_TYPE_TRUE   0x0A  // type of true
#define FUGA_TYPE_FALSE  0x0B  // type of false

// flags
#define FUGA_FLAG_ERROR  0x80

/**
*** ### FugaID
***
*** Unique identifier for each 
**/
typedef uint32_t FugaID;

#define FUGA_ID_LAZY    ((uint32_t)0)
#define FUGA_ID_NEEDED  ((uint32_t)0xFFFFFFFF)
#define FUGA_ID_OBJECT  ((uint32_t)1)

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

#ifndef FUGA_FUGA_TYPEDEF
#define FUGA_FUGA_TYPEDEF
typedef struct _Fuga Fuga;
#endif
struct _Fuga {
    Fuga* Object;
    Fuga* proto;
    struct FugaSlots* slots;
    FugaID id;

    // primitive type
    FugaType _type;
    uint32_t size:24;
    FugaData data;
};

/**
*** ### FugaObject
**/
typedef struct FugaObject {
    FugaGC* gc;
    FugaID id;
    Fuga* Prelude;
    Fuga* Number;
    Fuga* Int;
    Fuga* Real;
    Fuga* String;
    Fuga* Symbol;
} FugaObject;

#define FUGA_Object  (self->Object)
#define FUGA_Prelude (self->Object->data.OBJECT->Prelude)
#define FUGA_Number  (self->Object->data.OBJECT->Number)
#define FUGA_Int     (self->Object->data.OBJECT->Int)
#define FUGA_Real    (self->Object->data.OBJECT->Real)
#define FUGA_String  (self->Object->data.OBJECT->String)
#define FUGA_Symbol  (self->Object->data.OBJECT->Symbol)

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
*** ### Fuga_alloc
***
*** `Fuga_alloc` creates an object just like `Fuga_clone`, but it adds
*** a tiny bit of spice. Essentially, `Fuga_alloc` allows you to define
*** your own primitive data types in Fuga. In fact, this is how Ints and
*** Strings are allocated.
***
*** The way it works is that you give `Fuga_alloc` a prototype, a "type",
*** and a size, and it returns an object that was cloned from that prototype,
*** with the given type (to mark the primitive data type), and with enough
*** space in the `data` field to store something of the given size.
*** When creating a primitive, call Fuga_alloc, and then use the new object's
*** `data` field to store whatever you want.
***
*** - Params:
***     - `Fuga* proto`: the new Object's prototype.
*** - Returns: the new object
**/
Fuga* Fuga_alloc(Fuga* proto, uint32_t type, uint32_t size);
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
#define Fuga_type(obj)  (((obj)->_type) & ~FUGA_FLAG_ERROR)

/**
*** ### Fuga_error
***
*** Determine whether an error was raised.
***
*** - Params:
***     - `Fuga* obj`
*** - Returns: true if `obj` is a raised error, false otherwise.
**/
#define Fuga_error(obj) (((obj)->_type) & FUGA_FLAG_ERROR)


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
bool Fuga_has(Fuga* self, Fuga* name);
bool Fuga_hasi(Fuga* self, uint32_t index);
bool Fuga_hass(Fuga* self, const char* name);

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
Fuga* Fuga_get(Fuga* self, Fuga* name);
Fuga* Fuga_geti(Fuga* self, uint32_t index);
Fuga* Fuga_gets(Fuga* self, const char* name);

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

Fuga* Fuga_set(Fuga* self, Fuga* name, Fuga* value);
Fuga* Fuga_seti(Fuga* self, uint32_t index, Fuga* value);
Fuga* Fuga_sets(Fuga* self, const char* name, Fuga* value);

#endif
