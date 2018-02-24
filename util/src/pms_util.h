//
// Created by xmx on 27/01/2018.
//

#ifndef PMS_PMSUTIL_H
#define PMS_PMSUTIL_H

#include <stdio.h>
#include <stdint.h>

uint64_t pms_current_time_millis();

void pms_current_local_time_str(char* timestr, size_t size);

uint16_t read_uint16(const uint8_t *buf, int idx);

extern const char PMS_GIT_SHA1[];
extern const char PMS_VERSION[];

/*
#define LOG(format, arguments...) \
    printf(format, ##arguments);    \
    printf("\n");
*/


#endif //PMS_PMSUTIL_H
