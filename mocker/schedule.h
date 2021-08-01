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

    class Schedule {
    public:
        typedef std::shared_ptr<Schedule> ptr;
        typedef Mutex MutexType;

    public:
        explicit Schedule(size_t threads = 1, bool use_caller = true, const std::string& name = "");
        virtual ~Schedule();

        const std::string& getName() const { return m_name; }

        void start();
        void stop();

        template<class CortOrCb>
        void schedule(CortOrCb cc, int thread = -1);

        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end, int thread = -1);
    private:
        template<class CortOrCb>
        bool scheduleNoLock(CortOrCb cc, int thread = 0);

    protected:
        void tickle();

    public:
        static Schedule* GetCurrent();
        static Coroutine* GetMainCoroutine();

    private:
        struct ContextOfExecute {
            Coroutine::ptr coroutine;
            Thread::task cb;

            int thread;

            ContextOfExecute(Coroutine::ptr cort, int thr) : coroutine(std::move(cort)), thread(thr) {}
            ContextOfExecute(Coroutine::ptr* cort, int thr) : thread(thr) { coroutine.swap(*cort); }
            ContextOfExecute(Thread::task tk, int thr) : cb(std::move(tk)), thread(thr) {}
            ContextOfExecute(Thread::task* tk, int thr) : thread(thr) { cb.swap(*tk); }
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
    };



}

#endif //MOCKER_SCHEDULE_H
