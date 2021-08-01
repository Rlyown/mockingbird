//
// Created by ChaosChen on 2021/7/31.
//

#include <atomic>
#include <utility>
#include <mocker/coroutine.h>
#include <mocker/config.h>
#include <mocker/macro.h>
#include <mocker/log.h>

namespace mocker {
    static std::atomic<uint64_t> s_coroutine_id {0};
    static std::atomic<uint64_t> s_coroutine_count {0};

    static thread_local Coroutine* t_coroutine = nullptr;
    static thread_local Coroutine::ptr t_threadCoroutine = nullptr;

    static ConfigVar<uint32_t>::ptr g_coroutine_stack_size =
            Config::Lookup<uint32_t>("coroutine.stack_size",
                                     1024 * 1024,
                                     "coroutine stack size");

    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    class MallocStackAllocator {
    public:
        static void* Alloc(size_t size) {
            return malloc(size);
        }

        static void Dealloc(void* vp, size_t size) {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    ////////////////////////////////////////////////////////////////////
    /// Coroutine
    ////////////////////////////////////////////////////////////////////
    Coroutine::Coroutine() {
        m_state = EXEC;
        SetCurrent(this);

        if (getcontext(&m_ctx)) {
            MOCKER_ASSERT2(false, "getcontext");
        }

        ++s_coroutine_count;

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::Coroutine";
    }

    Coroutine::Coroutine(Coroutine::task cb, uint32_t stacksize)
            : m_id(++s_coroutine_id), m_cb(std::move(cb)) {
        ++s_coroutine_count;
        m_stacksize = stacksize ? stacksize : g_coroutine_stack_size->getValue();

        m_stack = StackAllocator::Alloc(m_stacksize);
        if (getcontext(&m_ctx)) {
            MOCKER_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Coroutine::MainFunc, 0);

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::Coroutine id=" << m_id;
    }

    Coroutine::~Coroutine() {
        --s_coroutine_count;
        if (m_stack) {
            MOCKER_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stacksize);
        } else {
            MOCKER_ASSERT(!m_cb);
            MOCKER_ASSERT(m_state == EXEC);

            Coroutine* cur = t_coroutine;
            if (cur == this) {
                SetCurrent(nullptr);
            }
        }

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::~Coroutine id=" << m_id;
    }

    void Coroutine::reset(Coroutine::task cb) {
        MOCKER_ASSERT(m_stack);
        MOCKER_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
        m_cb = cb;
        if (getcontext(&m_ctx)) {
            MOCKER_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Coroutine::MainFunc, 0);
        m_state = INIT;
    }

    void Coroutine::swapIn() {
        SetCurrent(this);
        MOCKER_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&t_threadCoroutine->m_ctx, &m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    void Coroutine::swapOut() {
        SetCurrent(t_threadCoroutine.get());
        if (swapcontext(&m_ctx, &t_threadCoroutine->m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    void Coroutine::SetCurrent(Coroutine *cort) {
        t_coroutine = cort;
    }

    Coroutine::ptr Coroutine::GetCurrent() {
        if (t_coroutine) {
            return t_coroutine->shared_from_this();
        }
        Coroutine::ptr main_cort(new Coroutine);
        MOCKER_ASSERT(t_coroutine == main_cort.get());
        t_threadCoroutine = main_cort;
        return t_coroutine->shared_from_this();
    }

    void Coroutine::Yield() {
        Coroutine::ptr cur = GetCurrent();
        cur->m_state = READY;
        cur->swapOut();
    }

    void Coroutine::Sleep() {
        Coroutine::ptr cur = GetCurrent();
        cur->m_state = HOLD;
        cur->swapOut();
    }

    uint64_t Coroutine::TotalCoroutines() {
        return s_coroutine_count;
    }

    void Coroutine::MainFunc() {
        Coroutine::ptr cur = GetCurrent();
        MOCKER_ASSERT(cur);

        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: " << ex.what();
        } catch (...) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: ";
        }

        /*
         * It will not cause OOM because ~Coroutine will deallocate the
         * whole coroutine stack.
         */
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();

        /*
         * Here will cause a share_ptr cycle reference. Because the stack
         * will not be deallocated, so the share_ptr count is one at least.
         * Also, weak_ptr is useless in this situation. Because weak_ptr.lock()
         * will create a template share_ptr, and this share_ptr can't
         * deallocated too.
         */
        MOCKER_ASSERT2(false, "never reached");
    }

    uint64_t Coroutine::GetCoroutineId() {
        if (t_coroutine) {
            return t_coroutine->getId();
        }
        return 0;
    }


}
