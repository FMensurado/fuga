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
    Fuga_setS(FUGA->String, "_name",  FUGA_STRING("String"));
    Fuga_setS(FUGA->String, "_depth", FUGA_INT(5));

    Fuga_setS(FUGA->String, "str", FUGA_METHOD_STR (FugaString_str));
    Fuga_setS(FUGA->String, "++",  FUGA_METHOD_1(FugaString_cat_));
    Fuga_setS(FUGA->String, "match", FUGA_METHOD_1(FugaString_match_));
    Fuga_setS(FUGA->String, "split", FUGA_METHOD_1(FugaString_split_));
    Fuga_setS(FUGA->String, "join",  FUGA_METHOD_1(FugaString_join_));
    Fuga_setS(FUGA->String, "lower", FUGA_METHOD_0(FugaString_lower));
    Fuga_setS(FUGA->String, "upper", FUGA_METHOD_0(FugaString_upper));

    Fuga_setS(FUGA->String, "from",   FUGA_METHOD_1(FugaString_from_M));
    Fuga_setS(FUGA->String, "to",     FUGA_METHOD_1(FugaString_to_M));
    Fuga_setS(FUGA->String, "fromTo", FUGA_METHOD_2(FugaString_from_to_M));
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

void* FugaString_match_(FugaString* self, FugaString* other) 
{
    FUGA_NEED(self);
    FUGA_NEED(other);
    if (!Fuga_isString(self) || !Fuga_isString(other))
        FUGA_RAISE(FUGA->MatchError, "String match: matches only on strs");
    if ((self->length != other->length) || (self->size != other->size))
        FUGA_RAISE(FUGA->MatchError, "String match: lengths don't match");
    if (memcmp(self->data, other->data, self->size))
        FUGA_RAISE(FUGA->MatchError, "String match: no match");
    return Fuga_clone(FUGA->Object);
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
    if (start < 0) start = 0;
    if (end < 0) end += self->length;
    if (end < 0) end = 0;
    if (end   > self->length) end = self->length;
    if (start >= end) return FUGA_STRING("");

    const char *src = self->data;
    long i = 0;
    size_t size = 0;
    for (; i < start; i++)
        src += FugaChar_size(src); 
    for (; i < end; i++)
        size += FugaChar_size(src + size);
    
    char buffer[size+1];
    memcpy(buffer, src, size);
    buffer[size] = 0;
    return FUGA_STRING(buffer);
}

#ifdef TESTING
TESTS(FugaString_from_to_) {
    void* self = Fuga_init();
    FugaString* src = FUGA_STRING("abcdefgh");

    TEST(FugaString_is_(FugaString_from_to_(src, 0, 8), "abcdefgh"));
    TEST(FugaString_is_(FugaString_from_to_(src, 0, 7), "abcdefg"));
    TEST(FugaString_is_(FugaString_from_to_(src, 0, -1), "abcdefg"));
    TEST(FugaString_is_(FugaString_from_to_(src, 1, -1), "bcdefg"));
    TEST(FugaString_is_(FugaString_from_to_(src, 3, 5), "de"));
    TEST(FugaString_is_(FugaString_from_to_(src, 4, 5), "e"));
    TEST(FugaString_is_(FugaString_from_to_(src, -5, -2), "def"));
    TEST(FugaString_is_(FugaString_from_to_(src, -10, -2), "abcdef"));
    TEST(FugaString_is_(FugaString_from_to_(src, -5, -10), ""));
    TEST(FugaString_is_(FugaString_from_to_(src, 0, 0), ""));

    Fuga_quit(self);
}
#endif

FugaString* FugaString_to_(
    FugaString* self,
    long end
) {
    ALWAYS(self);
    FUGA_CHECK(self);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String slice: expected string");
    if (end <  0) end += self->length;
    if (end <= 0) return FUGA_STRING("");
    if (end >= self->length) return self;

    const char *src = self->data;
    long i = 0;
    size_t size = 0;
    for (; i < end; i++)
        size += FugaChar_size(src + size);
    
    char buffer[size+1];
    memcpy(buffer, src, size);
    buffer[size] = 0;
    return FUGA_STRING(buffer);
}

FugaString* FugaString_from_M(FugaString* self, FugaInt* from) {
    FUGA_NEED(self); FUGA_NEED(from);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String from: expected string");
    if (!Fuga_isInt(from))
        FUGA_RAISE(FUGA->TypeError, "String from: expected int");
    return FugaString_from_(self, FugaInt_value(from));
}

FugaString* FugaString_from_to_M(
    FugaString* self,
    FugaInt* from,
    FugaInt* to
) {
    FUGA_NEED(self); FUGA_NEED(from); FUGA_NEED(to);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String fromTo: expected string");
    if (!Fuga_isInt(from))
        FUGA_RAISE(FUGA->TypeError, "String fromTo: expected int");
    if (!Fuga_isInt(to))
        FUGA_RAISE(FUGA->TypeError, "String fromTo: expected int");
    return FugaString_from_to_(self,
        FugaInt_value(from),
        FugaInt_value(to)
    );
}

FugaString* FugaString_to_M(FugaString* self, FugaInt* to) {
    FUGA_NEED(self); FUGA_NEED(to);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String to: expected string");
    if (!Fuga_isInt(to))
        FUGA_RAISE(FUGA->TypeError, "String to: expected int");
    return FugaString_to_(self, FugaInt_value(to));
}

