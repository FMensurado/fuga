/**
 * \file fuga.h
 * \brief Main Fuga header file.
 */
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

/**
 * \brief Fuga environmental data.
 *
 * FugaRoot holds objects and data of global importance, such as
 * garbage collection information, the global symbol table, and
 * various global objects. Whenever "self" is a fuga object, FugaRoot
 * can be accessed using the FUGA macro.
 *
 * \see FUGA
 */
struct FugaRoot {
    // GC info
    FugaGCList roots; /**< Objects that aren't meant to be freed. */
    FugaGCList black; /**< Objects that have been marked, and have marked
                           all of their children, in the current pass. */
    FugaGCList grey;  /**< Objects that have been marked, but have not
                           yet marked their children, in the current
                           pass. */
    FugaGCList white; /**< Objects that have not been marked in this pass
                           These may be freed at the end of the current
                           pass if they are still not marked. */

    // symbols
    FugaSymbols* symbols; /**< Global symbol table. Ensures that all
                               symbols that are equal have the same
                               identity. */

    // basic objects
    void* Object;   /**< Base object for all fuga objects. */
    void* Prelude;  /**< Base object for all fuga modules. */
    void* Number;   /**< Base object for number types. */
    void* Int;      /**< Base integer object. */
    void* String;   /**< Base string object. */
    void* Symbol;   /**< Base symbol object. */
    void* Msg;      /**< Base message object. */
    void* Method;   /**< Base method/function object. */
    void* Expr;     /**< Base expression object. An expression is a
                         sequence of messages (with an expression 'root'
                         that may or may not be a message). */
    void* nil;      /**< Value that represents no value. */
    void* Bool;     /**< Base boolean value object. */
    void* True;     /**< Boolean object that corresponds to "true". Known
                         as "true" in Fuga code. */
    void* False;    /**< Boolean object that corresponds to "false".
                         Known as "false" in Fuga code. */
    void* Thunk;    /**< Base first-class thunk object. */
    void* Path;     /**< File path manipulation object. */

    // Exceptions
    void* Exception;        /**< Base Exception object. */
    void* SlotError;        /**< Exception raised when various slot
                                 methods fail. Typically this happens
                                 when a name does not exist. */
    void* IOError;          /**< Exception raised when an IO operation
                                 fails in some way. */
    void* TypeError;        /**< Exception raised when an value is of an
                                 unexpected or unacceptable type. */
    void* ValueError;       /**< Exception raised when a value is of the
                                 right type, but not in an acceptable
                                 range. */
    void* SyntaxError;      /**< Exception raised when code is badly
                                 formed. */
    void* SyntaxUnfinished; /**< Exception raised when code is
                                 incomplete. For example, a parenthesis
                                 was opened but it was not closed.
                                 SyntaxUnfinished inherits from
                                 SyntaxError. */
    void* MatchError;       /**< Exception raised when a singular
                                 pattern matching operation fails. That
                                 is, when a "_match" method fails to
                                 match. */
};

/**
 * \brief Primitive type identifier.
 *
 * FugaType is used to identify the primitive type of a primitive Fuga
 * object, such as an integer, a string, a symbol, etc. The data
 * contained in the FugaType struct is secondary -- the important part
 * of this struct is it's address in memory. This is what is used to
 * distinguish a primitive integer from a primitive string or from a
 * vanilla object.
 * 
 * To define your own primitive type in C, you must declare, at a global
 * scope, an instance of FugaType (and you may declare it to be const).
 * You must then use Fuga_type_ to set an object's primitive type, and
 * you can use Fuga_type and Fuga_hasType_ to get or test an object's
 * primitive type.
 *
 * \see Fuga_type
 * \see Fuga_type_
 * \see Fuga_hasType_
 */
struct FugaType {
    const char* name; /**< Name of the primitive type. */
};

/**
 * \brief Garbage collection data for a single object.
 */
struct FugaGCInfo {
    FugaGCList  list;            /**< Next and prev elements of the
                                      current list. The root of the list
                                      is in FugaRoot. */
    void        (*mark) (void*); /**< Mark function. Mark the object's
                                      GCed dependencies. */
    void        (*free) (void*); /**< Free function. Free the object's
                                      non-GCed dependencies. */
    unsigned    pass;            /**< Last garbage collection pass in
                                      which the object was marked. */
    bool        root;            /**< Is this object in FUGA->roots? */
};

