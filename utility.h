#ifndef UTILITY_H
#define UTILITY_H
#include <sys/time.h>
#include <time.h>

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv)
{
    //char buffer[30];
    //time_t curtime;
    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    //curtime = tv->tv_sec;
    //strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    //printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

void timeval_print_s(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;
    struct tm *loctime;
    //printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    loctime = localtime(&curtime);
    strftime(buffer, 30, "%m-%d-%Y  %T", loctime);
    printf("%s.%06ld\n", buffer, tv->tv_usec);
}
#endif
