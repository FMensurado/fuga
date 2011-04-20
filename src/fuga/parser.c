#include "token.h"
#include "parser.h"
#include "test.h"

struct FugaParser {
    FugaLexer* lexer;
};

void FugaParser_mark(
    void *_parser
) {
    FugaParser* parser = _parser;
    Fuga_mark_(parser, parser->lexer);
}

FugaParser* FugaParser_new(
    void* self
) {
    FugaParser* parser = Fuga_clone_(FUGA->Object, sizeof(FugaParser));
    Fuga_onMark_(parser, FugaParser_mark);
    return parser;
}

void FugaParser_readCode_(
    FugaParser* parser,
    const char* code
) {
    parser->lexer = FugaLexer_new(parser);
    FugaLexer_readCode_(parser->lexer, code);
}

bool FugaParser_readFile_(
    FugaParser* parser,
    const char* filename
) {
    parser->lexer = FugaLexer_new(parser);
    return FugaLexer_readFile_(parser->lexer, filename);
}

void FugaParser_parseTokens_(
    FugaParser* parser,
    FugaLexer* lexer
) {
    parser->lexer = lexer;
}

bool FugaParser_check_(
    FugaParser* parser,
    FugaTokenType tokenType
) {
    ALWAYS(parser);
    if (tokenType == FugaLexer_peek(parser->lexer)->type) {
        return true;
    }
    return false;
}

void FugaParser_advance(
    FugaParser* parser
) {
    FugaLexer_next(parser->lexer);
}

bool FugaParser_advance_(
    FugaParser* parser,
    FugaTokenType tokenType
) {
    ALWAYS(parser);
    if (tokenType == FugaLexer_peek(parser->lexer)->type) {
        FugaLexer_next(parser->lexer);
        return true;
    }
    return false;
}

void* FugaParser_error_(
    FugaParser* self,
    const char* message
) {
    FUGA_RAISE(FUGA->SyntaxError, message);
}

void* FugaParser_unfinished_(
    FugaParser* self,
    const char* message
) {
    ALWAYS(self); ALWAYS(message);
    void* exceptionType;
    if (FugaLexer_peek(self->lexer)->type == FUGA_TOKEN_END)
        exceptionType = FUGA->SyntaxUnfinished;
    else
        exceptionType = FUGA->SyntaxError;
    FUGA_RAISE(exceptionType, message);
}

#define FUGA_PARSER_EXPECT(parser, tokenType, name)             \
    do {                                                        \
        if (!FugaParser_advance_(parser, tokenType))            \
            return FugaParser_error_(parser, "expected " name); \
    } while(0)

#define FUGA_PARSER_EXPECT_UNFINISHED(parser, tokenType, name)       \
    do {                                                             \
        if (!FugaParser_advance_(parser, tokenType))                 \
            return FugaParser_unfinished_(parser, "expected " name); \
    } while(0)


void* FugaParser_object(
    FugaParser* self
) {
    void* block;
    FUGA_PARSER_EXPECT(self, FUGA_TOKEN_LPAREN, "(");
    FUGA_CHECK(block = FugaParser_block(self));
    FUGA_PARSER_EXPECT_UNFINISHED(self, FUGA_TOKEN_RPAREN, ")");
    return block;
}

void* FugaParser_msg(
    FugaParser* self
) {
    FugaToken* token = FugaLexer_next(self->lexer);
    if (token->type != FUGA_TOKEN_NAME)
        FUGA_RAISE(FUGA->SyntaxError, "expected msg");
    FugaMsg* msg = FugaMsg_fromSymbol(FugaToken_symbol(token));
    FUGA_CHECK(msg);

    if (FugaParser_check_(self, FUGA_TOKEN_LPAREN)) {
        void* block = FugaParser_object(self);
        FUGA_CHECK(block);
        FUGA_HEADER(msg)->slots = FUGA_HEADER(block)->slots;
    }

    return msg;
}

