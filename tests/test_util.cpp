//
// Created by ChaosChen on 2021/7/31.
//

#include <mocker/mocker.h>
#include <cassert>

static mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void test_assert() {
//    MOCKER_LOG_INFO(g_logger) << mocker::BacktraceToString(10);
    MOCKER_ASSERT2(0 == 1, "abcdefg xxx");
}

int main(int argc, char *argv[]) {
    test_assert();
    return 0;
}