FugaString* FugaString_cat_(FugaString* self, FugaString* other)
{
    ALWAYS(self); ALWAYS(other);
    FUGA_NEED(self); FUGA_NEED(other);
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
***     >>> " foo" split(" ")
***     ("", "foo")
***     
**/
void* FugaString_split_(
    FugaString* self,
    FugaString* splitter
) {
    bool ws = false;
    ALWAYS(self);
    FUGA_NEED(self);

    if (!splitter) {
        ws = true;
        splitter = FUGA_STRING(" \t\r\n");
    }
    FUGA_NEED(splitter);
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
            if (!ws || !FugaString_is_(frag, ""))
                FUGA_CHECK(Fuga_append_(results, frag));
        }
    }
    frag = FugaString_from_(self, start);
    FUGA_CHECK(frag);
    if (!ws || !FugaString_is_(frag, ""))
        FUGA_CHECK(Fuga_append_(results, frag));
    return results;
}


#ifdef TESTING
#define FUGA_STRING_SPLIT_TEST(t, s, r) \
    TEST(FugaString_is_(Fuga_str(FugaString_split_(FUGA_STRING(t), \
                                        s ? FUGA_STRING(s) : NULL)), \
                        r))
TESTS(FugaString_split_) {
    void* self = Fuga_init();

    FUGA_STRING_SPLIT_TEST("Hello world!", " ",
                           "(\"Hello\", \"world!\")")
    FUGA_STRING_SPLIT_TEST("foo bar baz", " ",
                           "(\"foo\", \"bar\", \"baz\")")
    FUGA_STRING_SPLIT_TEST("foo bar baz", " a",
                           "(\"foo\", \"b\", \"r\", \"b\", \"z\")")
    FUGA_STRING_SPLIT_TEST(" a  b ", NULL,
                           "(\"a\", \"b\")")
    FUGA_STRING_SPLIT_TEST(" a  b ", " ",
                           "(\"\", \"a\", \"\", \"b\", \"\")")

    Fuga_quit(self);
}
#endif

FugaString* FugaString_join_(
    FugaString* self,
    void* object
) {
    FUGA_NEED(self); FUGA_NEED(object);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, 
            "String join: expected self to be a primitive string"
        );

    FugaString* result = FUGA_STRING("");
    FUGA_FOR(i, slot, object) {
        FUGA_NEED(slot);
        if (!Fuga_isString(slot)) {
            slot = Fuga_str(slot);
            if (!Fuga_isString(slot))
                FUGA_RAISE(FUGA->TypeError,
                    "String join: string conversion failed..."
                );
        }
        
        if (i > 0)
            result = FugaString_cat_(result, self);
        result = FugaString_cat_(result, slot); 
        FUGA_CHECK(result);
    }
    return result;
}

#ifdef TESTING
TESTS(FugaString_join_) {
    void* self = Fuga_init();
    void* parts;
    FugaString *joiner, *result;

    parts = Fuga_clone(FUGA->Object);
    Fuga_append_(parts, FUGA_STRING("hello"));
    Fuga_append_(parts, FUGA_STRING("world"));
    joiner = FUGA_STRING(" ");
    result = FugaString_join_(joiner, parts);
    TEST(FugaString_is_(result, "hello world"));

    parts = Fuga_clone(FUGA->Object);
    Fuga_append_(parts, FUGA_STRING("hello"));
    joiner = FUGA_STRING(" ");
    result = FugaString_join_(joiner, parts);
    TEST(FugaString_is_(result, "hello"));

    parts = Fuga_clone(FUGA->Object);
    Fuga_append_(parts, FUGA_STRING(""));
    joiner = FUGA_STRING(" ");
    result = FugaString_join_(joiner, parts);
    TEST(FugaString_is_(result, ""));

    parts = Fuga_clone(FUGA->Object);
    Fuga_append_(parts, FUGA_INT(10));
    Fuga_append_(parts, FUGA_INT(20));
    joiner = FUGA_STRING(" ");
    result = FugaString_join_(joiner, parts);
    TEST(FugaString_is_(result, "10 20"));

    parts = Fuga_clone(FUGA->Object);
    Fuga_append_(parts, FUGA_INT(10));
    Fuga_append_(parts, FUGA_INT(20));
    Fuga_append_(parts, FUGA_INT(30));
    joiner = FUGA_STRING(" + ");
    result = FugaString_join_(joiner, parts);
    TEST(FugaString_is_(result, "10 + 20 + 30"));

    Fuga_quit(self);
}
#endif


FugaString* FugaString_lower(FugaString* self) {
    FUGA_NEED(self);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String lower: not a string");

    FugaString* dest = FUGA_STRING(self->data); // make a copy
    FUGA_CHECK(dest);

    size_t j = 0;
    for (size_t i=0; i < self->size;
        i += FugaChar_size(self->data+i),
        j += FugaChar_size(dest->data+j))
    {
        ALWAYS(j < dest->size);
        FugaChar_lower(dest->data+j, self->data+i);
    }
    dest->data[j] = 0;
    dest->size    = j;

    return dest;
}

FugaString* FugaString_upper(FugaString* self) {
    FUGA_NEED(self);
    if (!Fuga_isString(self))
        FUGA_RAISE(FUGA->TypeError, "String upper: not a string");

    FugaString* dest = FUGA_STRING(self->data); // make a copy
    FUGA_CHECK(dest);

    size_t j = 0;
    for (size_t i=0; i < self->size;
        i += FugaChar_size(self->data+i),
        j += FugaChar_size(dest->data+j))
    {
        ALWAYS(j < dest->size);
        FugaChar_upper(dest->data+j, self->data+i);
    }
    dest->data[j] = 0;
    dest->size    = j;

    return dest;
}
