//
// Created by xmx on 27/01/2018.
//

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>



uint64_t pms_current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv,NULL);

    uint64_t sec = tv.tv_sec;
    uint64_t usec = tv.tv_usec;

    return sec * 1000 + usec / 1000;
}

void pms_current_local_time_str(char* timestr, size_t size)
{
    memset(timestr, 0, size);

    struct timeval    tv;
    struct timezone   tz;

    gettimeofday(&tv, &tz);

    /*
    printf("tv_sec:%ld\n",tv.tv_sec);
    printf("tv_usec:%ld\n",tv.tv_usec);
    printf("tz_minuteswest:%d\n",tz.tz_minuteswest);
    printf("tz_dsttime:%d\n",tz.tz_dsttime);
    */

    struct tm *p = localtime(&tv.tv_sec);
    sprintf(timestr, "%4d-%02d-%02d %02d:%02d:%02d.%03d",
            1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec/1000);
}

uint16_t read_uint16(uint8_t *buf, int idx) {
    uint16_t data;
    data = buf[idx] << 8;
    data += buf[idx + 1];
    return data;
}

