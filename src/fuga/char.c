#include "char.h"
#include "test.h"
#include <ctype.h>

size_t FugaChar_size(
    const char *c
) {
    return 1;
}

bool FugaChar_isOp(
    const char *c
) {
    switch (*c) {
    case '~': case '`': case '@': case '$': case '%': case '^':
    case '&': case '*': case '-': case '+': case '=': case '|':
    case ';': case ':': case '<': case '>': case '.': case '?':
    case '/': case '\'':
        return true;

    default:
        return false;
    }
}


bool FugaChar_isName(
    const char *c
) {
    return *c == '_' || *c == '?' || *c == '!' || isalnum(*c);
}

bool FugaChar_isDigit(
    const char* c
) {
    return isdigit(*c);
}

bool FugaChar_isSpecial(
    const char* c
) {
    switch (*c) {
    case '(': case ')': case '[': case ']': case '{': case '}':
    case '\n': case ',': case '\0':
        return true;

    default:
        return false;
    }
}

bool FugaChar_escapeError(
    const char* c
) {
    if (*c == '\0')
        return true;
    if (*c == '\\') {
        switch (c[1]) {
        case '\\': case '\n': case '\"': case 'r': case 'n': case 't':
            return false;
        default:
            return true;
        }
    }
    return false;
}

// size in string

// return the size of a character in a string with escapes
size_t FugaChar_sizeBeforeUnescape(
    const char* c
) {
    if (*c == '\\') return 2;
    return 1;
}

// return the size of a character in a string with escapes after it's
// been unescaped
size_t FugaChar_sizeAfterUnescape(
    const char* c
) {
    if ((c[0] == '\\') && (c[1] == '\n'))
        return 0;
    return 1;
}

size_t FugaChar_sizeBeforeEscape(
    const char* c
) {
    return FugaChar_size(c);
}

size_t FugaChar_sizeAfterEscape(
    const char* c
) {
    switch (*c) {
        case '\\': case '\"': case '\n': case '\t': case '\r':
            return 2;
        default:
            return 1;
    }
}

void FugaChar_unescape(
    char *dest,
    const char *src
) {
    if (src[0] == '\\') {
        switch (src[1]) {
        case '\n': break;
        case '\\': *dest = '\\'; break;
        case '\"': *dest = '\"'; break;
        case 'n':  *dest = '\n'; break;
        case 't':  *dest = '\t'; break;
        case 'r':  *dest = '\r'; break;
        default: NEVER(1);
        }
    } else {
        *dest = *src;
    }
}

void FugaChar_escape(
    char* dest,
    const char *src
) {
    dest[0] = '\\';
    switch (src[0]) {
    case '\\': dest[1] = '\\';      break;
    case '\"': dest[1] = '\"';      break;
    case '\n': dest[1] = 'n';       break;
    case '\r': dest[1] = 'r';       break;
    case '\t': dest[1] = 't';       break;
    default:   dest[0] = src[0];
    }
}

void FugaChar_lower(char* dest, const char* src) {
    if (*src >= 'A' && *src <= 'Z') {
        *dest = *src + 'a' - 'A';
    } else {
        *dest = *src;
    }
}

void FugaChar_upper(char* dest, const char* src) {
    if (*src >= 'a' && *src <= 'z') {
        *dest = *src + 'A' - 'a';
    } else {
        *dest = *src;
    }
}
