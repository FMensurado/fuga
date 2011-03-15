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
*** ### FugaParser_operators
***
*** Return the Parser's operators metadata object.
**/
void* FugaParser_operators(
    FugaParser* self
);

/**
*** ### FugaParser_infixr_precedence_
*** 
*** Set a right-associative operator's precedence.
**/
void FugaParser_infix_precedence_(
    FugaParser* self,
    const char* op,
    size_t precedence
);

/**
*** ### FugaParser_precedence_
***
*** Return an operator's precedence.
**/
size_t FugaParser_precedence_(
    FugaParser* self,
    const char* op
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

/**
*** ### FugaParser_expression_
***
*** Parse a single Fuga expression at a given binding power.
**/
void* FugaParser_expression_(
    FugaParser* self,
    size_t rbp
);


#endif

