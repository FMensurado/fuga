#include <string.h>
#include <stdio.h>
#include "string.h"
#include "test.h"
#include "symbol.h"
#include "char.h"

const FugaType FugaString_type = {
    .name = "String"
};

void FugaString_init(void* self)
{
    Fuga_setS(FUGA->String, "str", FUGA_METHOD_STR (FugaString_str));
    Fuga_setS(FUGA->String, "++",  FUGA_METHOD_1ARG(FugaString_cat_));
}

FugaString* FugaString_new(void* self, const char* value)
{
    ALWAYS(self); ALWAYS(value);
    FUGA_CHECK(self);
    size_t size = strlen(value);
    FugaString* result = Fuga_clone_(
        FUGA->String, 
        sizeof(FugaString) + size + 1
    );
    Fuga_type_(result, &FugaString_type);
    result->size   = size;
    result->length = size; // FIXME: determine actual string length
    strcpy(result->data, value);
    return result;
}

#ifdef TESTING
TESTS(FUGA_STRING) {
    FugaString* self = Fuga_init();
    TEST(Fuga_isString(FUGA_STRING("")));
    TEST(Fuga_isString(FUGA_STRING("hello")));
    Fuga_quit(self);
}
#endif


FugaSymbol* FugaString_toSymbol(FugaString* self)
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

void FugaString_print(FugaString* self)
{
    ALWAYS(self);
    ALWAYS(Fuga_isString(self));
    
    puts(self->data);
}


bool FugaString_is_(FugaString* self, const char* str)
{
    ALWAYS(self);
    self = Fuga_need(self);
    return Fuga_isString(self) && (strcmp(self->data, str) == 0);
}


void* FugaString_str(void* _self)
{
    FugaString* self = _self;
    ALWAYS(self);
    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String str: expected primitive string"
        );
    }

    char* c = self->data;
    size_t size = 0;
    for (size_t i = 0; i < self->size;) {
        size += FugaChar_sizeAfterEscape (c+i);
        i    += FugaChar_sizeBeforeEscape(c+i);
    }

    char* buffer = malloc(size+3);
    size_t index = 0;
    buffer[index++] = '"';
    for (size_t i = 0; i < self->size;) {
        FugaChar_escape(buffer+index, c+i);
        index += FugaChar_sizeAfterEscape (c+i);
        i     += FugaChar_sizeBeforeEscape(c+i);
    }
    buffer[index++] = '"';
    buffer[index++] = 0;
    FugaString* result = FUGA_STRING(buffer);
    free(buffer);
    return result;
}

FugaString* FugaString_from_(FugaString* self, long start)
{
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isString(self)) {
        FUGA_RAISE(FUGA->TypeError,
            "String slice: expected primitive string"
        );
    }

    if (start < 0)
        start += self->length;
        
    if (start >= self->length)
        return FUGA_STRING("");

    if (start < 0)
        return self;

    // FIXME: start is not really the right offset to the start'th char.
    return FUGA_STRING(self->data + start);
}

void* FugaString_cat_(void* _self, void* _other)
{
    FugaString* self  = _self;
    FugaString* other = _other;
    ALWAYS(self); ALWAYS(other);
    if (!Fuga_isString(self) || !Fuga_isString(other)) {
        FUGA_RAISE(FUGA->TypeError,
            "String \\++: expected primitive strings"
        );
    }

    char buffer[self->size + other->size + 1];
    memcpy(buffer,               self->data,  self->size);
    memcpy(buffer + self->size, other->data, other->size + 1);
    return FUGA_STRING(buffer);
}

