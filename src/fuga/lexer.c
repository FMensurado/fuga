#include "lexer.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>
#include "char.h"

struct FugaLexer {
    char* code;     // current code pointer
    char* codeBase; // base code pointer
    char* codeEnd;
    char* filename;
    size_t line;
    size_t column;
    FugaToken* token;
};

void FugaLexer_free(
    void* _self
) {
    FugaLexer* self = _self;
    free(self->codeBase);
}

void FugaLexer_mark(
    void* _self
) {
    FugaLexer* self = _self;
    Fuga_mark_(self, self->token);
    Fuga_mark_(self, self->filename);
}

FugaLexer* FugaLexer_new(
    void* self
) {
    FugaLexer* lexer = Fuga_clone_(FUGA->Object, sizeof(FugaLexer));
    Fuga_onFree_(lexer, FugaLexer_free);
    Fuga_onMark_(lexer, FugaLexer_mark);
    return lexer;
}

char* _FugaLexer_strdup(
    const char* src
) {
    char* dest = malloc(strlen(src)+1);
    strcpy(dest, src);
    return dest;
}

void FugaLexer_readCode_(
    FugaLexer* self,
    const char* code
) {
    self->code     = _FugaLexer_strdup(code);
    self->codeBase = self->code;
    self->codeEnd  = self->code + strlen(code);
    self->line     = 1;
    self->column   = 1;
}

#define BUFFSIZE 1024
bool FugaLexer_readFile_(
    FugaLexer* self,
    const char* filename
) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return false;

    char* code = malloc(BUFFSIZE+1);
    size_t length = 0;
    size_t capacity = BUFFSIZE;
    while (1) {
        int c = getc(fp);
        if (c == EOF) break;
        code[length++] = c;
        if (length >= capacity) {
            capacity += BUFFSIZE;
            code = realloc(code, capacity+1);
        }
    }
    code[length] = 0;
    self->code     = code;
    self->codeBase = code;
    self->codeEnd  = code+length;
    self->line     = 1;
    self->column   = 1;
    return true;
}

void _FugaLexer_strip     (FugaLexer*);
void _FugaLexer_lex       (FugaLexer*);
void _FugaLexer_lexInt    (FugaLexer*);
void _FugaLexer_lexOp     (FugaLexer*);
void _FugaLexer_lexDoc    (FugaLexer*);
void _FugaLexer_lexName   (FugaLexer*);
void _FugaLexer_lexString (FugaLexer*);
void _FugaLexer_lexSymbol (FugaLexer*);
void _FugaLexer_lexSpecial(FugaLexer*);
void _FugaLexer_lexError  (FugaLexer*);
void _FugaLexer_lexError_ (FugaLexer*, size_t);

FugaToken* FugaLexer_peek(
    FugaLexer* self
) {
    if (!self->token) {
        _FugaLexer_strip(self);
        if (!self->token) {
            self->token = FugaToken_new(self);
            self->token->filename = self->filename;
            self->token->line     = self->line;
            self->token->column   = self->column;
            _FugaLexer_lex(self);
        }
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
        while (1) {
            while ((self->code[i] != '\0') && (self->code[i] != '\n'))
                i++;
            if (self->code[i-1] != '\\' || self->code[i] != '\n')
                break;
            i++;
        }
        _FugaLexer_consume_(self, i);
        _FugaLexer_strip(self);
    } else if(self->code[i] == '\\') {
        if (self->code[i+1] == '#') {
            while ((self->code[i] != '\0') &&
                   (self->code[i] != '\n'))
                i++;
            if (self->code[i] == '\n') i++;
            _FugaLexer_consume_(self, i);
            _FugaLexer_strip(self);
        } else if (self->code[i+1] == '\n') {
            _FugaLexer_consume_(self, i+2);
            _FugaLexer_strip(self);
        } else {
            _FugaLexer_consume_(self, i);
        }
    } else {
        _FugaLexer_consume_(self, i);
    }
}

void _FugaLexer_lex(
    FugaLexer* self
) {
    if (self->code[0] == '"')
        _FugaLexer_lexString(self);
    else if (self->code[0] == ':')
        _FugaLexer_lexSymbol(self);
    else if (FugaChar_isSpecial(self->code))
        _FugaLexer_lexSpecial(self);
    else if (FugaChar_isOp(self->code))
        _FugaLexer_lexOp(self);
    else if (FugaChar_isName(self->code) || (self->code[0] == '\\'))
        _FugaLexer_lexName(self);
    else
        _FugaLexer_lexError(self);
}

void _FugaLexer_lexError_(
    FugaLexer* self,
    size_t num_chars
) {
    ALWAYS(self); ALWAYS(self->token); ALWAYS(self->code);
    if (!self->token) {
        self->token = FugaToken_new(self);
        self->token->filename = self->filename;
        self->token->line     = self->line;
        self->token->column   = self->column;
    }
    self->token->type = FUGA_TOKEN_ERROR;
    _FugaLexer_consume_(self, num_chars);
}

void _FugaLexer_lexError(
    FugaLexer* self
) {
    _FugaLexer_lexError_(self, 1);
}

