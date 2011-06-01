#include "fuga/fuga.h"
#include "fuga/parser.h"

#include <stdio.h>
#include <string.h>

#define PROMPT1 ">>> "
#define PROMPT2 "... "

void* readAux(
    FugaParser* parser,
    FugaString* prefix
) {
    void* self = parser;
    char buffer[1024];
    fflush(stdout);
    if (buffer != fgets(buffer, 1024, stdin)) {
        if (feof(stdin)) {
            printf("quit\n");
            return NULL;
        }
        FUGA_RAISE(FUGA->IOError, "Error reading file.");
    }
    if (strcmp(buffer, "quit\n") == 0) {
        return NULL;
    }
    FugaString* code = FUGA_STRING(buffer);
    code = FugaString_cat_(prefix, code);
    FugaParser_readCode_(parser, code->data);
    void* result = FugaParser_block(parser);
    FUGA_TRY(result) {
        FUGA_CATCH(FUGA->SyntaxUnfinished) {
            printf(PROMPT2);
            return readAux(parser, code);
        }
        FUGA_RERAISE;
    }
    return result;
}

void* read(
    FugaParser* self
) {
    printf(PROMPT1);
    return readAux(self, FUGA_STRING(""));
}

void* evalPrint(
    void* self,
    void* block
) {
    FUGA_CHECK(block);
    FUGA_FOR(i, slot, block) {
        if (Fuga_hasDocI(block, i))
            Fuga_setS(self, "_doc", Fuga_getDocI(block, i));
        void* value  = Fuga_eval(slot, self, self);
        Fuga_delS(self, "_doc");
        if (Fuga_isNil(value))
            continue;
        FUGA_CHECK(Fuga_print(value));
    }
    return NULL;
}

void repl()
{
    void* self = Fuga_init();
    FugaParser *parser = FugaParser_new(self);
    
    self = Fuga_clone(FUGA->Prelude);
    Fuga_root(self);
    Fuga_root(parser);
    Fuga_setS(self, "_this", self);

    printf("Fuga 0.0b. Use \"quit\" to quit.\n");
    while (1) {
        void* block = read(parser);
        if (!block) break;
        void* error = evalPrint(self, block);
        if (error) Fuga_printException(error);
        Fuga_collect(self);
    }

    Fuga_quit(self);
}

void runFile(
    const char* filename
) {
    void* self   = Fuga_init();
    void* result = Fuga_load_(self, filename);
    void* error  = Fuga_catch(result);
    if (error)
        Fuga_printException(error);
}

int main(int argc, char** argv)
{
    if (argc == 2) {
        runFile(argv[1]);
    } else {
        repl();
    }
    return 0;
}

