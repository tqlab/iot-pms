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
void test_format();

int main(int argc, char *argv[])
{
    //test_curl();
    test_long_int();
    test_time();
    test_json();
    test_format();
}
