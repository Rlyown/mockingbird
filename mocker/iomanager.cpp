//
// Created by ChaosChen on 2021/8/8.
//

#include <cerrno>
#include <cstring>
#include <fcntl.h>

#include <mocker/iomanager.h>
#include <mocker/log.h>
#include <mocker/macro.h>

namespace mocker {

    static mocker::Logger::ptr g_logger = MOCKER_LOG_SYSTEM(); /* NOLINT */

    IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
            : Scheduler(threads, use_caller, name) {
        m_epollFd = epoll_create(5000);
        MOCKER_ASSERT(m_epollFd > 0)

        int rt = pipe(m_tickleFds);
        MOCKER_ASSERT(rt == 0)

        epoll_event event{};
        memset(&event, 0, sizeof(epoll_event));
        event.events = READ | EPOLLET;
        event.data.fd = m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        MOCKER_ASSERT(rt == 0)

        rt = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        MOCKER_ASSERT(rt == 0)

        contextResize(32);

        start();
    }

    IOManager::~IOManager() {
        stop();
        close(m_epollFd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        for (auto &m_fdContext : m_fdContexts) {
            delete m_fdContext;
        }
    }

    int IOManager::addEvent(int fd, IOManager::Event event, Coroutine::task cb) {
        FdContext *fd_ctx = nullptr;
        {
            RWMutexType::ReadLock readLock(m_rwmutex);
            if ((int)m_fdContexts.size() > fd) {
                fd_ctx = m_fdContexts[fd];
            } else {
                readLock.unlock();
                RWMutexType::WriteLock writeLock(m_rwmutex);
                contextResize((size_t)(fd * 1.5));
                fd_ctx = m_fdContexts[fd];
            }
        }

        FdContext::MutexType::Lock lock(fd_ctx->m_mutex);
        if (fd_ctx->m_events & event) {
            MOCKER_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                       << " event=" << event << "fd_ctx.event=" << fd_ctx->m_events;
            MOCKER_ASSERT(!(fd_ctx->m_events & event))
        }

        int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | fd_ctx->m_events | event;
        epollEvent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epollFd, op, fd, &epollEvent);
        if (rt) {
            MOCKER_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", "
                                       << epollEvent.events << "): " << rt << " (" << errno << ") (" << strerror(errno)
                                       << ")";
            return -1;
        }

        ++m_pendingEventCount;
        fd_ctx->m_events = (Event) (fd_ctx->m_events | event);
        FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
        MOCKER_ASSERT(!event_ctx.scheduler
                      && !event_ctx.coroutine
                      && !event_ctx.cb)

        event_ctx.scheduler = Scheduler::GetCurrent();
        if (cb) {
            event_ctx.cb.swap(cb);
        } else {
            event_ctx.coroutine = Coroutine::GetCurrent();
            MOCKER_ASSERT(event_ctx.coroutine->getState() == Coroutine::EXEC)
        }

        return 0;
    }

    bool IOManager::delEvent(int fd, IOManager::Event event) {
        FdContext *fd_ctx = nullptr;
        {
            RWMutexType::ReadLock readLock(m_rwmutex);
            if ((int)m_fdContexts.size() <= fd) {
                return false;
            }
            fd_ctx = m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock(fd_ctx->m_mutex);
        if (!(fd_ctx->m_events & event)) {
            return false;
        }

        auto new_events = (Event) (fd_ctx->m_events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | new_events;
        epollEvent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epollFd, op, fd, &epollEvent);
        if (rt) {
            MOCKER_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", "
                                       << epollEvent.events << "): " << rt << " (" << errno << ") (" << strerror(errno)
                                       << ")";
            return false;
        }

        --m_pendingEventCount;
        fd_ctx->m_events = new_events;
        FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);

        return true;
    }

