#include "token.h"
#include "test.h"

#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void FugaToken_free(
    void* _self
) {
    FugaToken* self = _self;
    free(self->value);
}

void FugaToken_mark(
    void* _self
) {
    FugaToken* self = _self;
    Fuga_mark_(self, self->filename);
}

FugaToken* FugaToken_new(
    void* self
) {
    FugaToken* token = Fuga_clone_(FUGA->Object, sizeof(FugaToken));
    Fuga_onFree_(token, FugaToken_free);
    Fuga_onMark_(token, FugaToken_mark);
    return token;
}

FugaInt* FugaToken_int(
    FugaToken* self
) {
    ALWAYS(self); ALWAYS(self->value);
    ALWAYS(self->type == FUGA_TOKEN_INT);
    return FUGA_INT(*(long*)self->value);
}

FugaString* FugaToken_string(
    FugaToken* self
) {
    ALWAYS(self); ALWAYS(self->value);
    NEVER(self->type == FUGA_TOKEN_INT);
    return FUGA_STRING(self->value);
}

FugaSymbol* FugaToken_symbol(
    FugaToken* self
) {
    ALWAYS(self); ALWAYS(self->value);
    NEVER(self->type == FUGA_TOKEN_INT);
    return FUGA_SYMBOL(self->value);
}

