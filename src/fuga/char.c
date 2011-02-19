#include "char.h"
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

