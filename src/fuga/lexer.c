#include "lexer.h"
#include "test.h"
#include "gc.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct FugaLexer {
    char* code;     // current code pointer
    char* codeBase; // base code pointer
    char* codeEnd;
    char* filename;
    size_t line;
    size_t column;
    FugaToken* token;
};

void _FugaLexer_free(
    void* _self
) {
    FugaLexer* self = _self;
    free(self->codeBase);
}

void _FugaLexer_mark(
    void* _self
) {
    FugaLexer* self = _self;
    FugaGC_mark(self, self->token);
    FugaGC_mark(self, self->filename);
}

FugaLexer* FugaLexer_new(
    void* gc
) {
    FugaLexer* self = FugaGC_alloc(gc, sizeof(FugaLexer));
    FugaGC_onFree(self, _FugaLexer_free);
    FugaGC_onMark(self, _FugaLexer_mark);
    return self;
}

void FugaLexer_readCode_(
    FugaLexer* self,
    const char* code
) {
    self->code = self->codeBase = malloc(strlen(code)+1);
    strcpy(self->code, code);
    self->codeEnd = self->code + strlen(self->code);
    self->line = self->column = 1;
}

void FugaLexer_readFile_(
    FugaLexer* self,
    const char* filename
);

void _FugaLexer_strip     (FugaLexer*);
void _FugaLexer_lex       (FugaLexer*);
void _FugaLexer_lexInt    (FugaLexer*);
void _FugaLexer_lexOp     (FugaLexer*);
void _FugaLexer_lexDoc    (FugaLexer*);
void _FugaLexer_lexName   (FugaLexer*);
void _FugaLexer_lexString (FugaLexer*);
void _FugaLexer_lexSymbol (FugaLexer*);
void _FugaLexer_lexSingle (FugaLexer*);
void _FugaLexer_lexError  (FugaLexer*);

FugaToken* FugaLexer_peek(
    FugaLexer* self
) {
    if (!self->token) {
        _FugaLexer_strip(self);
        self->token = FugaToken_new(self);
        self->token->filename = self->filename;
        self->token->line     = self->line;
        self->token->column   = self->column;
        _FugaLexer_lex(self);
    }
    return self->token;
}

FugaToken* FugaLexer_next(
    FugaLexer* self
) {
    FugaToken* result = FugaLexer_peek(self);
    self->token = NULL;
    return result;
}

/**
*** ### _FugaLexer_consume_
*** 
*** Consume n characters.
**/
void _FugaLexer_consume_(
    FugaLexer* self,
    size_t num_chars
) {
    size_t i;
    for (i = 0; i < num_chars; i++) {
        switch (self->code[i]) {
        case '\n':
            self->line++;
            self->column = 1;
            break;

        case '\t':
            self->column = (self->column & ~7) + 8;
            break;

        case '\r':
            break;

        case '\0':
            goto _FugaLexer_consume_end;
        
        default:
            self->column++;
        }
    }
    
  _FugaLexer_consume_end:
    self->code += i;
}

/**
*** ### _FugaLexer_prefix_
*** 
*** Get a prefix of the code of a certain length, consuming the 
*** characters in the process. The returned string is not GCed --
*** only use traditional C allocation for it.
**/
char* _FugaLexer_prefix_(
    FugaLexer* self,
    size_t num_chars
) {
    // FIXME: prevent getting strings that are larger than the code.
    if (self->code + num_chars > self->codeEnd)
        num_chars = (size_t)(self->codeEnd - self->code);
    char* value = malloc(num_chars+1);
    memcpy(value, self->code, num_chars);
    value[num_chars] = 0;
    _FugaLexer_consume_(self, num_chars);
    return value;
}

/**
*** ### _FugaLexer_strip
*** 
*** Remove whitespace and comments from the beginning of the stream.
**/
void _FugaLexer_strip(
    FugaLexer* self
) {
    int i=0;
    while (self->code[i] == ' '  ||
           self->code[i] == '\t' ||
           self->code[i] == '\r')
        i++;
    if (self->code[i] == '#') {
        i++;
        while ((self->code[i] != '\0') &&
               (self->code[i] != '\n'))
            i++;
        _FugaLexer_consume_(self, i);
        _FugaLexer_strip(self);
    } else {
        _FugaLexer_consume_(self, i);
    }
}

bool _FugaLexer_issingle(
    char c
) {
    switch (c) {
    case '(': case ')': case '[': case ']': case '{': case '}':
    case '\n': case ',': case '\0':
        return true;

    default:
        return false;
    }
}

