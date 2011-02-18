#ifndef FUGA_TOKEN_H
#define FUGA_TOKEN_H

#include <stdlib.h>
#include "fuga.h"

enum FugaTokenType {
    FUGA_TOKEN_ERROR = 255,
    FUGA_TOKEN_END = 0,
    
    // simple tokens
    FUGA_TOKEN_SEPARATOR,
    FUGA_TOKEN_EQUALS,
    FUGA_TOKEN_LPAREN,
    FUGA_TOKEN_RPAREN,

    // more interesting tokens
    FUGA_TOKEN_INT,
    FUGA_TOKEN_STRING,
    FUGA_TOKEN_SYMBOL,
    FUGA_TOKEN_DOC,
    FUGA_TOKEN_NAME,
    FUGA_TOKEN_OP
};

typedef struct FugaToken FugaToken;
typedef struct FugaTokenPosition FugaTokenPosition;
typedef enum FugaTokenType FugaTokenType;

struct FugaToken {
    FugaTokenType type;
    char* filename;
    size_t line;
    size_t column;
    void* value;
};

/**
*** ### FugaToken_new
*** 
*** Construct a blank token.
*** 
*** - Params:
***     - void* gc: a pointer to some garbage collected object
*** - Return: A blank token.
**/
FugaToken* FugaToken_new(
    void* gc
);

/**
*** ### FugaToken_int_
*** 
*** Construct a Fuga int out of a token.
***
*** - Params:
***     - FugaToken* token: must be a FUGA_TOKEN_INT
***     - Fuga* self: any Fuga object.
*** - Return: the Fuga int equivalent of the given token's value.
**/
Fuga* FugaToken_int_(
    FugaToken* token,
    Fuga* self
);

/**
*** ### FugaToken_string_
*** 
*** Construct a Fuga string out of a token.
***
*** - Params:
***     - FugaToken* token
***     - Fuga* self: any Fuga object.
*** - Return: the Fuga string equivalent of the given token's value.
**/
Fuga* FugaToken_string_(
    FugaToken* token,
    Fuga* self
);

/**
*** ### FugaToken_symbol_
*** 
*** Construct a Fuga symbol out of a token.
***
*** - Params:
***     - FugaToken* token
***     - Fuga* self: any Fuga object.
*** - Return: the Fuga symbol equivalent of the given token's value.
**/
Fuga* FugaToken_symbol_(
    FugaToken* token,
    Fuga* self
);

#endif

