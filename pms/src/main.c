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

size_t curl_write_data_null(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

void curl_post_data(const char *label, const char *url, const PMS_MEAS_T pms_meas) {
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
    sprintf(pm25_str, "%d", pms_meas.conc_pm2_5_amb);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "pm25",
                 CURLFORM_COPYCONTENTS, pm25_str, CURLFORM_END);

    char pm10_str[10];
    sprintf(pm10_str, "%d", pms_meas.conc_pm10_0_amb);
    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "pm10",
                 CURLFORM_COPYCONTENTS, pm10_str, CURLFORM_END);

    if (pms_meas.has_hcho) {
        char hcho_str[10];
        sprintf(hcho_str, "%d", pms_meas.hcho);
        curl_formadd(&post,
                     &last,
                     CURLFORM_COPYNAME, "hcho",
                     CURLFORM_COPYCONTENTS, hcho_str, CURLFORM_END);
    }

    if (pms_meas.has_temperature) {
        char temperature_str[10];
        sprintf(temperature_str, "%d", pms_meas.temperature);
        curl_formadd(&post,
                     &last,
                     CURLFORM_COPYNAME, "temperature",
                     CURLFORM_COPYCONTENTS, temperature_str, CURLFORM_END);
    }

    if (pms_meas.has_humidity) {
        char humidity_str[10];
        sprintf(humidity_str, "%d", pms_meas.humidity);
        curl_formadd(&post,
                     &last,
                     CURLFORM_COPYNAME, "humidity",
                     CURLFORM_COPYCONTENTS, humidity_str, CURLFORM_END);
    }

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
        if (res != CURLE_OK) {
            LOG_E("curl_easy_perform() failed: %s",
                  curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(post);
        /* free slist */
        curl_slist_free_all(headerlist);
    }
}

void print_version() {
    printf("\n");
    printf("pms version %s, build from git version: %s on %s\n", PMS_VERSION, PMS_GIT_SHA1, PMS_COMPILE_DATETIME);
    printf("supported model: PMS7003, PMS5003ST\n\n");
    printf("https://github.com/tqlab/iot-pms\n\n");
}

void print_usage() {
    print_version();
    printf("usage: pms\n");
    printf("            -m, --model <model>           PMS model.\n");
    printf("            -d, --dev <dev>               Dev file path, such as /dev/ttyUSB0 on Linux or /dev/cu.SLAB_USBtoUART on Mac OSX.\n");
    printf("            [--log <log file>]            Log file.\n");
    printf("            [-l, --label <label>]         Data post label.\n");
    printf("            [-u, --url <url>]             Data post target url.\n");
    printf("            [-i, --interval <interval>]   Data post interval in mill secs.\n");
    printf("            [-h, --help]                  Print this message.\n");
    printf("            [-v, --version]               Print version message.\n");
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
    uint64_t interval = 60 * 1000;

    static struct option long_options[] = {
            {"model",    required_argument, NULL, 'm'},
            {"dev",      required_argument, NULL, 'd'},
            {"log",      required_argument, NULL, 'o'},
            {"label",    required_argument, NULL, 'l'},
            {"url",      required_argument, NULL, 'u'},
            {"interval", required_argument, NULL, 'i'},
            {"help",     no_argument,       NULL, 'h'},
            {"version",  no_argument,       NULL, 'v'},
            {NULL, 0,                       NULL, 0}
    };

    int opt;
    int option_index = 0;

    opterr = 0;

    while ((opt = getopt_long(argc, argv, "m:d:l::u::i::hv", long_options, &option_index)) != -1) {
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
            case 'i' :
                interval = atoll(optarg);
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
        log_fp = fopen(log_file_name, "a+");
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

            // current time millis
            uint64_t current_timestamp = pms_current_time_millis();

            char *post_flag = "-";

            if (label != NULL && url != NULL && current_timestamp - last_post_timestamp > interval) {
                // prepare to post
                post_flag = "*";
            }

            PMS_MEAS_T pms_meas;

            if (strcmp(model, "PMS5003ST") == 0) {

                pms5003_parse(pms_parse_ctx.buf, &pms_meas);

                // output to console
                LOG_I("pm1=%dug/m^3, pm25=%dug/m^3, pm10=%dug/m^3, hcho=%dug/m^3, temperature=%.1fC, humidity=%.1f％, %s",
                      pms_meas.conc_pm1_0_cf1,
                      pms_meas.conc_pm2_5_cf1, pms_meas.conc_pm10_0_cf1,
                      pms_meas.hcho, (double) pms_meas.temperature / 10, (double) pms_meas.humidity / 10,
                      post_flag);

                if (log_fp != NULL) {
                    // output to log file
                    FLOG_I(log_fp, "%"PRIu64",%d,%d,%d,%d,%d,%s",
                           current_timestamp,
                           pms_meas.conc_pm2_5_amb, pms_meas.conc_pm10_0_amb,
                           pms_meas.hcho, pms_meas.temperature, pms_meas.humidity, post_flag);
                }
            } else if (strcmp(model, "PMS7003") == 0) {

                pms7003_parse(pms_parse_ctx.buf, &pms_meas);

                LOG_I("pm1=%dug/m^3, pm25=%dug/m^3, pm10=%dug/m^3, %s",
                      pms_meas.conc_pm1_0_cf1,
                      pms_meas.conc_pm2_5_cf1, pms_meas.conc_pm10_0_cf1,
                      post_flag);

                if (log_fp != NULL) {
                    FLOG_I(log_fp, "%"PRIu64",%d,%d,%d,%s",
                           current_timestamp,
                           pms_meas.conc_pm1_0_cf1,
                           pms_meas.conc_pm2_5_cf1, pms_meas.conc_pm10_0_cf1,
                           post_flag);
                }

            } else {
                print_usage();
                printf("Unsupported model.\n");
                exit(EXIT_FAILURE);
            }

            if (strcmp(post_flag, "*") == 0) {
                // post data to server every minute
                curl_post_data(label, url, pms_meas);

                last_post_timestamp = current_timestamp;
            }

        }

    }

}