void* FugaParser_root(
    FugaParser* self
) {
    FugaToken* token = FugaLexer_peek(self->lexer);
    void* value;

    switch (token->type) {
    case FUGA_TOKEN_LPAREN:
        return FugaParser_object(self);

    case FUGA_TOKEN_NAME:
        return FugaParser_msg(self);

    case FUGA_TOKEN_LBRACKET:
        FugaParser_advance(self);
        FUGA_CHECK(value = FugaParser_expression(self));
        FUGA_PARSER_EXPECT(self, FUGA_TOKEN_RBRACKET, "]");
        return value;

    case FUGA_TOKEN_INT:
        FugaParser_advance(self);
        return FugaToken_int(token);

    case FUGA_TOKEN_STRING:
        FugaParser_advance(self);
        return FugaToken_string(token);

    case FUGA_TOKEN_SYMBOL:
        FugaParser_advance(self);
        return FugaToken_symbol(token);

    case FUGA_TOKEN_OP:
        FugaParser_advance(self);
        void* msg = FugaMsg_fromSymbol(FugaToken_symbol(token));
        FUGA_CHECK(msg);
        FUGA_CHECK(Fuga_append_(msg, FugaParser_root(self)));
        return msg;

    default:
        FUGA_RAISE(FUGA->SyntaxError,
            "expected [, (, NAME, INT, STRING, or SYMBOL"
        );
    }

}

void* FugaParser_part(
    FugaParser* self
) {
    void* root = FugaParser_root(self);
    FUGA_CHECK(root);
    void* expr = NULL;

    while (FugaParser_check_(self, FUGA_TOKEN_NAME)) {
        if (!expr) {
            expr = Fuga_clone(FUGA->Expr);
            FUGA_CHECK(Fuga_append_(expr, root));
        }
        void* msg = FugaParser_msg(self);
        FUGA_CHECK(msg);
        FUGA_CHECK(Fuga_append_(expr, msg));
    }
    return expr ? expr : root;
}

void* FugaParser_expression(
    FugaParser* self
) {
    void* part = FugaParser_part(self);
    FUGA_CHECK(part);

    if (FugaParser_advance_(self, FUGA_TOKEN_LCURLY)) {
        void* body = FugaParser_block(self);
        FUGA_CHECK(body);
        FUGA_PARSER_EXPECT_UNFINISHED(self, FUGA_TOKEN_RCURLY, "}");
        void* method = FUGA_MSG("def");
        FUGA_CHECK(Fuga_append_(method, part));
        FUGA_CHECK(Fuga_extend_(method, body));
        return method;
    }

    while (FugaParser_check_(self, FUGA_TOKEN_OP)) {
        FugaToken *token = FugaLexer_next(self->lexer);
        void* msg = FugaMsg_fromSymbol(FugaToken_symbol(token));
        FUGA_CHECK(Fuga_append_(msg, part));
        FUGA_CHECK(Fuga_append_(msg, FugaParser_part(self)));
        part = msg;
    }

    return part;
}

void* FugaParser_slot_(
    FugaParser* self,
    void* block
) {
    void* doc = NULL;
    while (FugaParser_check_(self, FUGA_TOKEN_DOC)) {
        FugaToken *token = FugaLexer_next(self->lexer);
        FugaString *str = FugaToken_string(token);
        if (doc)
            doc = FugaString_cat_(doc, str);
        else
            doc = str;
        FugaParser_advance_(self, FUGA_TOKEN_SEPARATOR);
    }

    if (FugaParser_check_(self, FUGA_TOKEN_END)) {
        return FugaParser_unfinished_(
            self,
            "expected expression after slot documentation"
        );
    }

    FUGA_CHECK(Fuga_append_(block, FugaParser_expression(self)));
    if (doc) {
        FUGA_CHECK(Fuga_setDocI(block, -1, doc));
    }
    return FUGA->nil;
}

void* FugaParser_block(
    FugaParser* parser
) {
    void* self = parser;
    self = Fuga_clone(FUGA->Object);
    FugaToken* token;
    bool needsep = false;
    while (1) {
        if (FugaParser_advance_(parser, FUGA_TOKEN_SEPARATOR))
            needsep = false;
        while (FugaParser_advance_(parser, FUGA_TOKEN_SEPARATOR));

        token = FugaLexer_peek(parser->lexer);
        if ((token->type == FUGA_TOKEN_RPAREN) ||
            (token->type == FUGA_TOKEN_RCURLY) ||
            (token->type == FUGA_TOKEN_END))
            break;
        
        if (needsep)
            FUGA_RAISE(FUGA->SyntaxError,
                "expected separator between slots"
            );

        FUGA_CHECK(FugaParser_slot_(parser, self));
        needsep = true;
    }
    return self;
}

#ifdef TESTING

