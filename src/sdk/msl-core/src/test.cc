#include "test.h"

#include "rtc_base/logging.h"
#include <string>
#include <sstream>
#define TAG "test"
int test(int a, int b) {
    rtc::msl_print(rtc::LS_ZORRO, TAG, __FILE__, __LINE__, "====%d", (a+b));
    return a+b;
}

int test3(int a, int b) {
    return a+b;
}