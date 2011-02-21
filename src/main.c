#include "fuga/fuga.h"
#include "fuga/parser.h"

#include <stdio.h>
#include <string.h>

#define PROMPT1 ">>> "
#define PROMPT2 "... "

void repl(void)
{
    Fuga *self = Fuga_init();
    FugaParser *parser = FugaParser_new(self);
    char buffer[1024];
    
    self = Fuga_clone(FUGA->Prelude);
    FugaGC_root(self);
    FugaGC_root(parser);

    printf("Fuga interpreter.\n");
    while (1) {
        printf(PROMPT1);
        fflush(stdout);
        fgets(buffer, 1024, stdin);
        if (feof(stdin)) {
            printf("quit\n");
            break;
        }
        if (strcmp(buffer, "quit\n") == 0)
            break;
        FugaParser_readCode_(parser, buffer);
        Fuga* block = FugaParser_block(parser);
        Fuga* error;
        if ((error = Fuga_catch(block))) {
            printf("Syntax Error.\n");
            Fuga_printException(error);
            continue;
        }
        Fuga* value = Fuga_eval(block, self, self);
        if ((error = Fuga_catch(value))) {
            printf("Exception raised while evaluating.\n");
            Fuga_printException(error);
            continue;
        }
        Fuga* string = Fuga_str(value);
        if ((error = Fuga_catch(string))) {
            printf("Exception raised while calling str.\n");
            Fuga_printException(error);
            continue;
        }
        if (!Fuga_isString(string)) {
            printf("Error: str did not return a string.\n");
            Fuga_printException(error);
            continue;
        }
        FugaString_print(string);
        FugaGC_collect(self);
    }


    Fuga_quit(self);
}

int main(int argc, char** argv)
{
    repl();
    return 0;
}

