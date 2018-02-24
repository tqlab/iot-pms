//
// Created by xmx on 27/01/2018.
//

#ifndef PMS_PMSUTIL_H
#define PMS_PMSUTIL_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

uint64_t pms_current_time_millis();

void pms_current_local_time_str(char* timestr, size_t size);

uint16_t read_uint16(const uint8_t *buf, int idx);

extern const char PMS_GIT_SHA1[];
extern const char PMS_VERSION[];
extern const char PMS_COMPILE_DATETIME[];

/* LOG */

extern int use_tty;

#define USE_TTY()                        \
    do {                                 \
        use_tty = isatty(STDERR_FILENO); \
    } while (0)

#ifdef PMS_DEBUG
#define LOG_D(format, ...)                                                       \
    do {                                                                         \
            char timestr[20];                                                    \
            pms_current_local_time_str(timestr, 20);                             \
            if (isatty(STDERR_FILENO)) {                                         \
                fprintf(stdout, "%s, %s:%d, \e[01;35mDEBUG\e[0m - " format "\n", \
                        timestr, __FILE__, __LINE__,                             \
                        ## __VA_ARGS__);                                         \
            } else {                                                             \
                fprintf(stdout, "%s, DEBUG - " format "\n",                      \
                        timestr,                                                 \
                        ## __VA_ARGS__);                                         \
            }                                                                    \
            fflush(stdout);                                                      \
    }                                                                            \
    while (0)
#else
#define LOG_D(format, ...)                                                       \
    do {                                                                         \
    }                                                                            \
    while (0)
#endif

#define LOG_I(format, ...)                                                       \
    do {                                                                         \
            char timestr[20];                                                    \
            pms_current_local_time_str(timestr, 20);                             \
            if (isatty(STDERR_FILENO)) {                                         \
                fprintf(stdout, "%s, \e[01;32mINFO \e[0m - " format "\n",        \
                        timestr,                                                 \
                        ## __VA_ARGS__);                                         \
            } else {                                                             \
                fprintf(stdout, "%s, INFO  - " format "\n",                      \
                        timestr,                                                 \
                        ## __VA_ARGS__);                                         \
            }                                                                    \
            fflush(stdout);                                                      \
    }                                                                            \
    while (0)

#define LOG_E(format, ...)                                                       \
    do {                                                                         \
            char timestr[20];                                                    \
            pms_current_local_time_str(timestr, 20);                             \
            if (isatty(STDERR_FILENO)) {                                         \
                fprintf(stdout, "%s, %s:%d, \e[01;31mERROR\e[0m - " format "\n", \
                        timestr, __FILE__, __LINE__,                             \
                        ## __VA_ARGS__);                                         \
                fflush(stdout);                                                  \
            } else {                                                             \
                fprintf(stderr, "%s, ERROR - " format "\n",                      \
                        timestr,                                                 \
                        ## __VA_ARGS__);                                         \
                fflush(stderr);                                                  \
            }                                                                    \
    }                                                                            \
    while (0)


/* LOG END */


#endif //PMS_PMSUTIL_H
