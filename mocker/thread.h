//
// Created by ChaosChen on 2021/7/18.
//

#ifndef MOCKER_THREAD_H
#define MOCKER_THREAD_H

#include <thread>
#include <functional>
#include <memory>

#include <mocker/mutex.h>

namespace mocker {

    class Thread {
    public:
        typedef std::shared_ptr<Thread> ptr;
        typedef std::function<void()> task;

        Thread(Thread::task cb, const std::string& name = "");
        ~Thread();

        pid_t getId() const { return m_id; }
        const std::string& getName() const { return m_name; }

        void join();

        static Thread* GetCurrent();
        static const std::string& GetCurrentName();
        static void SetCurrentName(const std::string& name);

    public:
        Thread(const Thread&) = delete;
        Thread(const Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

    private:
        static void* Run(void* arg);
    private:
        /// thread id
        pid_t m_id = -1;
        /// thread object
        pthread_t m_thread = 0;
        /// callback function
        Thread::task m_cb;
        /// thread name
        std::string m_name;

        Semaphore m_semaphore;
    };
}

#endif //MOCKER_THREAD_H
