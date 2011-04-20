#ifndef FUGA_PARSER_H
#define FUGA_PARSER_H

#include "fuga.h"
#include "lexer.h"

typedef struct FugaParser FugaParser;

/**
*** ### FugaParser_new
***
*** Construct a parser.
*** 
*** - Params:
***     - Fuga* self: any Fuga object.
*** - Return: a fresh parser.
**/
FugaParser* FugaParser_new(
    void* self
);

/**
*** ### FugaParser_readCode_
***
*** Set what code to parse.
**/
void FugaParser_readCode_(
    FugaParser* parser,
    const char* code
);

/**
*** ### FugaParser_readFile_
*** 
*** Read a file to parse the code therein.
**/
bool FugaParser_readFile_(
    FugaParser* parser,
    const char* filename
);

/**
*** ### FugaParser_block
***
*** Parse a block (i.e., an object without the parentheses).
**/
void* FugaParser_block(
    FugaParser* self
);

/**
*** ### FugaParser_expression
***
*** Parse a single Fuga expression.
**/
void* FugaParser_expression(
    FugaParser* self
);

#endif