/**
 * \brief Data every fuga object must have.
 * 
 * All fuga objects have a FugaHeader. If "ptr" is a pointer to 
 * a fuga object, its FugaHeader is located right before "ptr". So,
 * its FugaHeader is located at "((FugaHeader*)ptr-1)". Rather than
 * relying on this fact, use the FUGA_HEADER macro, if you need to
 * access header data. Likewise, if "hdr" is a pointer to FugaHeader,
 * the actual data is located at "((FugaHeader*)hdr+1)". Use the
 * FUGA_DATA macro, rather than relying on that fact.
 *
 * \see FUGA_HEADER
 * \see FUGA_DATA
 */
struct FugaHeader {
    FugaGCInfo       gc;    /**< Garbage collection data. */
    FugaRoot*        root;  /**< Relevant Fuga environment. */
    const FugaType*  type;  /**< Primitive type of object. */
    FugaSlots*       slots; /**< Slots in object. */
    void*            proto; /**< Prototype of object. */
};

/**
 * \brief Get the FugaHeader of a fuga object.
 * \param self A pointer to a fuga object.
 * \return self's FugaHeader.
 * \see FugaHeader
 */
#define FUGA_HEADER(self)   (((FugaHeader*)(self))-1)

/**
 * \brief Get the object pointer, given a FugaHeader.
 * \param self A pointer to a FugaHeader
 * \return self's FugaHeader
 * \see FugaHeader
 */
#define FUGA_DATA(self)     ((void*)(((FugaHeader*)(self))+1))

/**
 * \brief The global FugaRoot/environment.
 * \warning "self" must be a pointer to a fuga object.
 * \see FugaRoot
 */
#define FUGA                (FUGA_HEADER(self)->root)

/**
 * \brief Construct a Fuga environment.
 *
 * This includes initializing builtin objects such as Object, Prelude,
 * Int, Symbol, Path, Loader, etc. See FugaRoot for such a list of global
 * objects.
 *
 * \return The Prelude object.
 * \see FugaRoot.
 */
void* Fuga_init(void);

/**
 * \brief Destruct a Fuga environment.
 *
 * Frees all objects in the enironment, by calling all the "free"
 * functions in the FugaGCInfo structure, and calling stdlib's free.
 * 
 * \param self Any object in the environment to be destroyed.
 * \see FugaGCInfo
 * \see Fuga_onMark_
 */
void Fuga_quit(void* self);

/**
*** ## Garbage Collection
*** ### Fuga_onMark_
*** 
*** Set the "onMark" function for a primitive. The "onMark" function
*** is called during garbage collection runs. The "onMark" function
*** must call `Fuga_mark_` for all references to other Fuga objects
*** within the primitive data. This is important because Fuga does not
*** understand primitive data, and may accidentally end up freeing
*** necessary data.
***
*** - Params:
***     - `void* self`: the primitive.
***     - `void (*markFn)(void*)`: the primitive's "onMark" function.
*** - Return: void.
**/
void Fuga_onMark_(void* self, void (*markFn)(void*));

/**
*** ### Fuga_onFree_
***
*** Set the "onFree" function for a primitive. The "onFree" function
*** is called when the object is being deallocated. The "onFree"
*** function must not attempt to deallocate the object itself -- it
*** must only free non-garbage-collected primitive data.
*** 
*** - Params:
***     - `void* self`: the primitive.
***     - `void (*freeFn)(void*)`: the primitive's "onFree" function.
*** - Return: void.
**/
void Fuga_onFree_(void* self, void (*freeFn)(void*));

/**
*** ### Fuga_mark_
*** 
*** Declare that `child` is referenced by `self` in primitive data.
*** This function should only be called from "onMark" function of `self`
*** (see `Fuga_onMark_`).
***
*** - Params:
***     - `void* self`: the parent.
***     - `void* child`: the child that's referenced by the parent in
***     primitive data.
*** - Return: void.
**/
void Fuga_mark_(void* self, void* child);

/**
*** ### Fuga_collect
***
*** Perform a garbage collection step. In other words, free any objects
*** that are no longer referenced anywhere. Be careful, though: you must
*** use `Fuga_root` (and `Fuga_unroot`) to control which objects are to
*** be preserved regardless of outside references.
***
*** - Params:
***     - `void* self`: any object in the Fuga environment.
*** - Return: void.
**/
void Fuga_collect(void* self);

/**
*** ### Fuga_root
***
*** Declare an object to be a "root", for the purpose of garbage
*** collection. In other words, the object won't be collected, even
*** if there are no relevant references to it. Use `Fuga_unroot` to
*** reverse this. If the object is already a "root", it will be
*** ignored.
***
*** - Params:
***     - `void* self`: the object to root.
*** - Return: void.
**/
void Fuga_root(void* self);