bool _FugaLexer_isop(
    char c
) {
    switch (c) {
    case '~': case '`': case '@': case '$': case '%': case '^':
    case '&': case '*': case '-': case '+': case '=': case '|':
    case ';': case ':': case '<': case '>': case '.': case '?':
    case '/':
        return true;

    default:
        return false;
    }
}

bool _FugaLexer_isname(
    char c
) {
    return c == '_' || c == '?' || c == '!' || isalnum(c);
}

void _FugaLexer_lex(
    FugaLexer* self
) {
    char c = self->code[0];
    if (c == '"')
        _FugaLexer_lexString(self);
    else if (c == ':')
        _FugaLexer_lexSymbol(self);
    else if (_FugaLexer_issingle(c))
        _FugaLexer_lexSingle(self);
    else if (_FugaLexer_isop(c))
        _FugaLexer_lexOp(self);
    else if (_FugaLexer_isname(c))
        _FugaLexer_lexName(self);
    else
        _FugaLexer_lexError(self);
}

void _FugaLexer_lexError_(
    FugaLexer* self,
    size_t num_chars
) {
    ALWAYS(self); ALWAYS(self->token); ALWAYS(self->code);
    self->token->type = FUGA_TOKEN_ERROR;
    _FugaLexer_consume_(self, num_chars);
}

void _FugaLexer_lexError(
    FugaLexer* self
) {
    _FugaLexer_lexError_(self, 1);
}

void _FugaLexer_lexSingle(
    FugaLexer* self
) {
    ALWAYS(self); ALWAYS(self->token); ALWAYS(self->code);
    
    switch(self->code[0]) {
    case '(':  self->token->type = FUGA_TOKEN_LPAREN; break;
    case ')':  self->token->type = FUGA_TOKEN_RPAREN; break;
    case ',':  self->token->type = FUGA_TOKEN_SEPARATOR; break;
    case '\n': self->token->type = FUGA_TOKEN_SEPARATOR; break;
    case '\0': self->token->type = FUGA_TOKEN_END; return;
    default: _FugaLexer_lexError(self); return;
    }

    _FugaLexer_consume_(self, 1);
}

void _FugaLexer_lexInt(
    FugaLexer* self
) {
    long value = 0;
    size_t i;
    for (i = 0; isdigit(self->code[i]); i++) {
        value *= 10;
        value += self->code[i] - '0';
    }
    self->token->type = FUGA_TOKEN_INT;
    self->token->value = malloc(sizeof(long));
    *(long*)self->token->value = value;
    _FugaLexer_consume_(self, i);
}

void _FugaLexer_lexName(
    FugaLexer* self
) {
    // FIXME: handle escaped operators
    int i;
    bool digits = true;
    for (i = 0; _FugaLexer_isname(self->code[i]); i++)
        digits = digits && isdigit(self->code[i]);

    if (!i) {
        _FugaLexer_lexError(self);
    } else if (digits) {
        _FugaLexer_lexInt(self);
    } else {
        self->token->type = FUGA_TOKEN_NAME;
        self->token->value = _FugaLexer_prefix_(self, i);
    }
}

void _FugaLexer_lexOp(
    FugaLexer* self
) {
    int i=0;
    while (_FugaLexer_isop(self->code[i]))
        i++;
    if (i == 1 && self->code[0] == '=') {
        self->token->type = FUGA_TOKEN_EQUALS;
        _FugaLexer_consume_(self, 1);
    } else {
        self->token->type = FUGA_TOKEN_OP;
        self->token->value = _FugaLexer_prefix_(self, i);
    }
}

void _FugaLexer_lexDoc(
    FugaLexer* self
) {
    _FugaLexer_consume_(self, 2);
    if (self->code[0] == ' ')
        _FugaLexer_consume_(self, 1);

    int i=0;
    while (self->code[i] != '\n')
        i++;

    self->token->type  = FUGA_TOKEN_DOC;
    self->token->value = _FugaLexer_prefix_(self, i);
}

void _FugaLexer_lexSymbol(
    FugaLexer* self
) {
    char c = self->code[1];
    if (c == ':') {
        _FugaLexer_lexDoc(self);
    } else if (_FugaLexer_isname(c)) {
        _FugaLexer_consume_(self, 1);
        _FugaLexer_lexName(self);
        self->token->type = FUGA_TOKEN_SYMBOL;
    } else {
        _FugaLexer_lexOp(self);
    }
}

