#include "fuga.h"
#include "slots.h"
#include "test.h"
#include "gc.h"



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
Fuga* Fuga_new() {
    FugaGC* gc = FugaGC_start();
    Fuga* self = FugaGC_alloc(gc, sizeof(Fuga),
                              _Fuga_freeFn, _Fuga_markFn);
    
    self->Object = self;
    self->proto  = NULL;
    self->slots  = FugaSlots_new(gc);
    self->id     = FUGA_ID_OBJECT;
    self->_type  = FUGA_TYPE_OBJECT;
    self->size   = sizeof(_FugaObject);
    
    self->data.OBJECT = malloc(sizeof);
    self->data.OBJECT->gc      = gc;
    self->data.OBJECT->id      = FUGA_ID_OBJECT;
    self->data.OBJECT->symbols = FugaSymbols_new(gc);
    
    FUGA_Prelude = Fuga_clone(self);
    FUGA_Number  = Fuga_clone(self);
    FUGA_Int     = Fuga_clone(FUGA_Number);
    FUGA_Real    = Fuga_clone(FUGA_Number);
    FUGA_String  = Fuga_clone(self);
    FUGA_Symbol  = Fuga_clone(self);
}

void _Fuga_freeFn() {
    pass;
}

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
void Fuga_free(Fuga* self) {
    ALWAYS(self);
    self = self->Object;
    self->data.OBJECT->gc = gc;
}

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
Fuga* Fuga_alloc(Fuga* proto, FugaType type, uint32_t size);
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

