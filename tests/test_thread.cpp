//
// Created by ChaosChen on 2021/7/18.
//

#include <vector>
#include <unistd.h>

#include <mocker/mocker.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

int count = 0;
mocker::RWMutex s_mutex;
mocker::Mutex mutex;

void fun1() {
    MOCKER_LOG_INFO(g_logger) << "name: " << mocker::Thread::getCurrentName()
        << " this.name: " << mocker::Thread::getCurrent()->getName()
        << " id: " << mocker::getThreadId()
        << " this.id: " << mocker::Thread::getCurrent()->getId();

    for (int i = 0; i < 100000; ++i) {
//        mocker::RWMutex::WriteLock lock(s_mutex);
        mocker::Mutex::Lock lock(mutex);
        ++count;
    }

}

void fun2() {
    int n = 100;
    while (n--)
        MOCKER_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

void fun3() {
    int n = 100;
    while (n--)
        MOCKER_LOG_INFO(g_logger) << "============================================================";
}

int main(int argc, char *argv[]) {
    YAML::Node root = YAML::LoadFile("../conf/log.yml");
    mocker::Config::loadFromYaml(root);
    MOCKER_LOG_INFO(g_logger) << "thread test begin";
    std::vector<mocker::Thread::ptr> thrs;
    for (int i = 0; i < 2; ++i) {
        mocker::Thread::ptr thr(new mocker::Thread(fun2, "name_" + std::to_string(i * 2)));
        mocker::Thread::ptr thr2(new mocker::Thread(fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (auto & thr : thrs) {
        thr->join();
    }

    MOCKER_LOG_INFO(g_logger) << "thread test end";
    MOCKER_LOG_INFO(g_logger) << "count=" << count;
    return 0;
}