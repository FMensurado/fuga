#ifndef FUGA_TOKEN_H
#define FUGA_TOKEN_H

#include <stdlib.h>

enum FugaTokenType {
    FUGA_TOKEN_ERROR = 255,
    FUGA_TOKEN_END = 0,
    
    // simple tokens
    FUGA_TOKEN_SEPARATOR,
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

FugaToken* FugaToken_new(
    void* gc
);

/*
Fuga* FugaToken_int(FugaToken* token, Fuga* self);
Fuga* FugaToken_string(FugaToken* token, Fuga* self);
Fuga* FugaToken_symbol(FugaToken* token, Fuga* self);
*/

#endif

