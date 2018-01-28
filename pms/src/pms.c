//
// Created by lijun on 26/1/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "pms.h"
#include "pmsutil.h"

static TState state;
static pms5003_meas_t pms5003_meas;
static pms7003_meas_t pms7003_meas;
static FILE *fp = NULL;

/**
    Initializes the measurement data state machine.
 */
void PmsInit(void) {
    state.state = BEGIN1;
    state.size = sizeof(state.buf);
    state.idx = state.len = 0;
    state.chk = state.sum = 0;
}

/**
    Processes one byte in the measurement data state machine.
    @param[in] b the byte
    @return true if a full message was received
 */
bool PmsProcess(uint8_t b) {
    switch (state.state) {
        // wait for BEGIN1 byte
        case BEGIN1:
            state.sum = b;
            if (b == MAGIC1) {
                state.state = BEGIN2;
            }
            break;
            // wait for BEGIN2 byte
        case BEGIN2:
            state.sum += b;
            if (b == MAGIC2) {
                state.state = LENGTH1;
            } else {
                state.state = BEGIN1;
                // retry
                return PmsProcess(b);
            }
            break;
            // verify data length
        case LENGTH1:
            state.sum += b;
            state.len = b << 8;
            state.state = LENGTH2;
            break;
        case LENGTH2:
            state.sum += b;
            state.len += b;
            state.len -= 2;     // exclude checksum bytes
            if (state.len <= state.size) {
                state.idx = 0;
                state.state = DATA;
            } else {
                // bogus length
                state.state = BEGIN1;
            }
            break;
            // store data
        case DATA:
            state.sum += b;
            if (state.idx < state.len) {
                state.buf[state.idx++] = b;
            }
            if (state.idx == state.len) {
                state.state = CHECK1;
            }
            break;
            // store checksum
        case CHECK1:
            state.chk = b << 8;
            state.state = CHECK2;
            break;
            // verify checksum
        case CHECK2:
            state.chk += b;
            state.state = BEGIN1;
            return (state.chk == state.sum);
        default:
            state.state = BEGIN1;
            break;
    }
    return false;
}

/**
    Parses a complete measurement data frame into a structure.
    @param[out] meas the parsed measurement data
 */
void Pms5003Parse(pms5003_meas_t *meas) {
    meas->concPM1_0_CF1 = read_uint16(state.buf, 0);
    meas->concPM2_5_CF1 = read_uint16(state.buf, 2);
    meas->concPM10_0_CF1 = read_uint16(state.buf, 4);
    meas->concPM1_0_amb = read_uint16(state.buf, 6);
    meas->concPM2_5_amb = read_uint16(state.buf, 8);
    meas->concPM10_0_amb = read_uint16(state.buf, 10);
    meas->rawGt0_3um = read_uint16(state.buf, 12);
    meas->rawGt0_5um = read_uint16(state.buf, 14);
    meas->rawGt1_0um = read_uint16(state.buf, 16);
    meas->rawGt2_5um = read_uint16(state.buf, 18);
    meas->rawGt5_0um = read_uint16(state.buf, 20);
    meas->rawGt10_0um = read_uint16(state.buf, 22);
    meas->hcho = read_uint16(state.buf, 24);
    meas->temperature = read_uint16(state.buf, 26);
    meas->humidity = read_uint16(state.buf, 28);
    meas->reserve = read_uint16(state.buf, 30);
    meas->version = state.buf[32];
    meas->errorCode = state.buf[33];

    char current_time_str[40];

    pms_current_local_time_str(current_time_str, 40);

    printf("%s\tpm1= %dug/m³\tpm25= %dug/m³\tpm10= %dug/m³\thcho= %d mg/m³\ttemperature = %d℃\thumidity = %d％\n",
           current_time_str,
           meas->concPM1_0_CF1,
           meas->concPM2_5_CF1, meas->concPM10_0_CF1, meas->hcho, meas->temperature, meas->humidity);
    fflush(stdout);

    fprintf(fp, "%s,%d,%d,%d,%d,%d\n",
            current_time_str,
            meas->concPM2_5_amb, meas->concPM10_0_amb, meas->hcho, meas->temperature, meas->humidity);
    fflush(fp);
}

void Pms7003Parse(pms7003_meas_t *meas) {
    meas->concPM1_0_CF1 = read_uint16(state.buf, 0);
    meas->concPM2_5_CF1 = read_uint16(state.buf, 2);
    meas->concPM10_0_CF1 = read_uint16(state.buf, 4);
    meas->concPM1_0_amb = read_uint16(state.buf, 6);
    meas->concPM2_5_amb = read_uint16(state.buf, 8);
    meas->concPM10_0_amb = read_uint16(state.buf, 10);
    meas->rawGt0_3um = read_uint16(state.buf, 12);
    meas->rawGt0_5um = read_uint16(state.buf, 14);
    meas->rawGt1_0um = read_uint16(state.buf, 16);
    meas->rawGt2_5um = read_uint16(state.buf, 18);
    meas->rawGt5_0um = read_uint16(state.buf, 20);
    meas->rawGt10_0um = read_uint16(state.buf, 22);
    meas->version = state.buf[24];
    meas->errorCode = state.buf[25];


    char current_time_str[40];
    pms_current_local_time_str(current_time_str, 40);

    printf("%s\tpm1= %dug/m³\tpm25= %dug/m³\tpm10= %dug/m³\n",
           current_time_str,
           meas->concPM1_0_CF1,
           meas->concPM2_5_CF1, meas->concPM10_0_CF1);
    fflush(stdout);

    fprintf(fp, "%s,%d,%d,%d\n",
            current_time_str,
            meas->concPM1_0_CF1,
            meas->concPM2_5_CF1, meas->concPM10_0_CF1);
    fflush(fp);
}


/**
    Creates a command buffer to send.
    @param[in] buf the command buffer
    @param[in] size the size of the command buffer, should be at least 7 bytes
    @param[in] cmd the command byte
    @param[in] data the data field
    @return the length of the command buffer, or 0 if the size was too small
*/
int PmsCreateCmd(uint8_t *buf, int size, uint8_t cmd, uint16_t data) {
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: pms %s %s\n", "5003", "/dev/ttyUSB0");
        return 0;
    }


    char *type = argv[1];
    char *portname = argv[2];

    char fileName[64] = {0};
    strcat (fileName, "pms");
    strcat (fileName, type);
    strcat (fileName, ".log");

    fp = fopen(fileName, "w+");

    int fd;
    int wlen;

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    set_interface_attribs(fd, B9600);

    /* simple output */
    wlen = write(fd, "Hello!\n", 7);
    if (wlen != 7) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
    tcdrain(fd);    /* delay for output */

    uint8_t frameBuf[64];
    int rdlen = 0;
    PmsInit();
    while ((rdlen = read(fd, frameBuf, 1)) > 0) {
        if (PmsProcess(frameBuf[0])) {
            if (strcmp(type, "5003") == 0) {
                Pms5003Parse(&pms5003_meas);
            } else {
                Pms7003Parse(&pms7003_meas);
            }

        }
    }

}