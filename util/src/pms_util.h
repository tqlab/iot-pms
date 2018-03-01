//
// Created by xmx on 27/01/2018.
//

#ifndef PMS_PMSUTIL_H
#define PMS_PMSUTIL_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

uint64_t pms_current_time_millis();

int pms_current_local_time_str(char *timestr, size_t size);

uint16_t read_uint16(const uint8_t *buf, int idx);

extern const char PMS_GIT_SHA1[];
extern const char PMS_VERSION[];
extern const char PMS_COMPILE_DATETIME[];

#define llu_cast(x) ((unsigned long long)(x))


/* LOG */

#ifdef PMS_DEBUG
#define LOG_D(format, ...)                                                          \
    do {                                                                            \
            char ___timestr[30];                                                    \
            pms_current_local_time_str(___timestr, 30);                             \
            if (isatty(STDERR_FILENO)) {                                            \
                fprintf(stdout, "%s, \e[01;35m[DEBUG]\e[0m <%s:%d> - " format "\n", \
                        ___timestr, __FILE__, __LINE__,                             \
                        ## __VA_ARGS__);                                            \
            } else {                                                                \
                fprintf(stdout, "%s, [DEBUG] <%s:%d> - " format "\n",               \
                        ___timestr, __FILE__, __LINE__,                             \
                        ## __VA_ARGS__);                                            \
            }                                                                       \
            fflush(stdout);                                                         \
    }                                                                               \
    while (0)
#else
#define LOG_D(format, ...)                                                          \
    do {                                                                            \
    }                                                                               \
    while (0)
#endif

#define LOG_I(format, ...)                                                          \
    do {                                                                            \
            char timestr[30];                                                       \
            pms_current_local_time_str(timestr, 30);                                \
            if (isatty(STDERR_FILENO)) {                                            \
                fprintf(stdout, "%s, \e[01;32m[INFO ]\e[0m - " format "\n",         \
                        timestr,                                                    \
                        ## __VA_ARGS__);                                            \
            } else {                                                                \
                fprintf(stdout, "%s, [INFO ] - " format "\n",                       \
                        timestr,                                                    \
                        ## __VA_ARGS__);                                            \
            }                                                                       \
            fflush(stdout);                                                         \
    }                                                                               \
    while (0)

#define LOG_E(format, ...)                                                          \
    do {                                                                            \
            char timestr[30];                                                       \
            pms_current_local_time_str(timestr, 30);                                \
            if (isatty(STDERR_FILENO)) {                                            \
                fprintf(stdout, "%s, \e[01;31m[ERROR]\e[0m <%s:%d> - " format "\n", \
                        timestr, __FILE__, __LINE__,                                \
                        ## __VA_ARGS__);                                            \
                fflush(stdout);                                                     \
            } else {                                                                \
                fprintf(stderr, "%s, [ERROR] <%s:%d> - " format "\n",               \
                        timestr, __FILE__, __LINE__,                                \
                        ## __VA_ARGS__);                                            \
                fflush(stderr);                                                     \
            }                                                                       \
    }                                                                               \
    while (0)

#define FLOG_I(file, format, ...)                                                   \
    do {                                                                            \
            char timestr[30];                                                       \
            pms_current_local_time_str(timestr, 30);                                \
            fprintf(file, "%s, [INFO ] - " format "\n",                             \
                timestr,                                                            \
                ## __VA_ARGS__);                                                    \
            fflush(file);                                                           \
    }                                                                               \
    while (0)

/* LOG END */


#endif //PMS_PMSUTIL_H