    bool IOManager::cancelEvent(int fd, IOManager::Event event) {
        FdContext *fd_ctx = nullptr;
        {
            RWMutexType::ReadLock readLock(m_rwmutex);
            if ((int)m_fdContexts.size() <= fd) {
                return false;
            }
            fd_ctx = m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock(fd_ctx->m_mutex);
        if (!(fd_ctx->m_events & event)) {
            return false;
        }

        auto new_events = (Event) (fd_ctx->m_events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | new_events;
        epollEvent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epollFd, op, fd, &epollEvent);
        if (rt) {
            MOCKER_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", "
                                       << epollEvent.events << "): " << rt << " (" << errno << ") (" << strerror(errno)
                                       << ")";
            return false;
        }

        fd_ctx->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }

    bool IOManager::cancelAll(int fd) {
        FdContext *fd_ctx = nullptr;
        {
            RWMutexType::ReadLock readLock(m_rwmutex);
            if ((int)m_fdContexts.size() <= fd) {
                return false;
            }
            fd_ctx = m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock(fd_ctx->m_mutex);
        if (!fd_ctx->m_events) {
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = 0;
        epollEvent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epollFd, op, fd, &epollEvent);
        if (rt) {
            MOCKER_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", "
                                       << epollEvent.events << "): " << rt << " (" << errno << ") (" << strerror(errno)
                                       << ")";
            return false;
        }

        if (fd_ctx->m_events & READ) {
            fd_ctx->triggerEvent(READ);
            --m_pendingEventCount;
        }
        if (fd_ctx->m_events & WRITE) {
            fd_ctx->triggerEvent(WRITE);
            --m_pendingEventCount;
        }

        MOCKER_ASSERT(fd_ctx->m_events == 0)
        return true;
    }

    IOManager *IOManager::GetCurrent() {
        return dynamic_cast<IOManager *> (Scheduler::GetCurrent());
    }

    void IOManager::tickle() {
        if (!hsaIdleThreads()) {
            return;
        }
        ssize_t rt = write(m_tickleFds[1], "T", 1);
        MOCKER_ASSERT(rt == 1)
    }

    bool IOManager::stopping() {
        return Scheduler::stopping() && m_pendingEventCount == 0;
    }

    void IOManager::idle() {
        static const int MAX_EVENTS = 64;

        auto *events = new epoll_event[MAX_EVENTS];
        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr) {
            delete[] ptr;
        });

        while (true) {
            if (stopping()) {
                MOCKER_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
                break;
            }

            int rt = 0;
            do {
                // ms
                static const int MAX_TIMEOUT = 5000;
                rt = epoll_wait(m_epollFd, events, MAX_EVENTS, MAX_TIMEOUT);
                if (rt < 0 && errno == EINTR) {
                } else {
                    break;
                }
            } while (true);

            for (int i = 0; i < rt; ++i) {
                epoll_event &epollEvent = events[i];
                if (epollEvent.data.fd == m_tickleFds[0]) {
                    uint8_t dummy;
                    while (read(m_tickleFds[0], &dummy, 1) == 1);
                    continue;
                }

                auto *fd_ctx = static_cast<FdContext *>(epollEvent.data.ptr);

                FdContext::MutexType::Lock lock(fd_ctx->m_mutex);
                if (epollEvent.events & (EPOLLERR | EPOLLHUP)) {
                    epollEvent.events |= READ | WRITE;
                }

                unsigned int real_events = 0;
                if (epollEvent.events & READ) {
                    real_events |= READ;
                }

                if (epollEvent.events & WRITE) {
                    real_events |= WRITE;
                }

                if ((fd_ctx->m_events & real_events) == 0) {
                    continue;
                }

                unsigned int left_events = (fd_ctx->m_events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                epollEvent.events = EPOLLET | left_events;

                int rt2 = epoll_ctl(m_epollFd, op, fd_ctx->fd, &epollEvent);
                if (rt2) {
                    MOCKER_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd_ctx->fd << ", "
                                               << epollEvent.events << "): " << rt2 << " (" << errno << ") ("
                                               << strerror(errno)
                                               << ")";
                    continue;
                }

                if (real_events & READ) {
                    fd_ctx->triggerEvent(READ);
                    --m_pendingEventCount;
                }
                if (real_events & WRITE) {
                    fd_ctx->triggerEvent(WRITE);
                    --m_pendingEventCount;
                }
            }

            Coroutine::ptr cur = Coroutine::GetCurrent();
            auto raw_ptr = cur.get();
            cur.reset();

            raw_ptr->swapOut();
        }

    }

    void IOManager::contextResize(size_t size) {
        MOCKER_ASSERT(size >= m_fdContexts.size())
        m_fdContexts.resize(size);

        for (size_t i = 0; i < m_fdContexts.size(); ++i) {
            if (!m_fdContexts[i]) {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = (int)i;
            }
        }
    }


    IOManager::FdContext::EventContext &IOManager::FdContext::getContext(IOManager::Event event) {
        switch (event) {
            case IOManager::READ:
                return IOManager::FdContext::read;
            case IOManager::WRITE:
                return IOManager::FdContext::write;
            default:
                MOCKER_ASSERT2(false, "getcontext")
        }
        throw std::invalid_argument("getContext invalid event");
    }

    void IOManager::FdContext::resetContext(IOManager::FdContext::EventContext &eventContext) {
        eventContext.scheduler = nullptr;
        eventContext.coroutine.reset();
        eventContext.cb = nullptr;
    }

    void IOManager::FdContext::triggerEvent(IOManager::Event event) {
        MOCKER_ASSERT(m_events & event)
        m_events = (Event) (m_events & ~event);
        EventContext &event_ctx = getContext(event);

        if (event_ctx.cb) {
            event_ctx.scheduler->schedule(&event_ctx.cb);
        } else if (event_ctx.coroutine) {
            event_ctx.scheduler->schedule(&event_ctx.coroutine);
        }
        event_ctx.scheduler = nullptr;
    }
}