//
// Created by ChaosChen on 2021/8/1.
//

#ifndef MOCKER_SCHEDULER_H
#define MOCKER_SCHEDULER_H

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
        /**
         * Add a task to the scheduler.
         * @tparam CortOrCb Task type. It can be std::function<void()> or Coroutine::ptr.
         * @param cc The task will be added to the scheduler.
         * @param thread Specify the thread to perform the task. If the value is -1,
         *               threads will be randomly allocated for execution.
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

        /**
         * Add a batch of task to the scheduler.
         * @tparam InputIterator The type of iterable container.
         * @param begin The begin iterator of the container.
         * @param end The end iterator of the container.
         */
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
        /**
         * The function that actually performs the operation of adding a task.
         * @tparam CortOrCb Task type. It can be std::function<void()> or Coroutine::ptr.
         * @param cc The function that actually performs the operation of adding a task.
         * @param thread Specify the thread to perform the task. If the value is -1,
         *               threads will be randomly allocated for execution.
         * @return Whether need a tickle.
         */
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

        bool hsaIdleThreads() const { return m_idleThreadCount > 0; }
    public:
        static Scheduler *GetCurrent();

        static Coroutine *GetMainCoroutine();

    private:
        /**
         * Tasks performed by the thread pool. The task can be a coroutine or a callback function.
         */
        struct ContextOfExecute {
            Coroutine::ptr coroutine;
            Coroutine::task cb;
            pid_t thread;

            /// Assign a coroutine to a thread
            ContextOfExecute(Coroutine::ptr cort, pid_t thr) : coroutine(std::move(cort)), thread(thr) {}

            /// Swap a coroutine in a thread
            ContextOfExecute(Coroutine::ptr *cort, pid_t thr) : thread(thr) { coroutine.swap(*cort); }

            /// Assign a callback function to a thread
            ContextOfExecute(Thread::task tk, pid_t thr) : cb(std::move(tk)), thread(thr) {}

            /// Swap a callback function in a thread
            ContextOfExecute(Thread::task *tk, pid_t thr) : thread(thr) { cb.swap(*tk); }

            /// Default constructor.
            ContextOfExecute() : thread(-1) {}

            void reset() {
                coroutine = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        MutexType m_mutex;
        /// Thread pool.
        std::vector<Thread::ptr> m_threads;
        /// Task list.
        std::list<ContextOfExecute> m_coroutines;
        /// Scheduler's name.
        std::string m_name;

        /// The main coroutine when use_caller is true.
        Coroutine::ptr m_rootCoroutine;

    protected:
        /// Threads' id in the pool.
        std::vector<pid_t> m_threadIds;
        /// Total number of threads in scheduler.
        size_t m_threadCount = 0;
        /// Number of running threads.
        std::atomic<size_t> m_activeThreadCount = {0};
        /// Number of idle threads.
        std::atomic<size_t> m_idleThreadCount = {0};
        /// A flag that the scheduler is stopping.
        bool m_stopping = true;
        /// A flag that the scheduler stop automatically.
        bool m_autoStop = false;
        /// The caller thread's id when use_caller is true.
        pid_t m_rootThread = 0;

    };


}

#endif //MOCKER_SCHEDULER_H
