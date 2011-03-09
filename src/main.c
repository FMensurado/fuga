#include "fuga/fuga.h"
#include "fuga/parser.h"

#include <stdio.h>
#include <string.h>

#define PROMPT1 ">>> "
#define PROMPT2 "... "

Fuga* read(
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

Fuga* evalPrint(
    Fuga* self,
    Fuga* block
) {
    FUGA_CHECK(block);
    long length = FugaInt_value(Fuga_numSlots(block));
    for (long i = 0; i < length; i++) {
        Fuga* slot   = Fuga_getSlot(block, FUGA_INT(i));
        Fuga* value  = Fuga_eval(slot, self, self);
        if (Fuga_isNil(value))
            continue;
        FUGA_CHECK(Fuga_print(value));
    }
    return NULL;
}

void repl()
{
    Fuga *self = Fuga_init();
    FugaParser *parser = FugaParser_new(self);
    
    self = Fuga_clone(FUGA->Prelude);
    FugaGC_root(self);
    FugaGC_root(parser);
    Fuga_setSlot(self, FUGA_SYMBOL("this"), self);

    printf("Fuga 0.1. Use \"quit\" to quit.\n");
    while (1) {
        Fuga* block = read(parser);
        if (!block) break;
        Fuga* error = evalPrint(self, block);
        if (error) Fuga_printException(error);
        FugaGC_collect(self);
    }

    Fuga_quit(self);
}

Fuga* runBlock(
    Fuga* self
) {
    FUGA_CHECK(self);
    Fuga* block = self;
    self = Fuga_clone(FUGA->Prelude);
    Fuga* scope = Fuga_clone(self);
    FUGA_CHECK(Fuga_setSlot(scope, FUGA_SYMBOL("this"), self));

    long numSlots = FugaInt_value(Fuga_numSlots(block));
    for (long i = 0; i < numSlots; i++) {
        Fuga* slot = Fuga_getSlot(block, FUGA_INT(i));
        FUGA_CHECK(slot);
        Fuga* result = Fuga_eval(slot, scope, scope);
        FUGA_CHECK(result);
        FUGA_CHECK(Fuga_print(result));
    }
    return FUGA->nil;
}

Fuga* loadFile(
    Fuga* self,
    const char* filename
) {
    FugaParser *parser = FugaParser_new(self);
    if (!FugaParser_readFile_(parser, filename)) 
        FUGA_RAISE(FUGA->IOError, "can't load module");
    Fuga* block = FugaParser_block(parser);
    // FIXME: ensure EOF
    FUGA_CHECK(runBlock(block));
    return FUGA->nil;
}

void runFile(
    const char* filename
) {
    Fuga* self   = Fuga_init();
    Fuga* result = loadFile(self, filename);
    Fuga* error  = Fuga_catch(result);
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

