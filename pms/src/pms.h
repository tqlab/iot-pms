//
// Created by lijun on 26/1/2018.
//

#ifndef PMS_PMS_H
#define PMS_PMS_H

// parsed measurement data
typedef struct {
    uint16_t concPM1_0_CF1;
    uint16_t concPM2_5_CF1;
    uint16_t concPM10_0_CF1;
    uint16_t concPM1_0_amb;
    uint16_t concPM2_5_amb;
    uint16_t concPM10_0_amb;
    uint16_t rawGt0_3um;
    uint16_t rawGt0_5um;
    uint16_t rawGt1_0um;
    uint16_t rawGt2_5um;
    uint16_t rawGt5_0um;
    uint16_t rawGt10_0um;
    uint8_t version;
    uint8_t errorCode;
} pms7003_meas_t;

// parsed measurement data
typedef struct {
    uint16_t concPM1_0_CF1;
    uint16_t concPM2_5_CF1;
    uint16_t concPM10_0_CF1;
    uint16_t concPM1_0_amb;
    uint16_t concPM2_5_amb;
    uint16_t concPM10_0_amb;
    uint16_t rawGt0_3um;
    uint16_t rawGt0_5um;
    uint16_t rawGt1_0um;
    uint16_t rawGt2_5um;
    uint16_t rawGt5_0um;
    uint16_t rawGt10_0um;

    uint16_t hcho;
    uint16_t temperature;
    uint16_t humidity;
    uint16_t reserve;

    uint8_t version;
    uint8_t errorCode;
} pms5003_meas_t;

// known command bytes
#define PMS_CMD_AUTO_MANUAL 0xE1    // data=0: perform measurement manually, data=1: perform measurement automatically
#define PMS_CMD_TRIG_MANUAL 0xE2    // trigger a manual measurement
#define PMS_CMD_ON_STANDBY  0xE4    // data=0: go to standby mode, data=1: go to normal mode


// magic header bytes (actually ASCII 'B' and 'M')
#define MAGIC1 0x42
#define MAGIC2 0x4D


// parsing state
typedef enum {
    BEGIN1,
    BEGIN2,
    LENGTH1,
    LENGTH2,
    DATA,
    CHECK1,
    CHECK2
} EState;

typedef struct {
    EState state;
    uint8_t buf[40];
    int size;
    int idx, len;
    uint16_t chk, sum;
} TState;


#endif //PMS_PMS_H