#define FUGA_PARSER_TEST(code,exp) do{          \
        FugaParser_readCode_(parser, code);     \
        void* self##__LINE__ = self;            \
        self = FugaParser_expression(parser);   \
        TEST(exp);                              \
        self = self##__LINE__;                  \
    } while(0)

TESTS(FugaParser) {
    void* self = Fuga_init();
    FugaParser* parser = FugaParser_new(self);
    
    FUGA_PARSER_TEST("",  Fuga_isRaised(self));
    FUGA_PARSER_TEST("1", FugaInt_is_(self, 1));
    FUGA_PARSER_TEST("\"Hello World!\"", Fuga_isString(self));
    FUGA_PARSER_TEST(":doremi", Fuga_isSymbol(self));
    FUGA_PARSER_TEST("doremi", Fuga_isMsg(self));
    FUGA_PARSER_TEST("()", Fuga_hasLength_(self, 0));
    FUGA_PARSER_TEST("(10)",
           Fuga_hasLength_(self, 1)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(0)), 10));
    FUGA_PARSER_TEST("(10,20)",
           Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(1)), 20));
    FUGA_PARSER_TEST("(\n\n\n10\n20\n\n:do,)",
           Fuga_hasLength_(self, 3)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(1)), 20)
        && Fuga_is_(FUGA_SYMBOL("do"), Fuga_get(self, FUGA_INT(2))));
    FUGA_PARSER_TEST("do()", Fuga_isMsg(self));
    FUGA_PARSER_TEST("do(re, mi)",
           Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2));
    FUGA_PARSER_TEST("do re",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("do re mi",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 3)
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(1)))
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(2))));
    FUGA_PARSER_TEST(":do re",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && (FUGA_SYMBOL("do") == Fuga_get(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_get(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("-42",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 1)
        && FugaInt_is_(Fuga_getI(self, 0), 42));
    FUGA_PARSER_TEST("10 = 20",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_get(self, FUGA_INT(1)), 20));
    FUGA_PARSER_TEST("10 + 20 * 30",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2)
        && Fuga_isMsg(Fuga_getI(self, 0))
        && Fuga_hasLength_(Fuga_getI(self, 0), 2));
    FUGA_PARSER_TEST("[10 + 20] * 30",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2)
        && Fuga_isMsg(Fuga_getI(self, 0))
        && Fuga_hasLength_(Fuga_getI(self, 0), 2));
    FUGA_PARSER_TEST("10 + [20 * 30]",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2)
        && Fuga_isMsg(Fuga_getI(self, 1))
        && Fuga_hasLength_(Fuga_getI(self, 1), 2));
    FUGA_PARSER_TEST("[10, 20]", Fuga_isRaised(self));

    FUGA_PARSER_TEST("foo {}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 1)
        &&  Fuga_isMsg(Fuga_get(self, FUGA_INT(0)))
        );
    FUGA_PARSER_TEST("foo {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_isMsg(Fuga_get(self, FUGA_INT(0)))
        &&  Fuga_isInt(Fuga_get(self, FUGA_INT(1)))
        );
    FUGA_PARSER_TEST("foo {10, :hi}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 3)
        &&  Fuga_isMsg(Fuga_get(self, FUGA_INT(0)))
        &&  Fuga_isInt(Fuga_get(self, FUGA_INT(1)))
        &&  Fuga_isSymbol(Fuga_get(self, FUGA_INT(2)))
        );
    FUGA_PARSER_TEST("foo bar {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_isExpr(Fuga_getI(self, 0))
        &&  Fuga_hasLength_(Fuga_getI(self, 0), 2)
        &&  Fuga_isInt (Fuga_getI(self, 1))
        );
    FUGA_PARSER_TEST("(:: foo\n 10)",
           !Fuga_isRaised(self)
        &&  Fuga_hasLength_(self, 1)
        &&  Fuga_hasDocI(self, 0)
        &&  FugaString_is_(Fuga_getDocI(self, 0), "foo\n")
        &&  FugaInt_is_(Fuga_getI(self, 0), 10));
    FUGA_PARSER_TEST("(:: foo  \n    ::bar\n 20)",
           !Fuga_isRaised(self)
        &&  Fuga_hasLength_(self, 1)
        &&  Fuga_hasDocI(self, 0)
        &&  FugaString_is_(Fuga_getDocI(self, 0), "foo  \nbar\n")
        &&  FugaInt_is_(Fuga_getI(self, 0), 20));


    Fuga_quit(self);
}
#endif

