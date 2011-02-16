#ifndef FUGA_LEXER_H
#define FUGA_LEXER_H

#include "token.h"

typedef struct FugaLexer FugaLexer;

/**
*** ### FugaLexer_new
***
*** Construct a new lexical analyzer.
***
*** - Params:
***     - void* gc: A pointer to any garbage collected object, so
***     the lexer may be garbage collected. (If this is NULL, the
***     lexer starts its own gc instance.)
*** - Return: the constructed lexer.
**/
FugaLexer* FugaLexer_new(
    void* gc
);

/**
*** ### FugaLexer_free
***
*** Deconstruct the lexer.
**/
void FugaLexer_free(
    FugaLexer* self
);

/**
*** ### FugaLexer_read_
*** 
*** Specify which code to have the lexer read.
**/
void FugaLexer_read_(
    FugaLexer* self,
    const char* code
);

/**
*** ### FugaLexer_open_
*** 
*** Specify which file to have the lexer read from.
**/
void FugaLexer_open_(
    FugaLexer* self,
    const char* filename
);

/**
*** ### FugaLexer_peek
***
*** Look at the next token without consuming it.
**/
FugaToken* FugaLexer_peek(
    FugaLexer* self
);

/**
*** ### FugaLexer_next
*** 
*** Get the next token and consume it.
**/
FugaToken* FugaLexer_next(
    FugaLexer* self
);

#endif

