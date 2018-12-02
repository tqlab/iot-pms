#include <inttypes.h>
#include <time.h>
#include "pms_util.h"


#define TEST_STR "123456"

void test_time() {
    char time_str[40];

    pms_current_local_time_str(time_str, 40);

    printf("printf1: %s\n", time_str);

    LOG_D("%s", time_str);

    printf("printf2: %s\n", time_str);
    LOG_I("%s", time_str);

    printf("printf3: %s\n", time_str);
    LOG_E("%s", time_str);

    printf("printf4: %s\n", time_str);

    LOG_D("%s", TEST_STR);

#ifdef DEBUG
    LOG_E("%s", "test");
#endif

}