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
#include <getopt.h>
#include <curl/curl.h>
#include "pms.h"
#include "pms_util.h"

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
void pms5003_parse(const uint8_t *buf, pms5003_meas_t *meas) {
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
    meas->temperature = read_uint16(buf, 26);
    meas->humidity = read_uint16(buf, 28);
    meas->reserve = read_uint16(buf, 30);
    meas->version = buf[32];
    meas->errorCode = buf[33];
}

void pms7003_parse(const uint8_t *buf, pms7003_meas_t *meas) {
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

void print_version() {
    printf("PMS version %s, build from git version: %s on %s\n", PMS_VERSION, PMS_GIT_SHA1, PMS_COMPILE_DATETIME);
    printf("Supported model: PMS7003, PMS5003ST\n");
    printf("https://github.com/tqlab/iot-pms\n");
}

void print_usage() {
    print_version();
    printf("Usage: pms\n");
    printf("            -m, --model <model>      PMS model.\n");
    printf("            -d, --dev <dev>          Dev file path, such as /dev/ttyUSB0 on Linux or /dev/cu.SLAB_USBtoUART on Mac OSX.\n");
    printf("            [--log <log file>]       Log file.\n");
    printf("            [-l, --label <label>]    Data post label.\n");
    printf("            [-u, --url <url>]        Data post target url.\n");
    printf("            [-h, --help]             Print this message.\n");
    printf("            [-v, --version]          Print version message.\n");
}


int main(int argc, char *argv[]) {

    /*
    if (argc < 5) {
        printf("Usage: pms %s %s %s %s\n", "5003", "/dev/ttyUSB0", "bedroom", "http://xxx.xxx.xxx/pms/post.json");
        return 0;
    }
     */

    char *model = NULL;
    char *dev_file_path = NULL;

    char *log_file_name = NULL;

    char *label = NULL;
    char *url = NULL;

    static struct option long_options[] = {
            {"model", required_argument, NULL, 'm'},
            {"dev",  required_argument,       NULL, 'd'},
            {"log",  required_argument,       NULL, 'o'},
            {"label", optional_argument, NULL, 'l'},
            {"url", optional_argument, NULL, 'u'},
            {"help", no_argument, NULL, 'h'},
            {"version", no_argument, NULL, 'v'},
            {NULL, 0, NULL, 0}
    };

    int opt;
    int option_index = 0;

    opterr = 0;

    while ( (opt = getopt_long(argc, argv, "m:d:l::u::hv", long_options, &option_index)) != -1) {
        switch (opt) {

            case 'm' :
                model = optarg;
                break;
            case 'd' :
                dev_file_path = optarg;
                break;
            case 'o' :
                log_file_name = optarg;
                break;
            case 'l' :
                label = optarg;
                break;
            case 'u':
                url = optarg;
                break;
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case '?':
                opterr = 1;
                break;
        }
    }

    if (opterr) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (model == NULL || dev_file_path == NULL) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    FILE *log_fp = NULL;
    if (log_file_name != NULL) {
        log_fp = fopen(log_file_name, "w+");
    }

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

            if (strcmp(model, "PMS5003ST") == 0) {

                pms5003_meas_t pms5003_meas;

                pms5003_parse(pms_parse_ctx.buf, &pms5003_meas);

                char *post_flag = "-";

                if (label !=NULL && url != NULL && current_timestamp - last_post_timestamp > 60 * 1000) {

                    // post data to server every minute
                    curl_post_data(label, url, pms5003_meas.conc_pm2_5_amb, pms5003_meas.conc_pm10_0_amb,
                                   pms5003_meas.hcho, pms5003_meas.temperature, pms5003_meas.humidity);

                    last_post_timestamp = current_timestamp;

                    post_flag = "*";
                }


                // output to console
                printf("%s, pm1= %dug/m^3, pm25= %dug/m^3, pm10= %dug/m^3, hcho= %dug/m^3, temperature= %.1fC, humidity= %.1f％, %s\n",
                       current_time_str,
                       pms5003_meas.conc_pm1_0_cf1,
                       pms5003_meas.conc_pm2_5_cf1, pms5003_meas.conc_pm10_0_cf1,
                       pms5003_meas.hcho, (double)pms5003_meas.temperature/10, (double)pms5003_meas.humidity/10, post_flag);
                fflush(stdout);

                if (log_fp != NULL) {
                    // output to log file
                    fprintf(log_fp, "%s,%"PRIu64",%d,%d,%d,%d,%d,%s\n",
                            current_time_str,current_timestamp,
                            pms5003_meas.conc_pm2_5_amb, pms5003_meas.conc_pm10_0_amb,
                            pms5003_meas.hcho, pms5003_meas.temperature, pms5003_meas.humidity, post_flag);

                    fflush(log_fp);
                }
            } else if (strcmp(model, "PMS7003") == 0) {

                pms7003_meas_t pms7003_meas;

                pms7003_parse(pms_parse_ctx.buf, &pms7003_meas);

                printf("%s, pm1= %dug/m^3, pm25= %dug/m^3, pm10= %dug/m^3\n",
                       current_time_str,
                       pms7003_meas.conc_pm1_0_cf1,
                       pms7003_meas.conc_pm2_5_cf1, pms7003_meas.conc_pm10_0_cf1);
                fflush(stdout);


                if (log_fp != NULL) {
                    fprintf(log_fp, "%s,%"PRIu64",%d,%d,%d\n",
                            current_time_str,current_timestamp,
                            pms7003_meas.conc_pm1_0_cf1,
                            pms7003_meas.conc_pm2_5_cf1, pms7003_meas.conc_pm10_0_cf1);
                    fflush(log_fp);
                }

            } else {
                print_usage();
                printf("Unsupported model.\n");
                exit(EXIT_FAILURE);
            }

        }

    }

}