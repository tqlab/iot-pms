//
// Created by xmx on 27/01/2018.
//

#ifndef PMS_PMSUTIL_H
#define PMS_PMSUTIL_H

#include <stdio.h>

long pms_current_time_millis();

void pms_current_local_time_str(char* timestr, size_t size);

uint16_t read_uint16(uint8_t *buf, int idx);


#endif //PMS_PMSUTIL_H