/**
*** ### Fuga_unroot
***
*** Declare an object to no longer be a "root". In other words, the
*** object will once again be available for garbage collection.
***
*** - Params:
***     - `void* self`: the object to unroot
*** - Return: void
**/
void Fuga_unroot(void* self);

/**
*** ## Prototyping
*** ### Fuga_clone
***
*** Create an empty object with a given prototype.
***
*** - Params:
***     - `void* proto`: the new object's prototype.
*** - Return: the new object.
**/
void* Fuga_clone(void* proto);

/**
*** ### Fuga_clone_
***
*** Create a primitive with a given prototype, and `size` bytes of
*** primitive data.
***
*** - Params:
***     - `void* proto`: the new object's prototype.
***     - `size_t size`: the number of bytes of primitive data to
***     allocate.
*** - Return: the primitive. Doubles as a pointer to the primitive data.
**/
void* Fuga_clone_(void* proto, size_t size);

/**
*** ## Primitives
*** ### Fuga_type
***
*** Return the primitive type of an object.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: the primitive type of `self`. Can be NULL.
**/
const FugaType* Fuga_type(void* self);

/**
*** ### Fuga_type_
***
*** Set the primitive type of an object.
***
*** - Params:
***     - `void* self`: the object.
***     - `const FugaType* type`: the new primitive type.
*** - Return: void.
**/
void            Fuga_type_  (void* self, const FugaType* type);

/**
*** ### Fuga_hasType_
***
*** Determine whether an object has a given primitive type.
***
*** - Params:
***     - `void* self`: the object.
***     - `const FugaType* type`: the primitive type to check for.
*** - Return: true iff the object has the given primitive type.
**/
bool Fuga_hasType_ (void* self, const FugaType* type);


/**
*** ### Fuga_isLazy
***
*** Determine whether an object is or was once a lazy value. Note that
*** the value may have already been evaluated. Thus, this isn't a good
*** way to check if a value is ready. This function is equivalent to
*** determining whether a value has the lazy primitive type,
*** `FugaLazy_type`.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is or was once a lazy value.
**/
bool Fuga_isLazy(void* self);
extern const FugaType FugaLazy_type;

/**
*** ### Fuga_isInt
*** 
*** Determine whether an object is an integer primitive.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is an integer primitive.
**/
bool Fuga_isInt    (void* self);
extern const FugaType FugaInt_type;

/**
*** ### Fuga_isString
*** 
*** Determine whether an object is a string primitive.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is a string primitive.
**/
bool Fuga_isString (void* self);
extern const FugaType FugaString_type;

/**
*** ### Fuga_isSymbol
*** 
*** Determine whether an object is a symbol primitive.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is a symbol primitive.
**/
bool Fuga_isSymbol (void* self);
extern const FugaType FugaSymbol_type;

/**
*** ### Fuga_isMsg
*** 
*** Determine whether an object is a message primitive.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is a message primitive.
**/
bool Fuga_isMsg    (void* self);
extern const FugaType FugaMsg_type;

/**
*** ### Fuga_isMethod
*** 
*** Determine whether an object is a method primitive.
***
*** - Params:
***     - `void* self`: the object.
*** - Return: true iff the object is a method primitive.
**/
bool Fuga_isMethod (void* self);
extern const FugaType FugaMethod_type;

#include "lazy.h"
#include "int.h"
#include "string.h"
#include "symbol.h"
#include "string.h"
#include "method.h"
#include "msg.h"
#include "bool.h"

/**
*** ## Object Identity
*** ### Fuga_is_
***
*** Determine whether two objects have the same identity. This function
*** evaluates lazy values automatically, in order to reach the verdict.
***
*** - Params:
***     - `void* self`: one object.
***     - `void* other`: the other object.
*** - Return: true iff the two objects have the same identity.
**/
bool Fuga_is_(void* self, void* other);

/**
*** ### Fuga_isa_
***
*** Determine whether one object inherits from another object. In other
*** words, determine whether `other` is in `self`'s prototype chain.
***
*** - Params:
***     - `void* self`: the possible descendent.
***     - `void* other`: the possible ancestor.
*** - Return: true iff `self` inherits from `other`.
**/
bool Fuga_isa_(void* self, void* other);

/**
*** ### Fuga_isTrue
***
*** Determine whether an object is the Fuga boolean "true".
***
*** - Params:
***     - `void* self`: the object to test.
*** - Return: true iff `self` is `FUGA_BOOL(true)`.
**/
bool Fuga_isTrue(void* self);

/**
*** ### Fuga_isFalse
***
*** Determine whether an object is the Fuga boolean "false".
***
*** - Params:
***     - `void* self`: the object to test.
*** - Return: `bool`, true iff `self` is `FUGA_BOOL(false)`.
**/
bool Fuga_isFalse(void* self);

