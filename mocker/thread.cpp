//
// Created by ChaosChen on 2021/7/18.
//

#include <mocker/thread.h>
#include <mocker/log.h>
#include <mocker/util.h>

#include <utility>

namespace mocker {
    // Current thread local variable
    static thread_local Thread* t_thread;
    static thread_local std::string t_thread_name = "UNKNOWN";

    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    ////////////////////////////////////////////////////////////////////
    /// Thread
    ////////////////////////////////////////////////////////////////////
    Thread* Thread::getCurrent() {
        return t_thread;
    }

    const std::string & Thread::getCurrentName() {
        return t_thread_name;
    }

    void Thread::setCurrentName(const std::string &name) {
        if (t_thread) {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(Thread::task cb, const std::string &name) {
        m_cb = std::move(cb);

        if (name.empty()) {
            m_name = "UNKNOWN";
        } else {
            m_name = name;
        }
        int r = pthread_create(&m_thread, nullptr, &Thread::run, this);

        if (r) {
            MOCKER_LOG_ERROR(g_logger) << "pthread_create thread fail, r=" << r
                << " name=" << name;
            throw std::logic_error("pthread_create error");
        }

        m_semaphore.wait();
    }

    Thread::~Thread() {
        if (m_thread) {
            pthread_detach(m_thread);
        }
    }

    void Thread::join() {
        if (m_thread) {
            int r = pthread_join(m_thread, nullptr);
            if (r) {
                MOCKER_LOG_ERROR(g_logger) << "pthread_join thread fail, r=" << r
                                                      << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    void * Thread::run(void *arg) {
        auto* thread = (Thread*)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = getThreadId();
        // set name for thread
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        Thread::task cb;
        cb.swap(thread->m_cb);

        thread->m_semaphore.notify();

        cb();
        return nullptr;
    }


}