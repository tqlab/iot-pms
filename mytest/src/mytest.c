//
// Created by lijun on 28/1/2018.
//
#include <inttypes.h>
#include "pmsutil.h"

void main(){

    printf("%"PRIu64"\n", pms_current_time_millis());
    printf("%"PRId64"\n", pms_current_time_millis());

    printf("%lld\n", pms_current_time_millis());
}