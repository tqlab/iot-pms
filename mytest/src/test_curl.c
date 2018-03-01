#include <inttypes.h>
#include <time.h>
#include <curl/curl.h>

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

void test_curl() {
    CURL *curl;
    CURLcode res;

    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    struct curl_slist *headerlist = NULL;
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
    if (curl) {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/pms/post.json");

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

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