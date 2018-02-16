//
// Created by lijun on 26/1/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <curl/curl.h>
#include "pms.h"
#include "pmsutil.h"

size_t curl_write_data_null(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

void curl_post_data(char *label, char *url, uint16_t pm25, uint16_t pm10, uint16_t hcho, uint16_t temperature,
                    uint16_t humidity) {
    CURL *curl;
    CURLcode res;

    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";


    curl_global_init(CURL_GLOBAL_ALL);

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "label",
                 CURLFORM_COPYCONTENTS, label, CURLFORM_END);

    char pm25_str[10];
    sprintf(pm25_str, "%d", pm25);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "pm25",
                 CURLFORM_COPYCONTENTS, pm25_str, CURLFORM_END);

    char pm10_str[10];
    sprintf(pm10_str, "%d", pm10);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "pm10",
                 CURLFORM_COPYCONTENTS, pm10_str, CURLFORM_END);

    char hcho_str[10];
    sprintf(hcho_str, "%d", hcho);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "hcho",
                 CURLFORM_COPYCONTENTS, hcho_str, CURLFORM_END);

    char temperature_str[10];
    sprintf(temperature_str, "%d", temperature);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "temperature",
                 CURLFORM_COPYCONTENTS, temperature_str, CURLFORM_END);

    char humidity_str[10];
    sprintf(humidity_str, "%d", humidity);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "humidity",
                 CURLFORM_COPYCONTENTS, humidity_str, CURLFORM_END);


    curl = curl_easy_init();
    /* initalize custom header list (stating that Expect: 100-continue is not
       wanted */
    headerlist = curl_slist_append(headerlist, buf);
    if (curl) {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data_null);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(post);
        /* free slist */
        curl_slist_free_all(headerlist);
    }
}

/**
    Initializes the measurement data state machine.
 */
void pms_init(PMS_PARSE_CTX *state) {
    state->state = BEGIN1;
    state->size = sizeof(state->buf);
    state->idx = state->len = 0;
    state->chk = state->sum = 0;
}

/**
    Processes one byte in the measurement data state machine.
    @param[in] b the byte
    @return true if a full message was received
 */
bool pms_process(PMS_PARSE_CTX *state, uint8_t b) {
    switch (state->state) {
        // wait for BEGIN1 byte
        case BEGIN1:
            state->sum = b;
            if (b == MAGIC1) {
                state->state = BEGIN2;
            }
            break;
            // wait for BEGIN2 byte
        case BEGIN2:
            state->sum += b;
            if (b == MAGIC2) {
                state->state = LENGTH1;
            } else {
                state->state = BEGIN1;
                // retry
                return pms_process(state, b);
            }
            break;
            // verify data length
        case LENGTH1:
            state->sum += b;
            state->len = b << 8;
            state->state = LENGTH2;
            break;
        case LENGTH2:
            state->sum += b;
            state->len += b;
            state->len -= 2;     // exclude checksum bytes
            if (state->len <= state->size) {
                state->idx = 0;
                state->state = DATA;
            } else {
                // bogus length
                state->state = BEGIN1;
            }
            break;
            // store data
        case DATA:
            state->sum += b;
            if (state->idx < state->len) {
                state->buf[state->idx++] = b;
            }
            if (state->idx == state->len) {
                state->state = CHECK1;
            }
            break;
            // store checksum
        case CHECK1:
            state->chk = b << 8;
            state->state = CHECK2;
            break;
            // verify checksum
        case CHECK2:
            state->chk += b;
            state->state = BEGIN1;
            return (state->chk == state->sum);
        default:
            state->state = BEGIN1;
            break;
    }
    return false;
}

/**
    Parses a complete measurement data frame into a structure.
    @param[out] meas the parsed measurement data
 */
void pms5003_parse(PMS_PARSE_CTX *state, pms5003_meas_t *meas) {
    meas->conc_pm1_0_cf1 = read_uint16(state->buf, 0);
    meas->conc_pm2_5_cf1 = read_uint16(state->buf, 2);
    meas->conc_pm10_0_cf1 = read_uint16(state->buf, 4);
    meas->conc_pm1_0_amb = read_uint16(state->buf, 6);
    meas->conc_pm2_5_amb = read_uint16(state->buf, 8);
    meas->conc_pm10_0_amb = read_uint16(state->buf, 10);
    meas->raw_gt0_3um = read_uint16(state->buf, 12);
    meas->raw_gt0_5um = read_uint16(state->buf, 14);
    meas->raw_gt1_0um = read_uint16(state->buf, 16);
    meas->raw_gt2_5um = read_uint16(state->buf, 18);
    meas->raw_gt5_0um = read_uint16(state->buf, 20);
    meas->raw_gt10_0um = read_uint16(state->buf, 22);
    meas->hcho = read_uint16(state->buf, 24);
    meas->temperature = read_uint16(state->buf, 26);
    meas->humidity = read_uint16(state->buf, 28);
    meas->reserve = read_uint16(state->buf, 30);
    meas->version = state->buf[32];
    meas->errorCode = state->buf[33];
}

