#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "pms_util.h"

void test_long_int() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint64_t sec = tv.tv_sec;
    uint64_t usec = tv.tv_usec;

    LOG_I("1. %llu", llu_cast(sec));
    LOG_I("2. %llu", llu_cast(usec));

    uint64_t timestamp = sec * 1000 + usec / 1000;

    LOG_I("3. %llu", llu_cast(timestamp));
    LOG_I("4. %lld", lld_cast(timestamp));

    LOG_I("5. %lld", timestamp);

    uint64_t timestamp_2 = 0;

    LOG_I("6. %llu", llu_cast(pms_current_time_millis(&timestamp_2)));

    LOG_I("7. %llu", llu_cast(timestamp_2));

    uint64_t tt = pms_current_time_millis(&timestamp_2);
    LOG_I("8. %llu", llu_cast(tt));

    LOG_I("9. %.1f", (double) 123 / 10);

    LOG_I("10. %lld", 1524651072231);
}