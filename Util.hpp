#ifndef UTIL_H
#define UTIL_H
#include <stdarg.h>
#include<cstdio>

#define LOGPRIORITY 1
using namespace std;
enum priority {
    PRIO_NONE,
    PRIO_LOG,
    PRIO_ALL
};

int _PRI = 0;
void write_log(int priority, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if(priority >= LOGPRIORITY)
    vprintf(format, args);
    va_end(args);
}

#endif
