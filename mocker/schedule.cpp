//
// Created by ChaosChen on 2021/8/1.
//

#include <mocker/schedule.h>
#include <mocker/log.h>

namespace mocker {
    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    ////////////////////////////////////////////////////////////////////
    /// Schedule
    ////////////////////////////////////////////////////////////////////
    Schedule::Schedule(size_t threads, bool use_caller, const std::string &name) {

    }

    Schedule::~Schedule() {

    }

    void Schedule::start() {

    }

    void Schedule::stop() {

    }

    template<class CortOrCb>
    void Schedule::schedule(CortOrCb cc, int thread) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(cc, thread);
        }

        if (need_tickle) {
            tickle();
        }

    }

    template<class CortOrCb>
    bool Schedule::scheduleNoLock(CortOrCb cc, int thread) {
        bool need_tickle = m_coroutines.empty();
        ContextOfExecute cot(cc, thread);

        if (cot.coroutine || cot.cb) {
            m_coroutines.push_back(cot);
        }
        return need_tickle;
    }

    template<class InputIterator>
    void Schedule::schedule(InputIterator begin, InputIterator end, int thread) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&(*begin), thread) || need_tickle;

                ++begin;
            }
        }

        if (need_tickle) {
            tickle();
        }
    }

    void Schedule::tickle() {

    }

    Schedule *Schedule::GetCurrent() {
        return nullptr;
    }

    Coroutine *Schedule::GetMainCoroutine() {
        return nullptr;
    }
}