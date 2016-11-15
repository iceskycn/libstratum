//
// Created by Gregory Istratov on 11/15/16.
//

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/syscall.h>
#include "log.h"

pthread_spinlock_t stderr_spinlock;
pthread_spinlock_t stdout_spinlock;


int loglevel = LOGLEVEL_DEBUG;

void init_log(int loglvl)
{
    loglevel = loglvl;
    pthread_spin_init(&stderr_spinlock, 0);
    pthread_spin_init(&stdout_spinlock, 0);
}

static __always_inline const char* loglevel_s(int lvl)
{
    switch(lvl)
    {
        case LOG_ERROR:
            return "ERROR";
        case LOG_DEBUG:
            return "DEBUG";
        case LOG_INFO:
            return "INFO ";
        default:
            return "UNKNOWN";
    }
}

void _log(const char* msg, const char* file, const char* fun, int line, int lvl)
{
    if(lvl <= loglevel)
    {
        pid_t tid = syscall(__NR_gettid);

        if(lvl == LOG_ERROR)
        {
            pthread_spin_lock(&stderr_spinlock);

            time_t t = time(NULL);
            struct tm* tml = localtime(&t);

            fprintf(stderr, "[%02d/%02d/%d - %02d:%02d:%02d][0x%08x][%s][%s]: %s - %s:%d\n",
                    tml->tm_mday, tml->tm_mon+1, tml->tm_year-100,
                    tml->tm_hour, tml->tm_min, tml->tm_sec,
                    tid, loglevel_s(lvl),fun, msg, file, line
            );
            fflush(stderr);

            pthread_spin_unlock(&stderr_spinlock);
        }
        else
        {
            pthread_spin_lock(&stdout_spinlock);

            time_t t = time(NULL);
            struct tm* tml = localtime(&t);

            fprintf(stdout, "[%02d/%02d/%d - %02d:%02d:%02d][0x%08x][%s][%s]: %s - %s:%d\n",
                    tml->tm_mday, tml->tm_mon+1, tml->tm_year-100,
                    tml->tm_hour, tml->tm_min, tml->tm_sec,
                    tid, loglevel_s(lvl), fun, msg, file, line
            );

            fflush(stdout);

            pthread_spin_unlock(&stdout_spinlock);
        }
    }
}
