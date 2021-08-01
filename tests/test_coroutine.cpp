//
// Created by ChaosChen on 2021/8/1.
//

#include <mocker/mocker.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void run_in_coroutine() {
    MOCKER_LOG_INFO(g_logger) << "run_in_coroutine begin";
    mocker::Coroutine::Sleep();
    MOCKER_LOG_INFO(g_logger) << "run_in_coroutine end";
    mocker::Coroutine::Sleep();
}

int main(int argc, char *argv[]) {
    mocker::Coroutine::GetCurrent();

    MOCKER_LOG_INFO(g_logger) << "main begin out";
    {
        MOCKER_LOG_INFO(g_logger) << "main begin";
        mocker::Coroutine::ptr coroutine(new mocker::Coroutine(run_in_coroutine));
        coroutine->swapIn();
        MOCKER_LOG_INFO(g_logger) << "main after swapIn";
        coroutine->swapIn();
        MOCKER_LOG_INFO(g_logger) << "main after end";
        coroutine->swapIn();
    }
    MOCKER_LOG_INFO(g_logger) << "main after end out";

    return 0;
}
