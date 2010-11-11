#ifndef FUGA_H
#define FUGA_H
/**
*** # Fuga, a homoiconic object-oriented programming language.
**/

#include "table.h"

/**
*** The `Fuga` struct represents an object in the world of Fuga.
*** 
*** Fields:
*** 
*** - `Fuga* Object`: Points to the root object.
***
*** - `Fuga* proto`: This object's prototype. If this is NULL, this object
*** must be Object, the root object.
***
*** - `FugaTable* slots`: contains a reference to this object's slots.
***
*** - `size_t type`: refers to the type of the `data` field. It uses
*** predefined values, FUGA_TYPE_..., and give room for user-defined types.
***
*** - `char data[]`: any data that can't be captured by the slots themselves.
*** Used for primitves such as numbers, strings, built in functions...
***
**/
typedef struct _Fuga Fuga;
struct _Fuga {
    Fuga* Object;
    Fuga* proto;
    FugaTable* slots
    size_t type;
    char data[];
};

/**
*** # Constructors and Destructors
***
*** `Fuga_new` allocates and initializes a new Fuga environment. At the moment
*** there are no parameters, but there may end up being some soonish.
*** 
*** See also, `Fuga_free` and `Fuga_clone`.
**/
Fuga* Fuga_new();

/**
*** `Fuga_free` deallocates the Fuga environment. This is useful when, e.g.,
*** you're done with your program, or a little Fuga "session" is done.
**/
void Fuga_free(Fuga* self);


#endif
