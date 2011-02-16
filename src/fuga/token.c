#include "token.h"
#include "gc.h"
#include "test.h"

#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void _FugaToken_free(
    void* _self
) {
    FugaToken* self = _self;
    free(self->value);
}

void _FugaToken_mark(
    void* _self
) {
    FugaToken* self = _self;
    FugaGC_mark(self, self->filename);
}

FugaToken* FugaToken_new(
    void* gc
) {
    FugaToken* self = FugaGC_alloc(gc, sizeof(FugaToken));
    FugaGC_onFree(self, _FugaToken_free);
    FugaGC_onMark(self, _FugaToken_mark);
    return self;
}

Fuga* FugaToken_int_(
    FugaToken* token,
    Fuga* self
) {
    ALWAYS(token); ALWAYS(token->value);
    ALWAYS(token->type == FUGA_TOKEN_INT);
    return FUGA_INT(*(long*)token->value);
}

Fuga* FugaToken_string_(
    FugaToken* token,
    Fuga* self
) {
    ALWAYS(token); ALWAYS(token->value);
    NEVER(token->type == FUGA_TOKEN_INT);
    return FUGA_STRING(token->value);
}

Fuga* FugaToken_symbol_(
    FugaToken* token,
    Fuga* self
) {
    ALWAYS(token); ALWAYS(token->value);
    NEVER(token->type == FUGA_TOKEN_INT);
    return FUGA_SYMBOL(token->value);
}

