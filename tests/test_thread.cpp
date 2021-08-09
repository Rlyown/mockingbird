//
// Created by ChaosChen on 2021/7/18.
//

#include <vector>
#include <unistd.h>
#include <sys/time.h>  /* NOLINT */
#include <iostream>

#include <mocker/mocker.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();  /* NOLINT */

int count = 0;
mocker::RWMutex s_mutex;
mocker::Mutex mutex;

void fun1() {
    MOCKER_LOG_INFO(g_logger) << "name: " << mocker::Thread::GetCurrentName()
        << " this.name: " << mocker::Thread::GetCurrent()->getName()
        << " id: " << mocker::GetThreadId()
        << " this.id: " << mocker::Thread::GetCurrent()->getId();

    for (int i = 0; i < 100000; ++i) {
//        mocker::RWMutex::WriteLock lock(s_mutex);
        mocker::Mutex::Lock lock(mutex);
        ++count;
    }

}

void fun2() {
//    int n = 10000;
    while (true)
        MOCKER_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

void fun3() {
//    int n = 10000;
    while (true)
        MOCKER_LOG_INFO(g_logger) << "============================================================";
}

int main(int argc, char *argv[]) {
    struct timeval t1, t2;
    double timeuse;

    YAML::Node root = YAML::LoadFile("../conf/log.yml");
    mocker::Config::LoadFromYaml(root);
    MOCKER_LOG_INFO(g_logger) << "thread test begin";
    std::vector<mocker::Thread::ptr> thrs;

    gettimeofday(&t1,nullptr);

    for (int i = 0; i < 2; ++i) {
        mocker::Thread::ptr thr(new mocker::Thread(fun2, "name_" + std::to_string(i * 2)));
        mocker::Thread::ptr thr2(new mocker::Thread(fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (auto & thr : thrs) {
        thr->join();
    }

    gettimeofday(&t2,nullptr);

    MOCKER_LOG_INFO(g_logger) << "thread test end";
    MOCKER_LOG_INFO(g_logger) << "count=" << count;

    timeuse = (t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000000.0;
    std::cout << "time = " << timeuse << std::endl;
    // run fun2 and fun3 at 10000 times
    // Mutex    -> 2.334
    // Spinlock -> 2.26452
    // CASLock  -> 2.5923


    return 0;
}