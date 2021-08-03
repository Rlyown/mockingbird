//
// Created by ChaosChen on 2021/8/2.
//

#include <mocker/mocker.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void test_coroutine() {
    static int s_count = 5;
    MOCKER_LOG_INFO(g_logger) << "test in coroutine s_count=" << s_count;

    sleep(1);
    if (--s_count >= 0)
        mocker::Scheduler::GetCurrent()->schedule(&test_coroutine, mocker::GetThreadId());
}

int main(int argc, char *argv[]) {
    mocker::Scheduler sc(3, false, "test");
    sc.start();
    sleep(2);
    sc.schedule(&test_coroutine);
    sc.stop();
    return 0;
}
