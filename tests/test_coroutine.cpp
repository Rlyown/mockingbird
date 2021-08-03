//
// Created by ChaosChen on 2021/8/1.
//

#include <memory>
#include <vector>
#include <mocker/mocker.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void run_in_coroutine() {
    MOCKER_LOG_INFO(g_logger) << "run_in_coroutine begin";
    mocker::Coroutine::Sleep();
    MOCKER_LOG_INFO(g_logger) << "run_in_coroutine end";
    mocker::Coroutine::Sleep();
}

void test_coroutine() {
    MOCKER_LOG_INFO(g_logger) << "main begin out";
    {
        mocker::Coroutine::GetCurrent();
        MOCKER_LOG_INFO(g_logger) << "main begin";
        mocker::Coroutine::ptr coroutine(new mocker::Coroutine(run_in_coroutine, 0, true));
        coroutine->swapIn();
        MOCKER_LOG_INFO(g_logger) << "main after swapIn";
        coroutine->swapIn();
        MOCKER_LOG_INFO(g_logger) << "main after end";
        coroutine->swapIn();
    }
    MOCKER_LOG_INFO(g_logger) << "main after end out";
}

int main(int argc, char *argv[]) {
    mocker::Thread::SetCurrentName("main");

    std::vector<mocker::Thread::ptr> thrs;
    for (int i = 0; i < 3; ++i) {
        thrs.push_back(std::make_shared<mocker::Thread>(test_coroutine, "name_" + std::to_string(i)));
    }

    for (const auto &thr : thrs) {
        thr->join();
    }



    return 0;
}
