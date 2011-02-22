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
size_t FugaChar_sizeEscaped(
    const char* c
) {
    if (*c == '\\') return 2;
    return 1;
}

size_t FugaChar_sizeUnescaped(
    const char* c
) {
    if ((c[0] == '\\') && (c[1] == '\n'))
        return 0;
    return 1;
}

void FugaChar_unescape(
    char *dest,
    const char *src
) {
    if (src[0] == '\\') {
        switch (src[1]) {
        case '\n': break;
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
