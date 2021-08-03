//
// Created by ChaosChen on 2021/7/31.
//

#ifndef MOCKER_COROUTINE_H
#define MOCKER_COROUTINE_H

#include <memory>
#include <ucontext.h>
#include <functional>

#include <mocker/mutex.h>

namespace mocker {
    class Coroutine : public std::enable_shared_from_this<Coroutine> {
    public:
        typedef std::shared_ptr<Coroutine> ptr;
        typedef std::function<void()> task;

        enum State {
            INIT,
            READY,
            EXEC,
            HOLD,
            TERM,
            EXCEPT
        };

    private:
        Coroutine();

    public:
        explicit Coroutine(task cb, uint32_t stacksize = 0, bool use_caller = false);
        ~Coroutine();

        // reset state from INIT, TERM
        void reset(task cb);
        // swap to other thread's current
        void swapIn();
        // swap from other thread's current
        void swapOut();
        // swap to current
        void call();
        // swap from current
        void back();

        uint64_t getId() const { return m_id; }
        State getState() const { return m_state; }
        void setState(State state) { m_state = state; }
    public:
        // set current coroutine
        static void SetCurrent(Coroutine* cort);
        // get current coroutine
        static Coroutine::ptr GetCurrent();
        // release cpu, and set READY
        static void Yield();
        // release cpu, and set HOLD
        static void Sleep();

        // total number of coroutines
        static uint64_t TotalCoroutines();

        static void MainFunc();
        static void CallerMainFunc();

        static uint64_t GetCoroutineId();
    private:
        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;
        State m_state = INIT;

        ucontext_t m_ctx;
        void* m_stack = nullptr;

        task m_cb;
    };
}

#endif //MOCKER_COROUTINE_H
