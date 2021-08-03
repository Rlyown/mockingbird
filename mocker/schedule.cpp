//
// Created by ChaosChen on 2021/8/1.
//

#include <mocker/schedule.h>
#include <mocker/log.h>
#include <mocker/macro.h>
#include <functional>

namespace mocker {
    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Coroutine *t_coroutine = nullptr;


    ////////////////////////////////////////////////////////////////////
    /// Scheduler
    ////////////////////////////////////////////////////////////////////
    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
            : m_name(name) {
        MOCKER_ASSERT(threads > 0);

        if (use_caller) {
            Coroutine::GetCurrent();
            --threads;

            MOCKER_ASSERT(GetCurrent() == nullptr);
            t_scheduler = this;

            m_rootCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true));
            Thread::SetCurrentName(m_name);

            t_coroutine = m_rootCoroutine.get();
            m_rootThread = GetThreadId();
            m_threadIds.push_back(m_rootThread);
        } else {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler() {
        MOCKER_ASSERT(m_stopping);
        if (GetCurrent() == this) {
            t_scheduler = nullptr;
        }

    }

    void Scheduler::start() {
        {
            MutexType::Lock lock(m_mutex);
            if (!m_stopping) {
                return;
            }
            m_stopping = false;
            MOCKER_ASSERT(m_threads.empty());
            m_threads.resize(m_threadCount);

            for (size_t i = 0; i < m_threadCount; ++i) {
                m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                              m_name + "_" + std::to_string(i)));
                m_threadIds.push_back(m_threads[i]->getId());
            }
        }
//        if (m_rootCoroutine) {
//            m_rootCoroutine->call();
//        }
    }

    void Scheduler::stop() {
        m_autoStop = true;

        if (m_rootCoroutine
            && m_threadCount == 0
            && (m_rootCoroutine->getState() == Coroutine::TERM
                || m_rootCoroutine->getState() == Coroutine::INIT)) {
            MOCKER_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

            if (stopping()) {
                return;
            }
        }

//        bool exit_on_this_coroutine = false;
        if (m_rootThread != -1) {
            MOCKER_ASSERT(GetCurrent() == this);
        } else {
            MOCKER_ASSERT(GetCurrent() != this);
        }

        m_stopping = true;
        for (size_t i = 0; i < m_threadCount; ++i) {
            tickle();
        }

        if (m_rootCoroutine) {
            tickle();
        }

        if (m_rootCoroutine) {
//            while (!stopping()) {
//                if (m_rootCoroutine->getState() == Coroutine::TERM
//                    || m_rootCoroutine->getState() == Coroutine::EXCEPT) {
//                    m_rootCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true));
//                    MOCKER_LOG_INFO(g_logger) << "root coroutine is terminated, reset";
//                    t_coroutine = m_rootCoroutine.get();
//                }
//                m_rootCoroutine->call();
//            }
            if (!stopping()) {
                m_rootCoroutine->call();
            }
        }

        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (const auto &thr : thrs) {
            thr->join();
        }

//        if (exit_on_this_coroutine) {
//        }
    }

    void Scheduler::tickle() {
        MOCKER_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping() {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping
               && m_coroutines.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::idle() {
        MOCKER_LOG_INFO(g_logger) << "idle";
        while (!stopping()) {
            mocker::Coroutine::Sleep();
        }
    }

    void Scheduler::run() {
        MOCKER_LOG_DEBUG(g_logger) << "call run()";
        setCurrent();

        if (GetThreadId() != m_rootThread) {
            t_coroutine = Coroutine::GetCurrent().get();
        }

        Coroutine::ptr idle_coroutine(new Coroutine(std::bind(&Scheduler::idle, this), false));
        Coroutine::ptr cb_coroutine;

        ContextOfExecute coe;
        while (true) {
            coe.reset();
            bool tickle_me = false;
            bool is_active = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_coroutines.begin();
                while (it != m_coroutines.end()) {
                    if (it->thread != -1 && it->thread != GetThreadId()) {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    MOCKER_ASSERT(it->coroutine || it->cb);
                    if (it->coroutine && it->coroutine->getState() == Coroutine::EXEC) {
                        ++it;
                        continue;
                    }

                    coe = *it;
                    m_coroutines.erase(it);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
            }

            if (tickle_me) {
                tickle();
            }

            if (coe.coroutine && (coe.coroutine->getState() != Coroutine::TERM
                                  && coe.coroutine->getState() != Coroutine::EXCEPT)) {
                coe.coroutine->swapIn();
                --m_activeThreadCount;

                if (coe.coroutine->getState() == Coroutine::READY) {
                    schedule(coe.coroutine);
                } else if (coe.coroutine->getState() != Coroutine::TERM
                           && coe.coroutine->getState() != Coroutine::EXCEPT) {
                    coe.coroutine->setState(Coroutine::HOLD);
                }

                coe.reset();
            } else if (coe.cb) {
                if (cb_coroutine) {
                    cb_coroutine->reset(coe.cb);
                } else {
                    cb_coroutine.reset(new Coroutine(coe.cb, false));
                }

                coe.reset();
                cb_coroutine->swapIn();
                --m_activeThreadCount;
                if (cb_coroutine->getState() == Coroutine::READY) {
                    schedule(cb_coroutine);
                    cb_coroutine.reset();
                } else if (cb_coroutine->getState() == Coroutine::EXCEPT
                           || cb_coroutine->getState() == Coroutine::TERM) {
                    cb_coroutine->reset(nullptr);
                } else {
                    // cb_coroutine->getState() != Coroutine::TERM
                    cb_coroutine->setState(Coroutine::HOLD);
                    cb_coroutine.reset();
                }
            } else {
                if (is_active) {
                    --m_activeThreadCount;
                }
                if (idle_coroutine->getState() == Coroutine::TERM) {
                    MOCKER_LOG_INFO(g_logger) << "idle coroutine terminated";
                    break;
                }

                ++m_idleThreadCount;
                idle_coroutine->swapIn();
                --m_idleThreadCount;

                if (idle_coroutine->getState() != Coroutine::TERM
                    && idle_coroutine->getState() != Coroutine::EXCEPT) {
                    idle_coroutine->setState(Coroutine::HOLD);
                }
            }
        }
    }

    void Scheduler::setCurrent() {
        t_scheduler = this;
    }

    Scheduler *Scheduler::GetCurrent() {
        return t_scheduler;
    }

    Coroutine *Scheduler::GetMainCoroutine() {
        return t_coroutine;
    }


}