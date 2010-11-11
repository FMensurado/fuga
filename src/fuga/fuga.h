#ifndef FUGA_H
#define FUGA_H
/** 
*** # Fuga, a homoiconic object-oriented programming language.
**/

#include "table.h"

/** ### Fuga
***
*** The `Fuga` struct represents an object in the world of Fuga.
*** 
*** - Fields:
***     - `Fuga* Object`: Points to the root object.
***     - `Fuga* proto`: This object's prototype. If this is NULL, this
***     object must be Object, the root object.
***     - `FugaTable* slots`: contains a reference to this object's slots.
***     - `size_t type`: refers to the type of the `data` field. It uses
***     predefined values, FUGA_TYPE_..., and give room for user-defined
***     types.
***     - `char data[]`: any data that can't be captured by the slots
***     themselves. Used for primitves such as numbers, strings, built 
***     in functions.
*** - See also: `Fuga_new`, `Fuga_clone`, `Fuga_free`
**/
typedef struct _Fuga Fuga;
struct _Fuga {
    Fuga* Object;
    Fuga* proto;
    FugaTable* slots
    size_t type;
    char data[];
};

/** ## Constructors and Destructors
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

/** ### Fuga_free
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

/** ## Prototyping
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

/** ### Fuga_alloc
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
Fuga* Fuga_alloc(Fuga* proto, size_t type, size_t size);


#endif
