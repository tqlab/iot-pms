//
// Created by lijun on 28/1/2018.
//
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <memory.h>
#include <json.h>
#include "pms_util.h"
#include "json.h"

void test_curl();
void test_long_int();
void test_time();
void test_json();

int main(int argc, char *argv[])
{
    //test_curl();
    //test_long_int();
    //test_time();
    test_json();
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

void test_curl()
{
    CURL *curl;
    CURLcode res;

    struct curl_httppost *post=NULL;
    struct curl_httppost *last=NULL;
    struct curl_slist *headerlist=NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "pm25",
                 CURLFORM_COPYCONTENTS, "119", CURLFORM_END);

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "hcho",
                 CURLFORM_COPYCONTENTS, "0", CURLFORM_END);

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "temperature",
                 CURLFORM_COPYCONTENTS, "191", CURLFORM_END);

    curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "humidity",
                 CURLFORM_COPYCONTENTS, "161", CURLFORM_END);


    curl = curl_easy_init();
    /* initalize custom header list (stating that Expect: 100-continue is not
       wanted */
    headerlist = curl_slist_append(headerlist, buf);
    if(curl) {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/pms/post.json");

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(post);
        /* free slist */
        curl_slist_free_all (headerlist);
    }
}

void test_long_int()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    uint64_t sec = tv.tv_sec;
    uint64_t usec = tv.tv_usec;

    LOG_I("1. %"PRIu64"", sec);
    LOG_I("2. %"PRIu64"", usec);

    uint64_t timestamp = sec * 1000 + usec / 1000;

    LOG_I("3. %"PRIu64"", timestamp);
    LOG_I("4. %"PRId64"", timestamp);

    LOG_I("5. %lld", timestamp);

    uint64_t timestamp_2 = 0;

    LOG_I("6. %"PRIu64"", pms_current_time_millis(&timestamp_2));

    LOG_I("7. %"PRIu64"", timestamp_2);

    uint64_t tt = pms_current_time_millis(&timestamp_2);
    LOG_I("8. %"PRIu64"", tt);

    LOG_I("9. %.1f", (double)123/10);
}

#define TEST_STR "123456"

void test_time() {
    char time_str[40];

    pms_current_local_time_str(time_str, 40);

    printf("printf1: %s\n", time_str);

    LOG_D("%s", time_str);

    printf("printf2: %s\n", time_str);
    LOG_I("%s", time_str);

    printf("printf3: %s\n", time_str);
    LOG_E("%s", time_str);

    printf("printf4: %s\n", time_str);

    LOG_D("%s", TEST_STR);

#ifdef PMS_DEBUG
    LOG_E("%s", "test");
#endif

}

void test_json() {
    json_char* json = (json_char*)"{\"dns\":[{\"domain\":\"mobilegw.alipay.com\",\"ttl\":300,\"ips\":[{\"ip\":\"110.76.30.77\",\"port\":443},{\"ip\":\"110.75.138.9\",\"port\":443},{\"ip\":\"110.75.245.22\",\"port\":443}]}],\"ttd\":7,\"code\":1000,\"clientIp\":\"42.120.75.67\"}";

    json_value* json_value = json_parse(json, strlen(json));


    LOG_D("%d", json_value->type);




    int length = json_value->u.object.length;
    LOG_D("%d", length);

    for (int x = 0; x < length; x++) {

        json_object_entry entry = json_value->u.object.values[x];

        if (entry.value->type == json_integer) {
            LOG_D("name= %s, type= %d, value= %"PRIu64"",
                  entry.name, entry.value->type, entry.value->u.integer);
        }

        if (entry.value->type == json_double) {
            LOG_D("name= %s, type= %d, value= %f",
                  entry.name, entry.value->type, entry.value->u.dbl);
        }

        if (entry.value->type == json_string) {
            LOG_D("name= %s, type= %d, value= %s",
                  entry.name, entry.value->type, entry.value->u.string.ptr);
        }

        if (entry.value->type == json_array) {
            LOG_D("name= %s, type= %d",
                  entry.name, entry.value->type);

            for (int i = 0; i < entry.value->u.array.length; ++i) {
                LOG_D("-> %d", entry.value->u.array.values[i]->type);
            }
        }



        //process_value(value->u.object.values[x].value, depth+1);
    }

    json_value_free(json_value);
}