
#ifndef FUGA_LOG_H
#define FUGA_LOG_H

#include <stdio.h>

void FugaLog_init(void);
void FugaLog_quit(void);

void FugaLog_log0(const char* action);
void FugaLog_log1(const char* action, void* arg1);
void FugaLog_log2(const char* action, void* arg1, void* arg2);
void FugaLog_log3(const char* action, void* arg1, void* arg2, void* arg3);

#endif

