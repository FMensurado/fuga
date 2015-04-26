
#include "log.h"

static FILE* FugaLog_file;

void FugaLog_init (void)
{
    FugaLog_file = fopen("log.txt", "w");
}

void FugaLog_quit (void)
{
    fclose(FugaLog_file);
}

void FugaLog_log0 (const char* action)
{
    fprintf(FugaLog_file, "%s\n", action);
}


void FugaLog_log1 (const char* action, void* arg1)
{
    fprintf(FugaLog_file, "%s %p\n", action, arg1);
}

void FugaLog_log2 (const char* action, void* arg1, void* arg2)
{
    fprintf(FugaLog_file, "%s %p %p\n", action, arg1, arg2);
}

void FugaLog_log3 (const char* action, void* arg1, void* arg2, void* arg3)
{
    fprintf(FugaLog_file, "%s %p %p %p\n", action, arg1, arg2, arg3);
}
