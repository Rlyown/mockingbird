//
// Created by ChaosChen on 2021/7/18.
//

#include <mocker/mocker.h>
#include <vector>
#include <unistd.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void fun1() {
    MOCKER_LOG_INFO(g_logger) << "name: " << mocker::Thread::getCurrentName()
        << " this.name: " << mocker::Thread::getCurrent()->getName()
        << " id: " << mocker::getThreadId()
        << " this.id: " << mocker::Thread::getCurrent()->getId();

    // sleep to check thread info from ps/top command
    sleep(60);
}

void fun2() {

}

int main(int argc, char *argv[]) {
    MOCKER_LOG_INFO(g_logger) << "thread test begin";
    std::vector<mocker::Thread::ptr> thrs;
    for (int i = 0; i < 5; ++i) {
        mocker::Thread::ptr thr(new mocker::Thread(fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }

    MOCKER_LOG_INFO(g_logger) << "thread test end";
    return 0;
}