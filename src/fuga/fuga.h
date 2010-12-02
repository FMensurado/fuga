#ifndef FUGA_H
#define FUGA_H
/** 
*** # Fuga, a homoiconic object-oriented programming language.
**/

#include <stdint.h>
#include <stdbool.h>

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
***         - `uint32_t type`: refers to the type of the `data` field. It uses
***         predefined values, such as `FUGA_TYPE_INT` or `FUGA_TYPE_NONE`.
***         - `char data[]`: any data that can't be captured by the slots
***         themselves. Used for primitves such as numbers, strings, built 
***         in functions.
***
*** - See also: `FUGA_TYPE`, `FugaSlot`, `Fuga_new`, `Fuga_clone`
**/
typedef struct _FugaSlots FugaSlots;
typedef struct _Fuga Fuga;

struct _Fuga {
    Fuga* Object;
    Fuga* proto;
    FugaSlots* slots;
    
    // primitive data
    uint32_t type;
    uint32_t size;
    char data[];
};

/**
*** ### FugaSlot
***
*** The `FugaSlot` struct represents a name - value pair.
*** 
*** - Fields:
***     - `name` is the name associated with this slot. name` must be a
***     Symbol, or NULL.
***     - `value` is the value of this slot.
**/
typedef struct _FugaSlot FugaSlot;
struct _FugaSlot {
   Fuga* name;
   Fuga* value;
};

/**
*** ### FugaSlots
*** 
*** Holds an object's slots.
***
*** - Fields:
***     - `uint32_t length`: number of slots 
***     - `uint32_t capacity`: number of slots this can contain before needing
***     to grow
***     - `FugaSlot slot`: individual slots
**/
struct _FugaSlots {
    uint32_t length;
    uint32_t capacity;
    void* _pvt;
    FugaSlot slot[];
};


/**
*** ### FUGA_TYPE 
***
*** This enumeration details the various kinds of Fuga primitives. It is
*** important to notice that you can have non-standard types -- you must
*** use the flag `FUGA_FLAG_USER` for non-standard types.
***
*** User-defined primitive types / library primitive types have the
*** unfortunate trouble of not having a taxonomy, so the onus of d
***
*** - Values:
***     - `FUGA_TYPE_NONE`: for objects that aren't primitive
***     - `FUGA_TYPE_OBJECT`: for the root object (Object)
***     - `FUGA_TYPE_VOID`: for the `void` object (indicating a lack of value)
***     - `FUGA_TYPE_NULL`: for the `null` object (indicating no value)
***     - `FUGA_TYPE_INT`: for (short) primitive ints
***     - `FUGA_TYPE_LONG`: for long primitive ints
***     - `FUGA_TYPE_REAL`: for real numbers
***     - `FUGA_TYPE_STRING`: for primitive strings
***     - `FUGA_TYPE_SYMBOL`: for primitive symbols (which are always unique)
***     - `FUGA_TYPE_METHOD`: for primitive methods
*** - Flags:
***     - `FUGA_FLAG_USER`: user-defined / library primitives must contain
***     this flag.
***     - `FUGA_FLAG_ERROR`: raised types.
**/

#define FUGA_TYPE_NONE   0 // not a primitve
    
// primitive types
#define FUGA_TYPE_OBJECT 1  // type of Object (the base object)
#define FUGA_TYPE_NULL   2  // type of null   (object that denotes no value)
#define FUGA_TYPE_INT    3  // for primitive ints
#define FUGA_TYPE_LONG   4  // for primitive ints
#define FUGA_TYPE_REAL   5  // for primitive reals
#define FUGA_TYPE_STRING 6  // for primitive strings
#define FUGA_TYPE_SYMBOL 7  // for primitive symbols
#define FUGA_TYPE_METHOD 8  // for primitive methods
    
// flags
#define FUGA_FLAG_USER    0x40000000
#define FUGA_FLAG_ERROR   0x80000000

/**
*** ## Constructors and Destructors
*** ### Fuga_new
***
*** `Fuga_new` allocates and initializes a new Fuga environment. At the moment
*** there are no parameters, but there may end up being some soonish.
*** 
*** - Parameters: (none)
*** - Returns: The base Fuga object, `Object`.
*** - See also:`Fuga_free` and `Fuga_clone`.
**/
Fuga* Fuga_new();

/**
*** ### Fuga_free
***
*** `Fuga_free` deallocates the Fuga environment. This is useful when, e.g.,
*** you're done with your program, or a little Fuga "session" is done.
***
*** - Parameters:
***     - `Fuga* self`: any object in the Fuga environment.
*** - Returns: void.
*** - See also: `Fuga_new`.
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
*** - Parameters:
***     - `Fuga* proto`: the new object's prototype.
*** - Returns: the new object
*** - See also: `Fuga_alloc`
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
*** - Parameters:
***     - `Fuga* proto`: the new Object's prototype.
*** - Returns: the new object
*** - See also: `Fuga_clone`
**/
Fuga* Fuga_alloc(Fuga* proto, uint32_t type, uint32_t size);

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
*** - See also: `Fuga_get`
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
*** - See also: `Fuga_has`, `Fuga_set`.
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
*** - See also: `Fuga_get`
**/

Fuga* Fuga_set(Fuga* self, Fuga* name, Fuga* value);
Fuga* Fuga_seti(Fuga* self, uint32_t index, Fuga* value);
Fuga* Fuga_sets(Fuga* self, const char* name, Fuga* value);

#endif