void _FugaLexer_lexSpecial(
    FugaLexer* self
) {
    ALWAYS(self); ALWAYS(self->token); ALWAYS(self->code);
    
    switch(self->code[0]) {
    case '(':  self->token->type = FUGA_TOKEN_LPAREN; break;
    case ')':  self->token->type = FUGA_TOKEN_RPAREN; break;
    case '[':  self->token->type = FUGA_TOKEN_LBRACKET; break;
    case ']':  self->token->type = FUGA_TOKEN_RBRACKET; break;
    case '{':  self->token->type = FUGA_TOKEN_LCURLY; break;
    case '}':  self->token->type = FUGA_TOKEN_RCURLY; break;
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
    for (i = 0; FugaChar_isDigit(self->code+i);
              i += FugaChar_size(self->code+i)) {
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
    if (self->code[0] == '\\') {
        _FugaLexer_consume_(self, 1);
        _FugaLexer_lexOp(self);
        self->token->type = FUGA_TOKEN_NAME;
        return;
    }

    int i;
    bool digits = true;
    for (i = 0; FugaChar_isName(self->code+i);
           i += FugaChar_size  (self->code+i))
        digits = digits && FugaChar_isDigit(self->code+i);

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
    bool error;
    if (self->code[0] == '?' || self->code[0] == '!')
        error = true;
    while (FugaChar_isOp(self->code+i))
        i += FugaChar_size(self->code+i);
    self->token->type = FUGA_TOKEN_OP;
    self->token->value = _FugaLexer_prefix_(self, i);
    if (self->code[0] == '(')
        self->token->type = FUGA_TOKEN_NAME;
    if (error)
        self->token->type = FUGA_TOKEN_ERROR;
}

void _FugaLexer_lexDoc(
    FugaLexer* self
) {
    _FugaLexer_consume_(self, 2);
    if (self->code[0] == ' ')
        _FugaLexer_consume_(self, 1);

    int i=0;
    while(1) {
        while (self->code[i] != '\n' && self->code[i] != 0)
            i++;
        if (self->code[i-1] != '\\' || self->code[i] != '\n')
            break;
        i++;
    }

    char* value = malloc(i+2);
    int index=0;
    i = 0;
    while(1) {
        while (self->code[i] != '\n' && self->code[i] != 0)
            value[index++] = self->code[i++];
        if (self->code[i-1] != '\\' || self->code[i] != '\n')
            break;
        index--; i++;
    }
    value[index++] = '\n';
    value[index++] = 0;

    self->token->type  = FUGA_TOKEN_DOC;
    self->token->value = value;
    _FugaLexer_consume_(self, i);
}

void _FugaLexer_lexSymbol(
    FugaLexer* self
) {
    char c = self->code[1];
    if (c == ':') {
        _FugaLexer_lexDoc(self);
    } else if (FugaChar_isName(self->code+1) || self->code[1] == '\\') {
        _FugaLexer_consume_(self, 1);
        _FugaLexer_lexName(self);
        if (self->token->type == FUGA_TOKEN_NAME)
            self->token->type = FUGA_TOKEN_SYMBOL;
        else
            self->token->type = FUGA_TOKEN_ERROR;
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
    for (i=0; self->code[i] != '"';) {
        if (FugaChar_escapeError(self->code + i)) {
            escapeError = true;
            if (!self->code[i]) break;
        }
        length += FugaChar_sizeAfterUnescape(self->code + i);
        i      += FugaChar_sizeBeforeUnescape(self->code + i);
    }
    if (escapeError) {
        _FugaLexer_lexError_(self, i+1);
        return;
    }

    // get string value
    char* value = malloc(length+1);
    size_t index = 0;
    for (size_t j=0; j<i; ) {
        FugaChar_unescape(value + index, self->code + j);
        index += FugaChar_sizeAfterUnescape (self->code + j);
        j     += FugaChar_sizeBeforeUnescape(self->code + j);
    }
    value[length] = '\0';

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
    void* gc = Fuga_init();
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

    FugaLexer_readCode_(self, "\"Hello,\nworld!\"");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_STRING, "Hello,\nworld!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

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
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_OP, "=");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "b");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    // integers
    FugaLexer_readCode_(self, "1");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 1);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "123");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 123);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "0");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 0);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "077");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 77);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    // names
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

    FugaLexer_readCode_(self, "_ _?!");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "_");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "_?!");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "?");
    FUGA_LEXER_TEST    (FUGA_TOKEN_ERROR);

    FugaLexer_readCode_(self, "][");
    FUGA_LEXER_TEST    (FUGA_TOKEN_RBRACKET);
    FUGA_LEXER_TEST    (FUGA_TOKEN_LBRACKET);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "+(a)");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "+");
    FUGA_LEXER_TEST    (FUGA_TOKEN_LPAREN);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "a");
    FUGA_LEXER_TEST    (FUGA_TOKEN_RPAREN);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "\\+ :\\** :\\:=");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "+");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "**");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, ":=");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "a := b");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "a");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_OP,   ":=");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME, "b");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "::Hello\nthere:: Good\n:bye");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_DOC,    "Hello\n");
    FUGA_LEXER_TEST    (FUGA_TOKEN_SEPARATOR);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_NAME,   "there");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_DOC,    "Good\n");
    FUGA_LEXER_TEST    (FUGA_TOKEN_SEPARATOR);
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_SYMBOL, "bye");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, ":: Hello\\\n there");
    FUGA_LEXER_TEST_STR(FUGA_TOKEN_DOC,    "Hello there\n");
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, ":65");
    FUGA_LEXER_TEST    (FUGA_TOKEN_ERROR);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "10#20\\\n30\n40");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 10);
    FUGA_LEXER_TEST    (FUGA_TOKEN_SEPARATOR);
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 40);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    FugaLexer_readCode_(self, "10\\\n20");
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 10);
    FUGA_LEXER_TEST_INT(FUGA_TOKEN_INT, 20);
    FUGA_LEXER_TEST    (FUGA_TOKEN_END);

    Fuga_quit(gc);
}
#endif

