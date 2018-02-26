//
// Created by lijun on 26/1/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include "pms.h"
#include "pms_util.h"

/**
    Initializes the measurement data state machine.
 */
void pms_init(PMS_PARSE_CTX *state) {
    state->state = BEGIN1;
    state->size = sizeof(state->buf);
    state->idx = state->len = 0;
    state->chk = state->sum = 0;
}

// magic header bytes (actually ASCII 'B' and 'M')
#define MAGIC1 0x42
#define MAGIC2 0x4D

/**
    Processes one byte in the measurement data state machine.
    @param[in] b the byte
    @return true if a full message was received
 */
bool pms_process(PMS_PARSE_CTX *pms_parse_ctx, uint8_t b) {
    switch (pms_parse_ctx->state) {
        // wait for BEGIN1 byte
        case BEGIN1:
            pms_parse_ctx->sum = b;
            if (b == MAGIC1) {
                pms_parse_ctx->state = BEGIN2;
            }
            break;
            // wait for BEGIN2 byte
        case BEGIN2:
            pms_parse_ctx->sum += b;
            if (b == MAGIC2) {
                pms_parse_ctx->state = LENGTH1;
            } else {
                pms_parse_ctx->state = BEGIN1;
                // retry
                return pms_process(pms_parse_ctx, b);
            }
            break;
            // verify data length
        case LENGTH1:
            pms_parse_ctx->sum += b;
            pms_parse_ctx->len = b << 8;
            pms_parse_ctx->state = LENGTH2;
            break;
        case LENGTH2:
            pms_parse_ctx->sum += b;
            pms_parse_ctx->len += b;
            pms_parse_ctx->len -= 2;     // exclude checksum bytes
            if (pms_parse_ctx->len <= pms_parse_ctx->size) {
                pms_parse_ctx->idx = 0;
                pms_parse_ctx->state = DATA;
            } else {
                // bogus length
                pms_parse_ctx->state = BEGIN1;
            }
            break;
            // store data
        case DATA:
            pms_parse_ctx->sum += b;
            if (pms_parse_ctx->idx < pms_parse_ctx->len) {
                pms_parse_ctx->buf[pms_parse_ctx->idx++] = b;
            }
            if (pms_parse_ctx->idx == pms_parse_ctx->len) {
                pms_parse_ctx->state = CHECK1;
            }
            break;
            // store checksum
        case CHECK1:
            pms_parse_ctx->chk = b << 8;
            pms_parse_ctx->state = CHECK2;
            break;
            // verify checksum
        case CHECK2:
            pms_parse_ctx->chk += b;
            pms_parse_ctx->state = BEGIN1;
            return (pms_parse_ctx->chk == pms_parse_ctx->sum);
        default:
            pms_parse_ctx->state = BEGIN1;
            break;
    }
    return false;
}

/**
    Parses a complete measurement data frame into a structure.
    @param[out] meas the parsed measurement data
 */
void pms5003_parse(const uint8_t *buf, PMS_MEAS_T *meas) {
    meas->conc_pm1_0_cf1 = read_uint16(buf, 0);
    meas->conc_pm2_5_cf1 = read_uint16(buf, 2);
    meas->conc_pm10_0_cf1 = read_uint16(buf, 4);
    meas->conc_pm1_0_amb = read_uint16(buf, 6);
    meas->conc_pm2_5_amb = read_uint16(buf, 8);
    meas->conc_pm10_0_amb = read_uint16(buf, 10);
    meas->raw_gt0_3um = read_uint16(buf, 12);
    meas->raw_gt0_5um = read_uint16(buf, 14);
    meas->raw_gt1_0um = read_uint16(buf, 16);
    meas->raw_gt2_5um = read_uint16(buf, 18);
    meas->raw_gt5_0um = read_uint16(buf, 20);
    meas->raw_gt10_0um = read_uint16(buf, 22);

    meas->hcho = read_uint16(buf, 24);
    meas->has_hcho = true;

    meas->temperature = read_uint16(buf, 26);
    meas->has_temperature = true;

    meas->humidity = read_uint16(buf, 28);
    meas->has_humidity = true;

    meas->reserve = read_uint16(buf, 30);
    meas->version = buf[32];
    meas->errorCode = buf[33];
}

void pms7003_parse(const uint8_t *buf, PMS_MEAS_T *meas) {
    meas->conc_pm1_0_cf1 = read_uint16(buf, 0);
    meas->conc_pm2_5_cf1 = read_uint16(buf, 2);
    meas->conc_pm10_0_cf1 = read_uint16(buf, 4);
    meas->conc_pm1_0_amb = read_uint16(buf, 6);
    meas->conc_pm2_5_amb = read_uint16(buf, 8);
    meas->conc_pm10_0_amb = read_uint16(buf, 10);
    meas->raw_gt0_3um = read_uint16(buf, 12);
    meas->raw_gt0_5um = read_uint16(buf, 14);
    meas->raw_gt1_0um = read_uint16(buf, 16);
    meas->raw_gt2_5um = read_uint16(buf, 18);
    meas->raw_gt5_0um = read_uint16(buf, 20);
    meas->raw_gt10_0um = read_uint16(buf, 22);

    meas->has_hcho = false;
    meas->has_temperature = false;
    meas->has_humidity = false;

    meas->version = buf[24];
    meas->errorCode = buf[25];
}


/**
    Creates a command buffer to send.
    @param[in] buf the command buffer
    @param[in] size the size of the command buffer, should be at least 7 bytes
    @param[in] cmd the command byte
    @param[in] data the data field
    @return the length of the command buffer, or 0 if the size was too small
*/
int pms_create_cmd(uint8_t *buf, int size, uint8_t cmd, uint16_t data) {
    if (size < 7) {
        return 0;
    }

    int idx = 0;
    buf[idx++] = MAGIC1;
    buf[idx++] = MAGIC2;
    buf[idx++] = cmd;
    buf[idx++] = (data >> 8) & 0xFF;
    buf[idx++] = (data >> 0) & 0xFF;
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += buf[i];
    }
    buf[idx++] = (sum >> 8) & 0xFF;
    buf[idx++] = (sum >> 0) & 0xFF;
    return idx;
}


int set_interface_attribs(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t) speed);
    cfsetispeed(&tty, (speed_t) speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