void pms7003_parse(PMS_PARSE_CTX *state, pms7003_meas_t *meas) {
    meas->conc_pm1_0_cf1 = read_uint16(state->buf, 0);
    meas->conc_pm2_5_cf1 = read_uint16(state->buf, 2);
    meas->conc_pm10_0_cf1 = read_uint16(state->buf, 4);
    meas->conc_pm1_0_amb = read_uint16(state->buf, 6);
    meas->conc_pm2_5_amb = read_uint16(state->buf, 8);
    meas->conc_pm10_0_amb = read_uint16(state->buf, 10);
    meas->raw_gt0_3um = read_uint16(state->buf, 12);
    meas->raw_gt0_5um = read_uint16(state->buf, 14);
    meas->raw_gt1_0um = read_uint16(state->buf, 16);
    meas->raw_gt2_5um = read_uint16(state->buf, 18);
    meas->raw_gt5_0um = read_uint16(state->buf, 20);
    meas->raw_gt10_0um = read_uint16(state->buf, 22);
    meas->version = state->buf[24];
    meas->errorCode = state->buf[25];
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

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: pms %s %s %s %s\n", "5003", "/dev/ttyUSB0", "bedroom", "http://xxx.xxx.xxx/pms/post.json");
        return 0;
    }


    char *type = argv[1];
    char *dev_file_path = argv[2];

    char *label = argv[3];
    char *url = argv[4];

    char log_file_name[64] = {0};
    strcat (log_file_name, "pms");
    strcat (log_file_name, type);
    strcat (log_file_name, ".log");

    FILE *log_fp = fopen(log_file_name, "w+");

    int dev_fd = open(dev_file_path, O_RDWR | O_NOCTTY | O_SYNC);
    if (dev_fd < 0) {
        printf("Error opening %s: %s\n", dev_file_path, strerror(errno));
        return -1;
    }
    set_interface_attribs(dev_fd, B9600);

    /* simple output */
    /*
    ssize_t wlen = write(dev_fd, "Hello!\n", 7);
    if (wlen != 7) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
    */

    tcdrain(dev_fd);    /* delay for output */

    PMS_PARSE_CTX pms_parse_ctx;
    pms_init(&pms_parse_ctx);

    uint8_t frameBuf[64];
    ssize_t rdlen = 0;

    // last post data to server timestamp
    uint64_t last_post_timestamp = 0;

    while ((rdlen = read(dev_fd, frameBuf, 1)) > 0) {

        if (pms_process(&pms_parse_ctx, frameBuf[0])) {

            // human readable time str
            char current_time_str[40];

            // get the time str
            pms_current_local_time_str(current_time_str, 40);

            // current time millis
            uint64_t current_timestamp = pms_current_time_millis();

            if (strcmp(type, "5003") == 0) {

                pms5003_meas_t pms5003_meas;

                pms5003_parse(&pms_parse_ctx, &pms5003_meas);

                // output to console
                printf("%s\tpm1= %dug/m^3\tpm25= %dug/m^3\tpm10= %dug/m^3\thcho= %d mg/m^3\ttemperature = %dC\thumidity = %d％\n",
                       current_time_str,
                       pms5003_meas.conc_pm1_0_cf1,
                       pms5003_meas.conc_pm2_5_cf1, pms5003_meas.conc_pm10_0_cf1,
                       pms5003_meas.hcho, pms5003_meas.temperature, pms5003_meas.humidity);
                fflush(stdout);

                // output to log file
                fprintf(log_fp, "%s,%d,%d,%d,%d,%d\n",
                        current_time_str,
                        pms5003_meas.conc_pm2_5_amb, pms5003_meas.conc_pm10_0_amb,
                        pms5003_meas.hcho, pms5003_meas.temperature, pms5003_meas.humidity);

                fflush(log_fp);

                if (current_timestamp - last_post_timestamp > 60 * 1000) {

                    // post data to server every minute
                    curl_post_data(label, url, pms5003_meas.conc_pm2_5_amb, pms5003_meas.conc_pm10_0_amb,
                                   pms5003_meas.hcho, pms5003_meas.temperature, pms5003_meas.humidity);

                    last_post_timestamp = current_timestamp;
                }

            } else {

                pms7003_meas_t pms7003_meas;

                pms7003_parse(&pms_parse_ctx, &pms7003_meas);

                printf("%s\tpm1= %dug/m³\tpm25= %dug/m³\tpm10= %dug/m³\n",
                       current_time_str,
                       pms7003_meas.conc_pm1_0_cf1,
                       pms7003_meas.conc_pm2_5_cf1, pms7003_meas.conc_pm10_0_cf1);
                fflush(stdout);

                fprintf(log_fp, "%s,%d,%d,%d\n",
                        current_time_str,
                        pms7003_meas.conc_pm1_0_cf1,
                        pms7003_meas.conc_pm2_5_cf1, pms7003_meas.conc_pm10_0_cf1);
                fflush(log_fp);
            }

        }

    }

}