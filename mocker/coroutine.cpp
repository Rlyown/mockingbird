//
// Created by ChaosChen on 2021/7/31.
//

#include <atomic>
#include <utility>
#include <mocker/coroutine.h>
#include <mocker/config.h>
#include <mocker/macro.h>
#include <mocker/log.h>
#include <mocker/scheduler.h>

namespace mocker {
    /// Coroutine id allocator.
    static std::atomic<uint64_t> s_coroutine_id{0};
    /// Number of coroutines.
    static std::atomic<uint64_t> s_coroutine_count{0};

    /// Current running coroutine.
    static thread_local Coroutine *t_coroutine = nullptr;
    /// Main coroutine in thread.
    static thread_local Coroutine::ptr t_threadCoroutine = nullptr;

    /// Global config of coroutine's default stack size.
    static ConfigVar<uint32_t>::ptr g_coroutine_stack_size =
            Config::Lookup<uint32_t>("coroutine.stack_size",
                                     1024 * 1024,
                                     "coroutine stack size");

    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    /**
     * Common memory allocator/de-allocator.
     */
    class MallocStackAllocator {
    public:
        /**
         * Allocate a memory to use.
         * @param size Number of bytes to allocate.
         * @return The point to the memory.
         */
        static void *Alloc(size_t size) {
            return malloc(size);
        }

