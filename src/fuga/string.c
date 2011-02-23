#include <string.h>
#include "string.h"
#include "test.h"
#include "symbol.h"
#include "char.h"

void FugaString_init(Fuga* self)
{
    Fuga_setSlot(FUGA->String, FUGA_SYMBOL("str"),
        FUGA_METHOD_STR(FugaString_str));
    Fuga_setSlot(FUGA->String, FUGA_SYMBOL("++"),
        FUGA_METHOD_1ARG(FugaString_cat));
}

Fuga* FugaString_new(Fuga* self, const char* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_CHECK(self);
    self = Fuga_clone(FUGA->String);
    self->type = FUGA_TYPE_STRING;
    self->size = strlen(value)+1;
    self->data = FugaGC_alloc(self, self->size);
    strcpy(self->data, value);
    return self;
}

#ifdef TESTING
TESTS(FUGA_STRING) {
    Fuga* self = Fuga_init();
    TEST(Fuga_isString(FUGA_STRING("")));
    TEST(Fuga_isString(FUGA_STRING("hello")));
    Fuga_quit(self);
}
#endif


Fuga* FugaString_toSymbol(Fuga* self)
{
    ALWAYS(self);

    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String toSymbol: expected primitive string"
        );
    }

    if (!FugaSymbol_isValid(self->data)) {
        FUGA_RAISE(FUGA->TypeError,
            "String toSymbol: invalid name"
        );
    }

    return FUGA_SYMBOL(self->data);
}

#include <stdio.h>
void FugaString_print(Fuga* self)
{
    ALWAYS(self);
    ALWAYS(Fuga_isString(self));
    
    puts(self->data);
}


bool FugaString_isEqualTo(Fuga* self, const char* str)
{
    ALWAYS(self);
    self = Fuga_need(self);
    return Fuga_isString(self) && (strcmp(self->data, str) == 0);
}


Fuga* FugaString_str(Fuga* self)
{
    ALWAYS(self);
    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String str: expected primitive string"
        );
    }

    char* c = self->data;
    size_t size = 0;
    for (size_t i = 0; i < self->size-1;) {
        size += FugaChar_sizeAfterEscape (c+i);
        i    += FugaChar_sizeBeforeEscape(c+i);
    }

    char* buffer = malloc(size+3);
    size_t index = 0;
    buffer[index++] = '"';
    for (size_t i = 0; i < self->size-1;) {
        FugaChar_escape(buffer+index, c+i);
        index += FugaChar_sizeAfterEscape (c+i);
        i     += FugaChar_sizeBeforeEscape(c+i);
    }
    buffer[index++] = '"';
    buffer[index++] = 0;
    Fuga* result = FUGA_STRING(buffer);
    free(buffer);
    return result;
}

Fuga* FugaString_sliceFrom(Fuga* self, long start)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String slice: expected primitive string"
        );
    }

    if (start < 0)
        start += self->size - 1;
        
    // FIXME: self->size does not really represent length.
    if (start < 0 || start >= self->size)
        return FUGA_STRING("");

    // FIXME: start is not really the right offset to the start'th char.
    return FUGA_STRING((char*)self->data + start);
}

Fuga* FugaString_cat(Fuga* self, Fuga* other)
{
    ALWAYS(self); ALWAYS(other);
    if (!Fuga_isString(self) || !Fuga_isString(other)) {
        FUGA_RAISE(FUGA->TypeError,
            "String \\++: expected primitive strings"
        );
    }

    char buffer[self->size + other->size];
    memcpy(buffer,                self->data,  self->size-1);
    memcpy(buffer + self->size-1, other->data, other->size);
    return FUGA_STRING(buffer);
}

