#include <inttypes.h>
#include <stdio.h>

#define llu_cast(x) ((unsigned long long)(x))
#define lli_cast(x) ((long long)(x))

#define MYLOG(format, ...)                                                          \
    do {                                                                            \
        fprintf(stdout, "[DEBUG] <%s:%d> - " format "\n",                           \
                __FILE__, __LINE__,                                                 \
                ## __VA_ARGS__);                                                    \
    }while (0)

void test_format() {

    uint64_t timestamp = 1519895847566;

    MYLOG("%llu", timestamp);
    printf("%lli\n", lli_cast(timestamp));


    //printf("vvvvv %" PRIu64 " aaaa\n", timestamp);

    MYLOG("%s", PRIuMAX);

    MYLOG("char: %lu", sizeof(char));

    MYLOG("short: %lu", sizeof(short));

    MYLOG("int: %lu", sizeof(int));

    MYLOG("long: %lu", sizeof(long));

    MYLOG("long long: %lu", sizeof(long long));

    MYLOG("long long unsigned: %lu", sizeof(long long unsigned));

    MYLOG("uint64_t: %lu", sizeof(uint64_t));

}