/**
*** ### Fuga_isNil
***
*** Determine whether an object is the Fuga "nil".
***
*** - Params:
***     - `void* self`: the object to test.
*** - Return: `bool`, true iff `self` is `FUGA->nil`.
**/
bool Fuga_isNil(void* self);

/**
*** ### Fuga_isExpr
***
*** Determine whether an object is an expression. In other words,
*** determine whether an object inherits from `FUGA->Expr`.
***
*** - Params:
***     - `void* self`: the object to test.
*** - Return: `bool`, true iff `self` inherits from `FUGA->Expr`.
**/
bool Fuga_isExpr(void* self);

/**
*** ## Exception Handling
***
*** To raise an exception, we flag the lowest bit of a pointer.
*** Therefore, it's important that our pointers are always aligned
*** on at least a 2-byte boundary. Most platforms prefer this,
*** but it's something to keep in mind.
***
*** Return values represent exceptions. So it is important to always
*** check for exceptions. This is typically done using `FUGA_CHECK` or
*** `FUGA_NEED`. If you want to raise an exception, you typically want
*** to use `FUGA_RAISE`. If you want to catch an exception, you
*** typicall want to use `FUGA_TRY`, `FUGA_CATCH`, and `FUGA_RERAISE`.
***
*** ### Fuga_isRaised
***
*** Determine whether the value represents a raised exception.
***
*** - Params:
***     - `void* self`: the value.
*** - Return: `bool`, true iff the value is raised.
**/
bool Fuga_isRaised(void* self);

/**
*** ### Fuga_raise
***
*** Raise an exception.
***
*** - Params:
***     - `void* self`: the exception value.
*** - Return: the exception, raised.
**/
void* Fuga_raise(void* self);

/**
*** ### Fuga_catch
***
*** Test for and catch a raised exception.
***
*** - Params:
***     - `void* self`: the possible exception.
*** - Return: the expection value, unraised, if `self` is a raised 
*** exception. Otherwise returns `NULL`.
**/
void* Fuga_catch(void* self);

