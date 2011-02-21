#include "fuga/fuga.h"
#include "fuga/parser.h"

#include <stdio.h>
#include <string.h>

#define PROMPT1 ">>> "
#define PROMPT2 "... "

Fuga* read(FugaParser* parser)
{
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

Fuga* evalPrint(Fuga* self, Fuga* block)
{
    FUGA_CHECK(block);
    long length = FugaInt_value(Fuga_numSlots(block));
    for (long i = 0; i < length; i++) {
        Fuga* slot  = Fuga_getSlot(block, FUGA_INT(i));
        Fuga* value = Fuga_eval(slot, self, self);
        if (Fuga_isNil(value))
            continue;
        FUGA_CHECK(Fuga_print(value));
    }
    return NULL;
}

void repl(void)
{
    Fuga *self = Fuga_init();
    FugaParser *parser = FugaParser_new(self);
    
    self = Fuga_clone(FUGA->Prelude);
    FugaGC_root(self);
    FugaGC_root(parser);

    Fuga_setSlot(self, FUGA_SYMBOL("__this__"), self);

    printf("Fuga version 0.1\n");
    while (1) {
        Fuga* block = read(parser);
        if (!block) break;
        Fuga* error = evalPrint(self, block);
        if (error) {
            error = Fuga_catch(error);
            Fuga_printException(error);
        }
        FugaGC_collect(self);
    }

    Fuga_quit(self);
}

int main(int argc, char** argv)
{
    repl();
    return 0;
}

