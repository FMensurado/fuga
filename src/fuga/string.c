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

/**
*** ### FugaString_from_to_
*** 
*** Return a the character at the given index as a single string.
**/
FugaString* FugaString_from_to_(
    FugaString* self,
    long start,
    long end
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String slice: expected string");
    if (start < 0) start += self->length;
    if (start < 0)
        FUGA_RAISE(FUGA->ValueError,
            "String slice: start index too negative"
        );
    if (end < 0) end += self->length;
    if (end < 0)
        FUGA_RAISE(FUGA->ValueError,
            "String slice: end index too negative"
        );

    if (start >= self->length) return FUGA_STRING("");
    if (end   >= self->length) end = self->length;
    if (start >= end) return FUGA_STRING("");

    const char *src = self->data;
    long i;
    size_t size;
    for (i=0; i < start; i++)
        src += FugaChar_size(src); 
    for (size=0; i < end; i++);
        size += FugaChar_size(src + size);

    char buffer[size+1];
    memcpy(buffer, src, size);
    buffer[size] = 0;
    return FUGA_STRING(buffer);
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

/**
*** ### FugaString_find_
***
*** Find the first occurence of a substring within self. If there is no
*** such occurence, return -1.
**/
FugaInt* FugaString_find_(
    FugaString* self,
    FugaString* substring
) {
    ALWAYS(self); ALWAYS(substring);
    FUGA_CHECK(self); FUGA_CHECK(substring);
    if (!Fuga_isString(self) || !Fuga_isString(substring))
        FUGA_RAISE(FUGA->TypeError, "String find: expected strings");    
    const char* src = self->data;
    for (long i = 0; i <= self->length - substring->length; i++) {
        if (memcmp(src, substring->data, substring->size) == 0)
            return FUGA_INT(i);
        src += FugaChar_size(src);
    }
    return FUGA_INT(-1);
}

/**
*** ### FugaString_in_
***
*** Is self contained in superstring?
**/
void* FugaString_in_(
    FugaString* self,
    FugaString* superstring
) {
    FugaInt* index = FugaString_find_(superstring, self);
    FUGA_CHECK(index);
    return FUGA_BOOL(!FugaInt_is_(index, -1));
}

/**
*** ### FugaString_at_
*** 
*** Return a the character at the given index as a single string.
**/
FugaString* FugaString_at_(
    FugaString* self,
    FugaInt* index
) {
    ALWAYS(self);     ALWAYS(index);
    FUGA_CHECK(self); FUGA_CHECK(index);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String at: expected string");
    if (!Fuga_isInt(index))
        FUGA_RAISE(FUGA->TypeError, "String at: expected int");
    if (FugaInt_value(index) < 0)
        FUGA_RAISE(FUGA->ValueError, "String at: index negative");
    if (FugaInt_value(index) >= self->length)
        FUGA_RAISE(FUGA->ValueError, "String at: index too large");

    const char *src = self->data;
    long indexc = FugaInt_value(index);
    for (long i = 0; i < indexc; i++)
        src += FugaChar_size(src); 

    size_t charSize = FugaChar_size(src);
    char buffer[charSize+1];
    memcpy(buffer, src, charSize);
    buffer[charSize] = 0;
    return FUGA_STRING(buffer);
}

/**
*** ### FugaString_split_
*** 
*** Split a string using a character mask. Return an object whose slots
*** are the given substrings.
***
***     >>> "Hello world!" split(" ")
***     ("Hello", "world!")
***     >>> "Foo Bar Baz" split("a ")
***     ("Foo", "B", "r", "B", "z")
***     
**/
void* FugaString_split_(
    FugaString* self,
    FugaString* splitter
) {
    ALWAYS(self);       ALWAYS(splitter);
    FUGA_NEED(self);    FUGA_NEED(splitter);
    if (!Fuga_isString(self) || !Fuga_isString(splitter))
        FUGA_RAISE(FUGA->TypeError,
            "String split: expected primitive strings"
        );

    void* results = Fuga_clone(FUGA->Object);
    size_t start  = 0;
    FugaString* frag;
    for (size_t i = 0; i < self->length; i++) {
        FugaString* chr = FugaString_at_(self, FUGA_INT(i));
        FUGA_CHECK(chr);
        if (Fuga_isTrue(FugaString_in_(chr, splitter))) {
            frag = FugaString_from_to_(self, start, i);
            FUGA_CHECK(frag);
            start = i+1;
            if (!FugaString_is_(frag, ""))
                FUGA_CHECK(Fuga_append_(results, frag));
        }
    }
    frag = FugaString_from_(self, start);
    FUGA_CHECK(frag);
    if (!FugaString_is_(frag, ""))
        FUGA_CHECK(Fuga_append_(results, frag));
    return results;
}

