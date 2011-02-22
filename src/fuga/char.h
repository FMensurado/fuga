#pragma once
#ifndef FUGA_CHAR
#define FUGA_CHAR

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

size_t FugaChar_size(const char*);

// generic character types
bool   FugaChar_isSpace(const char*);
bool   FugaChar_isOp(const char*);
bool   FugaChar_isName(const char*);
bool   FugaChar_isDigit(const char*);
bool   FugaChar_isSpecial(const char*);

// escaping
bool   FugaChar_escapeError(const char*);
size_t FugaChar_sizeEscaped(const char*);
size_t FugaChar_sizeUnescaped(const char*);
void   FugaChar_escape(char* dest, const char* src);
void   FugaChar_unescape(char* dest, const char* src);

#endif

