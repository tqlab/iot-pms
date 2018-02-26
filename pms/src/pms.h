//
// Created by lijun on 26/1/2018.
//

#ifndef PMS_PMS_H
#define PMS_PMS_H

#include <stdbool.h>

// parsed measurement data
typedef struct {
    uint16_t conc_pm1_0_cf1;
    uint16_t conc_pm2_5_cf1;
    uint16_t conc_pm10_0_cf1;
    uint16_t conc_pm1_0_amb;
    uint16_t conc_pm2_5_amb;
    uint16_t conc_pm10_0_amb;
    uint16_t raw_gt0_3um;
    uint16_t raw_gt0_5um;
    uint16_t raw_gt1_0um;
    uint16_t raw_gt2_5um;
    uint16_t raw_gt5_0um;
    uint16_t raw_gt10_0um;

    uint16_t hcho;
    bool has_hcho;
    uint16_t temperature;
    bool has_temperature;
    uint16_t humidity;
    bool has_humidity;
    uint16_t reserve;

    uint8_t version;
    uint8_t errorCode;
} PMS_MEAS_T;

// known command bytes
#define PMS_CMD_AUTO_MANUAL 0xE1    // data=0: perform measurement manually, data=1: perform measurement automatically
#define PMS_CMD_TRIG_MANUAL 0xE2    // trigger a manual measurement
#define PMS_CMD_ON_STANDBY  0xE4    // data=0: go to standby mode, data=1: go to normal mode

// parsing state
typedef enum {
    BEGIN1,
    BEGIN2,
    LENGTH1,
    LENGTH2,
    DATA,
    CHECK1,
    CHECK2
} PMS_PARSE_STATE;

typedef struct {
    PMS_PARSE_STATE state;
    uint8_t buf[40];
    int size;
    int idx, len;
    uint16_t chk, sum;
} PMS_PARSE_CTX;


void pms_init(PMS_PARSE_CTX *state);
bool pms_process(PMS_PARSE_CTX *pms_parse_ctx, uint8_t b);
void pms5003_parse(const uint8_t *buf, PMS_MEAS_T *meas);
void pms7003_parse(const uint8_t *buf, PMS_MEAS_T *meas);
int pms_create_cmd(uint8_t *buf, int size, uint8_t cmd, uint16_t data);
int set_interface_attribs(int fd, int speed);

#endif //PMS_PMS_H
