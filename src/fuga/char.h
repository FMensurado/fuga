#pragma once
#ifndef FUGA_CHAR
#define FUGA_CHAR

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bool   FugaChar_isSpace(const char*);
bool   FugaChar_isOp(const char*);
bool   FugaChar_isName(const char*);
bool   FugaChar_isDigit(const char*);
bool   FugaChar_isSpecial(const char*);
size_t FugaChar_size(const char*);

#endif

