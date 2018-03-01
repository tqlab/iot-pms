#include <inttypes.h>
#include <time.h>
#include <curl/curl.h>
#include <memory.h>
#include "pms_util.h"
#include "json.h"

void test_json() {
    json_char *json = (json_char *) "{\"dns\":[{\"domain\":\"mobilegw.alipay.com\",\"ttl\":300,\"ips\":[{\"ip\":\"110.76.30.77\",\"port\":443},{\"ip\":\"110.75.138.9\",\"port\":443},{\"ip\":\"110.75.245.22\",\"port\":443}]}],\"ttd\":7,\"code\":1000,\"clientIp\":\"42.120.75.67\"}";

    json_value *json_value = json_parse(json, strlen(json));


    LOG_D("%d", json_value->type);


    int length = json_value->u.object.length;
    LOG_D("%d", length);

    for (int x = 0; x < length; x++) {

        json_object_entry entry = json_value->u.object.values[x];

        if (entry.value->type == json_integer) {
            LOG_D("name= %s, type= %d, value= %lld",
                  entry.name, entry.value->type, lld_cast(entry.value->u.integer));
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