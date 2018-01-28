//
// Created by lijun on 28/1/2018.
//
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "pmsutil.h"

void main(){


    struct timeval tv;
    gettimeofday(&tv,NULL);

    uint64_t sec = tv.tv_sec;
    uint64_t usec = tv.tv_usec;

    printf("%"PRIu64"\n", sec);
    printf("%"PRIu64"\n", usec);

    uint64_t timestamp = sec * 1000 + usec / 1000;

    printf("%"PRIu64"\n", timestamp);
    printf("%"PRId64"\n", timestamp);

    printf("%lld\n", timestamp);

    printf("%"PRIu64"\n", pms_current_time_millis());

    uint64_t tt = pms_current_time_millis();
    printf("%"PRIu64"\n", tt);

}