        /**
         * Deallocate memory after using.
         * @param vp the memory point.
         * @param size the size of the memory.
         */
        static void Dealloc(void *vp, size_t size) {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    // /////////////////////////////////////////////////////////////////
    //  Coroutine
    // /////////////////////////////////////////////////////////////////
    /**
     * Private constructor. Use it to create the main_coroutine.
     */
    Coroutine::Coroutine() {
        m_state = EXEC;
        SetCurrent(this);

        if (getcontext(&m_ctx)) {
            MOCKER_ASSERT2(false, "getcontext");
        }

        ++s_coroutine_count;

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::Coroutine id=" << m_id;
    }

    /**
     * Public constructor.
     * @param cb Callback function executed by the coroutine.
     * @param stacksize Number of bytes for coroutine stack size. If set to 0, the default size of 1MB is used.
     * @param use_caller Whether to use the caller thread to create.
     */
    Coroutine::Coroutine(task cb, uint32_t stacksize, bool use_caller)
            : m_id(++s_coroutine_id), m_cb(std::move(cb)) {
        ++s_coroutine_count;
        // default stack size can be set by config file
        m_stacksize = stacksize ? stacksize : g_coroutine_stack_size->getValue();

        m_stack = StackAllocator::Alloc(m_stacksize);
        if (getcontext(&m_ctx)) {
            MOCKER_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        if (!use_caller) {
            makecontext(&m_ctx, &Coroutine::MainFunc, 0);
        } else {
            makecontext(&m_ctx, &Coroutine::CallerMainFunc, 0);
        }

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::Coroutine id=" << m_id;
    }

    /**
     * Reclaim the memory of the coroutine stack, and clear the t_coroutine point.
     */
    Coroutine::~Coroutine() {
        --s_coroutine_count;
        if (m_stack) {
            MOCKER_ASSERT2(m_state == TERM || m_state == INIT || m_state == EXCEPT,
                           "m_state " + std::to_string(m_state));
            StackAllocator::Dealloc(m_stack, m_stacksize);
        } else {
            MOCKER_ASSERT(!m_cb);
            MOCKER_ASSERT(m_state == EXEC);

            Coroutine *cur = t_coroutine;
            if (cur == this) {
                SetCurrent(nullptr);
            }
        }

        MOCKER_LOG_DEBUG(g_logger) << "Coroutine::~Coroutine id=" << m_id;
    }

    /**
     * Reset the callback function in coroutine.
     * @param cb Callback function executed by the coroutine.
     */
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

    /**
     * Replace the scheduler's main_coroutine with sub_coroutine. Invoke sub_coroutine to run.
     */
    void Coroutine::swapIn() {
        SetCurrent(this);
        MOCKER_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&Scheduler::GetMainCoroutine()->m_ctx, &m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    /**
     * Replace the sub_coroutine with scheduler's main_coroutine. Invoke main_coroutine to run.
     */
    void Coroutine::swapOut() {
        SetCurrent(Scheduler::GetMainCoroutine());
        if (swapcontext(&m_ctx, &Scheduler::GetMainCoroutine()->m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    /**
     * Replace the thread's main_coroutine with sub_coroutine. Invoke sub_coroutine to run.
     */
    void Coroutine::call() {
        SetCurrent(this);
        MOCKER_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&t_threadCoroutine->m_ctx, &m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    /**
     * Replace the sub_coroutine with thread's main_coroutine. Invoke main_coroutine to run.
     */
    void Coroutine::back() {
        SetCurrent(t_threadCoroutine.get());
        if (swapcontext(&m_ctx, &t_threadCoroutine->m_ctx)) {
            MOCKER_ASSERT2(false, "swapcontext")
        }
    }

    /**
     * Set a coroutine to run
     * @param cort The coroutine ready to run
     */
    void Coroutine::SetCurrent(Coroutine *cort) {
        t_coroutine = cort;
    }

    /**
     * Get the current running coroutine. If nothing to run, it will create a main_coroutine to run.
     * @return Share_ptr of current coroutine.
     */
    Coroutine::ptr Coroutine::GetCurrent() {
        if (t_coroutine) {
            return t_coroutine->shared_from_this();
        }
        Coroutine::ptr main_cort(new Coroutine);
        MOCKER_ASSERT(t_coroutine == main_cort.get());
        t_threadCoroutine = main_cort;
        return t_coroutine->shared_from_this();
    }

    /**
     * Give up the CPU and set ready.
     */
    void Coroutine::Yield() {
        Coroutine::ptr cur = GetCurrent();
        cur->m_state = READY;
        cur->swapOut();
    }

    /**
     * Give up the CPU and set hold.
     */
    void Coroutine::Sleep() {
        Coroutine::ptr cur = GetCurrent();
        cur->m_state = HOLD;
        cur->swapOut();
    }

    /**
     * Get the total coroutines in the whole progress.
     * @return Number of coroutines.
     */
    uint64_t Coroutine::TotalCoroutines() {
        return s_coroutine_count;
    }

    /**
     * Run the callback function. And it will call Coroutine::swapOut to release after running over.
     * It will be used by the scheduler so that the management of the coroutine can be applied to multiple threads.
     */
    void Coroutine::MainFunc() {
        Coroutine::ptr cur = GetCurrent();
        MOCKER_ASSERT(cur);

        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception &ex) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: " << ex.what()
                                       << "\n" << BacktraceToString(100);
        } catch (...) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: "
                                       << "\n" << BacktraceToString(100);
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

    /**
     * Run the callback function. And it will call Coroutine::back to release after running over.
     * It is only used inside a single thread.
     */
    void Coroutine::CallerMainFunc() {
        Coroutine::ptr cur = GetCurrent();
        MOCKER_ASSERT(cur);

        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception &ex) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: " << ex.what()
                                       << "\n" << BacktraceToString(100);
        } catch (...) {
            cur->m_state = EXCEPT;
            MOCKER_LOG_ERROR(g_logger) << "Coroutine Exception: "
                                       << "\n" << BacktraceToString(100);
        }

        /*
         * It will not cause OOM because ~Coroutine will deallocate the
         * whole coroutine stack.
         */
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->back();

        /*
         * Here will cause a share_ptr cycle reference. Because the stack
         * will not be deallocated, so the share_ptr count is one at least.
         * Also, weak_ptr is useless in this situation. Because weak_ptr.lock()
         * will create a template share_ptr, and this share_ptr can't
         * deallocated too.
         */
        MOCKER_ASSERT2(false, "never reached");
    }

    /**
     * Get the current coroutine's id.
     * @return Coroutine's id.
     */
    uint64_t Coroutine::GetCurrentId() {
        if (t_coroutine) {
            return t_coroutine->getId();
        }
        return 0;
    }


}