void _FugaLexer_lexString(
    FugaLexer* self
) {
    _FugaLexer_consume_(self, 1);

    // determine string length, and validate
    size_t length=0;
    size_t i;
    bool escapeError = false;
    for (i=0; self->code[i] != '"'; i++) {
        switch (self->code[i]) { 
        case '\0':
            _FugaLexer_lexError_(self, i);
            return;
        case '\\':
            i++;
            switch (self->code[i]) {
            case '\0':
                _FugaLexer_lexError_(self, i);
                return;

            case '\n':
                break;
            
            case '\"': case '\\': case 'n': case 't': case 'r':
                length++;
                break;
    
            default:
                escapeError = true;
            }
            break;

        default:
            length++;
        }
    }

    if (escapeError) {
        _FugaLexer_lexError_(self, i+1);
        return;
    }

    // get string value
    // FIXME: add support for \0 and \xff
    char* value = malloc(length+1);
    size_t index = 0;
    for (size_t j=0; j<i; j++) {
        if (self->code[j] == '\\') {
            j++;
            switch (self->code[j]) {
            case '\n': break;
            case '\"': value[index++] = '\"'; break;
            case 'n':  value[index++] = '\n'; break;
            case 't':  value[index++] = '\t'; break;
            case 'r':  value[index++] = '\r'; break;
            default: NEVER(self->code[j] || 1);
            }
        } else {
            value[index++] = self->code[j];
        }
    }

    self->token->type = FUGA_TOKEN_STRING;
    self->token->value = value;
    _FugaLexer_consume_(self, i+1);
}

#ifdef TESTING
bool _FugaToken_test_(
    FugaToken* self,
    FugaTokenType type
) {
    return self->type == type;
}

bool _FugaToken_test_str_(
    FugaToken* self,
    FugaTokenType type,
    const char* msg
) {
    if (self->type != type)
        return false;
    return self->value && (strcmp(self->value, msg) == 0);
}

bool _FugaToken_test_int_(
    FugaToken* self,
    FugaTokenType type,
    long value
) {
    if (self->type != type)
        return false;
    return self->value && (*(long*)self->value == value);
}

bool _FugaLexer_test_(
    FugaLexer* self,
    FugaTokenType type
) {
    return _FugaToken_test_(FugaLexer_next(self), type);
}

bool _FugaLexer_test_str_(
    FugaLexer* self,
    FugaTokenType type,
    const char* msg
) {
    return _FugaToken_test_str_(FugaLexer_next(self), type, msg);
}

bool _FugaLexer_test_int_(
    FugaLexer* self,
    FugaTokenType type,
    long value
) {
    return _FugaToken_test_int_(FugaLexer_next(self), type, value);
}

#define FUGA_LEXER_TEST(type) \
    TEST(_FugaLexer_test_(self, type))

#define FUGA_LEXER_TEST_STR(type, msg) \
    TEST(_FugaLexer_test_str_(self, type, msg))

#define FUGA_LEXER_TEST_INT(type, value) \
    TEST(_FugaLexer_test_int_(self, type, value))

TESTS(FugaLexer) {
    void* gc = FugaGC_start();
    FugaLexer* self = FugaLexer_new(gc);

    FugaLexer_readCode_(self, "");
    FUGA_LEXER_TEST(FUGA_TOKEN_END);
    FUGA_LEXER_TEST(FUGA_TOKEN_END);
    FUGA_LEXER_TEST(FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"");
    FUGA_LEXER_TEST(FUGA_TOKEN_ERROR);
    FUGA_LEXER_TEST(FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello, world!\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello, world!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello");
    FUGA_LEXER_TEST(FUGA_TOKEN_ERROR);
    FUGA_LEXER_TEST(FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello,\\nworld!\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello,\nworld!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello\\tworld!\\r\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello\tworld!\r");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello,\\\"world!\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello,\"world!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello, \\\nworld!\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello, world!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\"Hello, \\world!\"");
    FUGA_LEXER_TEST(FUGA_TOKEN_ERROR);
    FUGA_LEXER_TEST(FUGA_TOKEN_END);

    FugaLexer_readCode_(self, ":doremi");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "doremi");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, ":do :re :mi");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "do");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "re");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "mi");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, ":do,:re\n:mi");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "do");
    FUGA_LEXER_TEST    (FUGA_TOKEN_SEPARATOR);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "re");
    FUGA_LEXER_TEST    (FUGA_TOKEN_SEPARATOR);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "mi");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "do");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "do");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "a = b");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "a");
    FUGA_LEXER_TEST    (FUGA_TOKEN_EQUALS);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "b");

    FugaLexer_readCode_(self, "0");
    FUGA_LEXER_TEST    (FUGA_TOKEN_INT);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "0");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 0);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "40");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 40);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "1st");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "1st");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "do re mi");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "do");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "re");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "mi");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "do. re? mi!");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "do");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_OP,   ".");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "re?");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "mi!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "do. re? mi!");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "do");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_OP,   ".");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "re?");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "mi!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaGC_end(gc);
}
#endif

