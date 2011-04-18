#ifndef FUGA_THUNK_H
#define FUGA_THUNK_H

#include "fuga.h"

/**
*** # FugaThunk
***
*** First-class thunks in Fuga. This is a convenient
**/
typedef struct FugaThunk FugaThunk;


/**
*** ### FugaThunk_init
***
*** Initialize the Thunk object.
**/
void FugaThunk_init(void* self);

/**
*** ## Constructors
*** ### FugaThunk_new
***
*** Construct a first-class thunk from an unevaluated lazy value.
**/
FugaThunk* FugaThunk_new(void* value);

/**
*** ### FugaThunk_new_
***
*** Cosntruct a first-class thunk from a code object and its lexical
*** scope.
**/
FugaThunk* FugaThunk_new_(void* code, void* scope);

/**
*** ### Fuga
***
*** Get the thunk's code.
**/
void* FugaThunk_code(FugaThunk* thunk);

/**
*** ### FugaThunk_scope
***
*** Get the thunk's scope.
**/
void* FugaThunk_scope(FugaThunk* thunk);

/**
*** ### FugaThunk_lazy
***
*** Revert the thunk back into a lazy value.
**/
void* FugaThunk_lazy(FugaThunk* thunk);

/**
*** ### FugaThunk_eval
***
*** Evaluate the thunk.
**/
void* FugaThunk_eval(FugaThunk* thunk);

/**
*** ### FugaThunk_eval
***
*** Evaluate the thunk in a new scope.
**/
void* FugaThunk_eval_(FugaThunk* thunk, void* scope);

/**
*** ### FugaThunk_evalMethod
***
*** Wraps the previous evals into a single method.
**/
void* FugaThunk_evalMethod(FugaThunk* thunk, void* args);

/**
*** ### FugaThunk_str
**/
FugaString* FugaThunk_str(FugaThunk* thunk);

#endif

