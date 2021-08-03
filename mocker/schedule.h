//
// Created by ChaosChen on 2021/8/1.
//

#ifndef MOCKER_SCHEDULE_H
#define MOCKER_SCHEDULE_H

#include <memory>
#include <utility>
#include <vector>
#include <list>

#include <mocker/mutex.h>
#include <mocker/coroutine.h>
#include <mocker/thread.h>

namespace mocker {

    class Scheduler {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;

    public:
        explicit Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");

        virtual ~Scheduler();

        const std::string &getName() const { return m_name; }

        void start();

        void stop();

        /*
         * schedule, schedule, scheduleNoLock
         * These three functions need implement in header file, not cpp file.
         * if not, it will cause ld error "undefined reference to".
         */
        template<class CortOrCb>
        void schedule(CortOrCb cc, pid_t thread = -1) {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = scheduleNoLock(cc, thread);
            }

            if (need_tickle) {
                tickle();
            }
        }

        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while (begin != end) {
                    need_tickle = scheduleNoLock(&(*begin), -1) || need_tickle;

                    ++begin;
                }
            }

            if (need_tickle) {
                tickle();
            }
        }

    private:
        template<class CortOrCb>
        bool scheduleNoLock(CortOrCb cc, pid_t thread) {
            bool need_tickle = m_coroutines.empty();
            ContextOfExecute coe(cc, thread);

            if (coe.coroutine || coe.cb) {
                m_coroutines.push_back(coe);
            }
            return need_tickle;
        }

    protected:
        virtual void tickle();

        virtual bool stopping();

        virtual void idle();

        void run();

        void setCurrent();

    public:
        static Scheduler *GetCurrent();

        static Coroutine *GetMainCoroutine();

    private:
        struct ContextOfExecute {
            Coroutine::ptr coroutine;
            Thread::task cb;

            pid_t thread;

            ContextOfExecute(Coroutine::ptr cort, pid_t thr) : coroutine(std::move(cort)), thread(thr) {}

            ContextOfExecute(Coroutine::ptr *cort, pid_t thr) : thread(thr) { coroutine.swap(*cort); }

            ContextOfExecute(Thread::task tk, pid_t thr) : cb(std::move(tk)), thread(thr) {}

            ContextOfExecute(Thread::task *tk, pid_t thr) : thread(thr) { cb.swap(*tk); }

            ContextOfExecute() : thread(-1) {}

            void reset() {
                coroutine = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        MutexType m_mutex;
        std::vector<Thread::ptr> m_threads;
        std::list<ContextOfExecute> m_coroutines;
        std::string m_name;

        Coroutine::ptr m_rootCoroutine;

    protected:
        std::vector<pid_t> m_threadIds;
        size_t m_threadCount = 0;
        std::atomic<size_t> m_activeThreadCount = {0};
        std::atomic<size_t> m_idleThreadCount = {0};
        bool m_stopping = true;
        bool m_autoStop = false;
        pid_t m_rootThread = 0;


    };


}

#endif //MOCKER_SCHEDULE_H
