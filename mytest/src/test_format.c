#include <inttypes.h>
#include <stdio.h>

#define MYLOG(format, ...)                                                          \
    do {                                                                            \
        fprintf(stdout, "[DEBUG] <%s:%d> - " format "\n",                           \
                __FILE__, __LINE__,                                                 \
                ## __VA_ARGS__);                                                    \
    }while (0)

void test_format() {

    uint64_t timestamp = 1519895847566;

    MYLOG("%"PRIu64, timestamp);

    //printf("vvvvv %" PRIu64 " aaaa\n", timestamp);

    MYLOG("%s", PRIu64);




}
