#include "fuga/fuga.h"
#include "fuga/parser.h"

#include <stdio.h>
#include <string.h>

#define PROMPT1 ">>> "
#define PROMPT2 "... "

void* read(
    FugaParser* parser
) {
    char buffer[1024];
    printf(PROMPT1);
    fflush(stdout);
    fgets(buffer, 1024, stdin);
    if (feof(stdin)) {
        printf("quit\n");
        return NULL;
    }
    if (strcmp(buffer, "quit\n") == 0) {
        return NULL;
    }
    FugaParser_readCode_(parser, buffer);
    return FugaParser_block(parser);
}

void* evalPrint(
    void* self,
    void* block
) {
    FUGA_CHECK(block);
    long length = FugaInt_value(Fuga_length(block));
    for (long i = 0; i < length; i++) {
        void* slot   = Fuga_getI(block, i);
        void* value  = Fuga_eval(slot, self, self);
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
    Fuga_setS(self, "this", self);

    printf("Fuga 0.1. Use \"quit\" to quit.\n");
    while (1) {
        void* block = read(parser);
        if (!block) break;
        void* error = evalPrint(self, block);
        if (error) Fuga_printException(error);
        Fuga_collect(self);
    }

    Fuga_quit(self);
}

void* runBlock(
    void* self
) {
    FUGA_CHECK(self);
    void* block = self;
    self = Fuga_clone(FUGA->Prelude);
    void* scope = Fuga_clone(self);
    FUGA_CHECK(Fuga_setS(scope, "this", self));

    long numSlots = FugaInt_value(Fuga_length(block));
    for (long i = 0; i < numSlots; i++) {
        void* slot = Fuga_getI(block, i);
        FUGA_CHECK(slot);
        void* result = Fuga_eval(slot, scope, scope);
        FUGA_CHECK(result);
        FUGA_CHECK(Fuga_print(result));
    }
    return FUGA->nil;
}

void* loadFile(
    void* self,
    const char* filename
) {
    FugaParser *parser = FugaParser_new(self);
    if (!FugaParser_readFile_(parser, filename)) 
        FUGA_RAISE(FUGA->IOError, "can't load module");
    void* block = FugaParser_block(parser);
    // FIXME: ensure EOF
    FUGA_CHECK(runBlock(block));
    return FUGA->nil;
}

void runFile(
    const char* filename
) {
    void* self   = Fuga_init();
    void* result = loadFile(self, filename);
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