/**
*** ### FUGA_RAISE
***
*** Raise an exception, supplying a prototype and a message.
***
*** - Params:
***     - `void* type`: the exception prototype.
***     - `const char* msg`: the error message.
*** - Return: no return -- this macro exits out of the
*** calling procedure, returning the raised exception.
**/
#define FUGA_RAISE(type, msg)                                           \
    do {                                                                \
        void* error##__LINE__ = Fuga_clone(type);                       \
        Fuga_setS(error##__LINE__, "msg", FUGA_STRING(msg));            \
        return Fuga_raise(error##__LINE__);                             \
    } while(0)

/**
*** ### FUGA_CHECK
***
*** Check if the value is a raised exception. If it is, exit the calling
*** procedure, returning the raised exception.
***
*** - Params:
***     - `void* value`: the value to check.
*** - Return: no return -- this macro exits out of the calling procedure,
*** returning `value`, if `value` is a raised exception.
**/
#define FUGA_CHECK(value)                                               \
    do {                                                                \
        void* error##__LINE__ = Fuga_catch(value);                      \
        if (error##__LINE__)                                            \
            return Fuga_raise(error##__LINE__);                         \
    } while(0)

/**
*** ### FUGA_TRY
***
*** Catch an exception selectively. Emulates try { catch } blocks in
*** exception-enabled languages. Here's an example:
***
***     FUGA_TRY(riskyFunction(...)) {
***         FUGA_CATCH(FUGA->TypeError) {
***             printf("A type error was raised!")
***             break;
***         }
***         FUGA_CATCH(FUGA->SyntaxError) {
***             return FUGA->nil;
***         }
***         FUGA_RERAISE;
***     }
***
*** As you can see, it's important to "break" or "return" from each catch
*** statement, in order to avoid trigerring `FUGA_RERAISE`.
***
*** - Params:
***     - `void* result`: the value to test.
*** - Return: no return -- this is a macro that checks for exceptions
*** and does different actions depending on the type of exception.
**/
#define FUGA_TRY(result)   \
    for (void *exception = Fuga_catch(result); exception; exception=NULL)

/**
*** ### FUGA_CATCH
***
*** See `FUGA_TRY`.
***
*** Catch an exception if it is of a certain type (i.e., if it inherits
*** from a given prototype). Don't forget to `break` or `return` from
*** within the `FUGA_CATCH` block.
***
*** - Params:
***     - `void* type`: the type to check for.
*** - Result: perform the following C block or C statement if the
*** exception inherits from `type`.
**/
#define FUGA_CATCH(type) if (Fuga_isa_(exception, type))

/**
*** ### FUGA_RERAISE
***
*** See `FUGA_TRY`.
***
*** Reraise the exception, after failing to match its type with an
*** exception handler.
**/
#define FUGA_RERAISE     return Fuga_raise(exception)

/**
*** ### FUGA_IF
***
*** Fuga-specific, exception-safe if. It passes on any raised exceptions,
*** and it raises an exception if the tested values are not boolean.
***
*** This is a multi-line macro, so wrap it in curly braces if you need
*** a block.
***
*** - Params:
***     - `void* expr`: a Fuga boolean.
*** - Result: If `expr` is a fuga boolean, this acts like a C if
*** statement. If `expr` is a raised exception, this passes it on. If
*** `expr` is anything else, this raises an exception.
**/
#define FUGA_IF(expr)                                                   \
    void* value##__LINE__ = (expr);                                       \
    if (Fuga_isRaised(value##__LINE__))                                   \
        return value##__LINE__;                                           \
    if (!Fuga_isFalse(value##__LINE__) && !Fuga_isTrue(value##__LINE__))    \
        FUGA_RAISE(FUGA->TypeError, "expected boolean");                \
    if (Fuga_isTrue(value##__LINE__))

// Slot Manipulation

void* Fuga_proto    (void* self);
void* Fuga_slots    (void* self);
void* Fuga_dir      (void* self);

long Fuga_length     (void* self);
bool Fuga_hasLength_ (void* self, long length);

void* Fuga_hasS      (void* self, const char* name);
void* Fuga_hasRawS   (void* self, const char* name);
void* Fuga_hasDocS   (void* self, const char* name);
void* Fuga_getS      (void* self, const char* name);
void* Fuga_getRawS   (void* self, const char* name);
void* Fuga_getDocS   (void* self, const char* name);
void* Fuga_setS      (void* self, const char* name, void* value);
void* Fuga_setDocS   (void* self, const char* name, void* value);
void* Fuga_delS      (void* self, const char* name);

void* Fuga_has       (void* self, void* name);
void* Fuga_hasRaw    (void* self, void* name);
void* Fuga_hasName   (void* self, void* name);
void* Fuga_hasDoc    (void* self, void* name);
void* Fuga_get       (void* self, void* name);
void* Fuga_getRaw    (void* self, void* name);
void* Fuga_getName   (void* self, void* name);
void* Fuga_getDoc    (void* self, void* name);
void* Fuga_set       (void* self, void* name, void* value);
void* Fuga_setDoc    (void* self, void* name, void* value);
void* Fuga_modify    (void* self, void* name, void* value);
void* Fuga_del       (void* self, void* name);

void* Fuga_hasI      (void* self, long index);
void* Fuga_hasNameI  (void* self, long index);
void* Fuga_hasDocI   (void* self, long index);
void* Fuga_getI      (void* self, long index);
void* Fuga_getNameI  (void* self, long index);
void* Fuga_getDocI   (void* self, long index);
void* Fuga_setI      (void* self, long index, void* value);
void* Fuga_setDocI   (void* self, long index, void* value);
void* Fuga_delI      (void* self, long index);

void* Fuga_append_   (void* self, void* value);
void* Fuga_update_   (void* self, void* value);
void* Fuga_extend_   (void* self, void* value);
void* Fuga_copy      (void* self);

#define FUGA_FOR(i, slot, arg)                                      \
    FUGA_NEED(arg);                                                 \
    void* slot = Fuga_getI(arg, 0);                                 \
    for (long length = Fuga_length(arg),                            \
         i = 0;                                                     \
         i < length;                                                \
         i++,                                                       \
         slot = Fuga_getI(arg, i))

// Calling & Sending
void* Fuga_call (void* self, void* recv, void* args);
void* Fuga_send (void* self, void* msg, void* args);

// Evaluation
void* Fuga_eval         (void* self, void* recv, void* scope);
void* Fuga_evalSlots    (void* self, void* scope);
void* Fuga_evalIn       (void* self, void* scope);
void* Fuga_evalPrint    (void* self, void* scope);
void* Fuga_evalExpr     (void* self, void* recv, void* scope);
void* Fuga_evalModule   (void* self);
void* Fuga_evalModule_  (void* self, const char* filename);

// Misc
void* Fuga_load_ (void* self, const char* filename);

void* Fuga_str(void* self);
void* Fuga_strSlots(void* self);

void  Fuga_printException(void* self);
void* Fuga_print(void* self);

void* Fuga_match_(void* self, void* other);
void* FugaObject_match_(void* self, void* other);

#endif

