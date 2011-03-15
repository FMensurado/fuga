#include "token.h"
#include "parser.h"
#include "test.h"

struct FugaParser {
    void* operators;
    FugaLexer* lexer;
};

void FugaParser_mark(
    void *_parser
) {
    FugaParser* parser = _parser;
    Fuga_mark_(parser, parser->operators);
    Fuga_mark_(parser, parser->lexer);
}

FugaParser* FugaParser_new(
    void* self
) {
    FugaParser* parser = Fuga_clone_(FUGA->Object, sizeof(FugaParser));
    Fuga_onMark_(parser, FugaParser_mark);
    parser->operators = Fuga_clone(FUGA->Object);

    // relational: 500s
    FugaParser_infix_precedence_(parser, "==", 500);
    FugaParser_infix_precedence_(parser, "<=", 500);
    FugaParser_infix_precedence_(parser, ">=", 500);
    FugaParser_infix_precedence_(parser, "<",  500);
    FugaParser_infix_precedence_(parser, ">",  500);
    // collections: 600s
    FugaParser_infix_precedence_(parser, "++", 600);
    // default: 1000s
    // arithmetic: 1500s
    FugaParser_infix_precedence_(parser, "+",  1500);
    FugaParser_infix_precedence_(parser, "-",  1500);
    FugaParser_infix_precedence_(parser, "*",  1510);
    FugaParser_infix_precedence_(parser, "/",  1510);
    FugaParser_infix_precedence_(parser, "//", 1510);
    FugaParser_infix_precedence_(parser, "%",  1510);
    FugaParser_infix_precedence_(parser, "**", 1520);

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

void* FugaParser_operators(
    FugaParser* self
) {
    return self->operators;
}

void FugaParser_infix_precedence_(
    FugaParser* parser,
    const char* op,
    size_t precedence
) {
    void* self = parser->operators;
    void* opSymbol = FUGA_SYMBOL(op);
    void* opData;
    void* precedenceSymbol = FUGA_SYMBOL("precedence");
    if (Fuga_hasBy_(self, opSymbol)) {
        opData = Fuga_getBy_(self, opSymbol);
        opData = Fuga_need(opData);
        if (!Fuga_isRaised(opData)) {
            Fuga_setBy_to_(opData, precedenceSymbol, FUGA_INT(precedence));
            return;
        }
    } 
    opData = Fuga_clone(FUGA->Object);
    Fuga_setBy_to_(opData, precedenceSymbol, FUGA_INT(precedence));
    Fuga_setBy_to_(self, opSymbol, opData);
}

size_t FugaParser_precedence_(
    FugaParser* parser,
    const char* op
) {
    void* self = parser->operators;
    void* opdata = Fuga_getBy_(self, FUGA_SYMBOL(op));
    if (!Fuga_isRaised(opdata)) {
        void* precedence = Fuga_getBy_(opdata, FUGA_SYMBOL("precedence"));
        precedence = Fuga_need(precedence);
        if (Fuga_isInt(precedence)) {
            long p = FugaInt_value(precedence);
            if (p > 1999) p = 1999;
            if (p < 2)    p = 2;
            return p;
        }
    }
    return 1000; // default precedence
}

/**
*** ### _FugaParser_lbp_
*** 
*** Get the left binding power of a given token (within a parser
*** context). This tells us which kinds of expressions the token can be
*** a part of.
**/
size_t _FugaParser_lbp_(
    FugaParser* parser,
    FugaToken* token
) {
    switch (token->type) {
    case FUGA_TOKEN_ERROR:    return 9999;
    case FUGA_TOKEN_LPAREN:   return 2000;
    case FUGA_TOKEN_LBRACKET: return 2000;
    case FUGA_TOKEN_LCURLY:   return 2000;
    case FUGA_TOKEN_INT:      return 2000;
    case FUGA_TOKEN_STRING:   return 2000;
    case FUGA_TOKEN_SYMBOL:   return 2000;
    case FUGA_TOKEN_NAME:     return 2000;
    case FUGA_TOKEN_EQUALS:   return 1;
    case FUGA_TOKEN_OP:
        return FugaParser_precedence_(parser, token->value);
    default: return 0;
    }
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
    FugaParser* parser,
    const char* message
) {
    ALWAYS(parser); ALWAYS(message);
    void* self = parser->operators;
    ALWAYS(self);
    FUGA_RAISE(FUGA->SyntaxError, message);
}

#define FUGA_PARSER_EXPECT(parser, tokenType, name)             \
    do {                                                        \
        if (!FugaParser_advance_(parser, tokenType))            \
            return FugaParser_error_(parser, "expected " name); \
    } while(0)

void* _FugaParser_buildExpr_(
    void* self,
    void* msg
) {
    FUGA_CHECK(self);
    FUGA_CHECK(msg);
    if (Fuga_isExpr(self)) {
        FUGA_CHECK(Fuga_append_(self, msg));
        return self;
    } else { 
        void* expr = Fuga_clone(FUGA->Expr);
        FUGA_CHECK(Fuga_append_(expr, self));
        FUGA_CHECK(Fuga_append_(expr, msg));
        return expr;
    }
}

void* _FugaParser_buildMethodBody(
    void* self
) {
    FUGA_NEED(self);
    if(Fuga_hasLength_(self, 1)) {
        return Fuga_getBy_(self, FUGA_INT(0));
    } else {
        void* body = FUGA_MSG("do");
        FUGA_HEADER(body)->slots = FUGA_HEADER(self)->slots;
        return body;
    }
}

void* _FugaParser_buildMethod_(
    void* prev,
    void* block
) {
    void* self = block;
    FUGA_CHECK(self);

    void* body = _FugaParser_buildMethodBody(block);
    void* args = NULL;


    // separate args from prev.
    if (prev) {
        FUGA_CHECK(prev);
        if (Fuga_isExpr(prev)) {
            size_t last = FugaInt_value(Fuga_length(prev)) - 1;
            void* lastF = FUGA_INT(last);
            void*  msg  = Fuga_getBy_(prev, lastF);
            args = FugaMsg_args(msg);
            void* name = FugaMsg_fromSymbol(FugaMsg_name(msg));
            FUGA_CHECK(Fuga_setBy_to_(prev, lastF, name));
        } else if (Fuga_isMsg(prev)) {
            args = FugaMsg_args(prev);
            prev = FugaMsg_fromSymbol(FugaMsg_name(prev));
        } else {
            args = prev;
            prev = NULL;
        }
    } else {
        args = Fuga_clone(FUGA->Object);
    }

    void* rhs = FUGA_MSG("method");
    FUGA_CHECK(Fuga_append_(rhs, args));
    FUGA_CHECK(Fuga_append_(rhs, body));

    if (prev) {
        void* expr = FUGA_MSG("=");
        FUGA_CHECK(Fuga_append_(expr, prev));
        FUGA_CHECK(Fuga_append_(expr, rhs));
        return expr;
    } else {
        return rhs;
    }
}

/**
*** ### _FugaParser_derive_
*** 
*** Derive an expression or a value that begins with the given token.
**/
void* _FugaParser_derive_(
    FugaParser* parser,
    FugaToken* token
) {
    // FIXME: use more descriptive error messages.
    void* self = parser->operators;
    switch (token->type) {
    case FUGA_TOKEN_ERROR:
        FUGA_RAISE(FUGA->SyntaxError, "invalid token");
    
    case FUGA_TOKEN_LPAREN:
        self = FugaParser_block(parser);
        FUGA_CHECK(self);
        FUGA_PARSER_EXPECT(parser, FUGA_TOKEN_RPAREN, ")");
        return self;

    case FUGA_TOKEN_LBRACKET:
        self = FugaParser_expression(parser);
        FUGA_CHECK(self);
        FUGA_PARSER_EXPECT(parser, FUGA_TOKEN_RBRACKET, "]");
        return self;

    case FUGA_TOKEN_LCURLY:
        self = FugaParser_block(parser);
        FUGA_CHECK(self);
        FUGA_PARSER_EXPECT(parser, FUGA_TOKEN_RCURLY, "}");
        return _FugaParser_buildMethod_(NULL, self);

    case FUGA_TOKEN_INT:    return FugaToken_int(token);
    case FUGA_TOKEN_STRING: return FugaToken_string(token);
    case FUGA_TOKEN_SYMBOL: return FugaToken_symbol(token);

    case FUGA_TOKEN_NAME:
        self = FugaMsg_fromSymbol(FugaToken_symbol(token));
        FUGA_CHECK(self);
        if (FugaParser_advance_(parser, FUGA_TOKEN_LPAREN)) {
            void* slots = FugaParser_block(parser);
            FUGA_CHECK(slots);
            FUGA_PARSER_EXPECT(parser, FUGA_TOKEN_RPAREN, ")");
            FUGA_HEADER(self)->slots = FUGA_HEADER(slots)->slots;
        }
        return self;

    case FUGA_TOKEN_OP:
        self = FugaMsg_fromSymbol(FugaToken_symbol(token));
        FUGA_CHECK(self);
        void* part = FugaParser_expression_(parser, 2000);
        return _FugaParser_buildExpr_(part, self);

    default:
        FUGA_RAISE(FUGA->SyntaxError, "invalid syntax");
    }

}

/**
*** ### _FugaParser_derive_after_
*** 
*** Derive the continuation of an expression or value.
**/
void* _FugaParser_derive_after_(
    FugaParser* parser,
    FugaToken* token,
    void* expr
) {
    void* self = parser->operators;
    void* name;
    void* arg;
    void* msg;
    switch (token->type) {
    case FUGA_TOKEN_ERROR:
        FUGA_RAISE(FUGA->SyntaxError, "invalid token");
    
    case FUGA_TOKEN_NAME:
        self = _FugaParser_derive_(parser, token);
        return _FugaParser_buildExpr_(expr, self);

    case FUGA_TOKEN_EQUALS:
        msg = FUGA_MSG("=");
        Fuga_append_(msg, expr);
        Fuga_append_(msg, FugaParser_expression_(parser, 1));
        return msg;

    case FUGA_TOKEN_OP:
        name = FugaToken_symbol(token);
        arg  = FugaParser_expression_(parser,
            FugaParser_precedence_(parser, token->value));
        msg  = FugaMsg_fromSymbol(name);
        Fuga_append_(msg, arg);
        return _FugaParser_buildExpr_(expr, msg);

    case FUGA_TOKEN_LCURLY:
        self = FugaParser_block(parser);
        FUGA_CHECK(self);
        FUGA_PARSER_EXPECT(parser, FUGA_TOKEN_RCURLY, "}");
        return _FugaParser_buildMethod_(expr, self);

    default:
        FUGA_RAISE(FUGA->SyntaxError, "invalid syntax");
    }
}

/**
*** ### FugaParser_expression_
*** 
*** Get the next expression in the stream.
*** 
*** - Params:
***     - FugaParser* parser
***     - size_t rbp: the right binding power. Determines which kinds of
***     tokens we consider to be part of our expression.
*** - Return: the expression as a Fuga object.
**/
void* FugaParser_expression_(
    FugaParser* parser,
    size_t rbp
) {
    ALWAYS(parser);
    ALWAYS(parser->lexer);

    FugaToken* token = FugaLexer_next(parser->lexer);
    void* self = _FugaParser_derive_(parser, token);
    FUGA_CHECK(self);
    token = FugaLexer_peek(parser->lexer);
    while (rbp < _FugaParser_lbp_(parser, token)) {
        token = FugaLexer_next(parser->lexer);
        self = _FugaParser_derive_after_(parser, token, self);
        FUGA_CHECK(self);
        token = FugaLexer_peek(parser->lexer);
    }
    if (token->type == FUGA_TOKEN_ERROR)
        FUGA_RAISE(FUGA->SyntaxError, "invalid token");
    return self;
}

void* FugaParser_expression(
    FugaParser* parser
) {
    return FugaParser_expression_(parser, 0);
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

        void* slot = FugaParser_expression(parser);
        FUGA_CHECK(slot);
        Fuga_append_(self, slot);
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
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10));
    FUGA_PARSER_TEST("(10,20)",
           Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 20));
    FUGA_PARSER_TEST("(\n\n\n10\n20\n\n:do,)",
           Fuga_hasLength_(self, 3)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 20)
        && Fuga_is_(FUGA_SYMBOL("do"), Fuga_getBy_(self, FUGA_INT(2))));
    FUGA_PARSER_TEST("do()", Fuga_isMsg(self));
    FUGA_PARSER_TEST("do(re, mi)",
           Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2));
    FUGA_PARSER_TEST("do re",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("do re mi",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 3)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(2))));
    FUGA_PARSER_TEST(":do re",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && (FUGA_SYMBOL("do") == Fuga_getBy_(self, FUGA_INT(0)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("-42",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 42)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("10 + 20",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("10 = 20",
           !Fuga_isRaised(self)
        && Fuga_isMsg(self)
        && Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 20));
    FUGA_PARSER_TEST("10 * 20 * 30",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 3)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(2))));
    FUGA_PARSER_TEST("10 + 20 * 30",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 2)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("[10 + 20] * 30",
           !Fuga_isRaised(self)
        && Fuga_isExpr(self)
        && Fuga_hasLength_(self, 3)
        && FugaInt_is_(Fuga_getBy_(self, FUGA_INT(0)), 10)
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        && Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1))));
    FUGA_PARSER_TEST("[10, 20]", Fuga_isRaised(self));

    FUGA_PARSER_TEST("{10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 0)
        &&  FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 10));
    FUGA_PARSER_TEST("{}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 0)
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(1)), 0));
    FUGA_PARSER_TEST("{10, 20}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 0)
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(1)), 2));
    FUGA_PARSER_TEST("() {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 0)
        &&  FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 10));
    FUGA_PARSER_TEST("(a) {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 1)
        &&  FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 10));
    FUGA_PARSER_TEST("(a, b) {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 2)
        &&  FugaInt_is_(Fuga_getBy_(self, FUGA_INT(1)), 10));
    FUGA_PARSER_TEST("foo {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(0)))
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        &&  Fuga_hasLength_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(0)),
                0
            )
        &&  FugaInt_is_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(1)),
                10
            )
        );
    FUGA_PARSER_TEST("foo bar {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_isExpr(Fuga_getBy_(self, FUGA_INT(0)))
        &&  Fuga_hasLength_(Fuga_getBy_(self, FUGA_INT(0)), 2)
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        &&  Fuga_hasLength_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(0)),
                0
            )
        &&  FugaInt_is_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(1)),
                10
            )
        );
    FUGA_PARSER_TEST("foo (bar, baz) {10}",
           !Fuga_isRaised(self)
        &&  Fuga_isMsg(self)
        &&  Fuga_hasLength_(self, 2)
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(0)))
        &&  Fuga_isMsg(Fuga_getBy_(self, FUGA_INT(1)))
        &&  Fuga_hasLength_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(0)),
                2
            )
        &&  FugaInt_is_(
                Fuga_getBy_(Fuga_getBy_(self, FUGA_INT(1)),
                             FUGA_INT(1)),
                10
            )
        );

    Fuga_quit(self);
}
#endif

