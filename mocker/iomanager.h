//
// Created by ChaosChen on 2021/8/8.
//

#ifndef MOCKER_IOMANAGER_H
#define MOCKER_IOMANAGER_H

#include <memory>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <mocker/mutex.h>
#include <mocker/scheduler.h>

namespace mocker {
    class IOManager : public Scheduler {
    public:
        typedef std::shared_ptr<IOManager> ptr;
        typedef RWMutex RWMutexType;

        typedef EPOLL_EVENTS Event;
        static constexpr int NONE = 0;
        static constexpr Event WRITE = EPOLLOUT;
        static constexpr Event READ = EPOLLIN;

    private:
        struct FdContext {
            typedef Mutex MutexType;

            struct EventContext {
                /// Scheduler to be executed
                Scheduler *scheduler = nullptr;
                /// Event coroutine
                Coroutine::ptr coroutine = nullptr;
                /// event callback function
                Coroutine::task cb = nullptr;
            };

            EventContext &getContext(Event event);

            void resetContext(EventContext &eventContext);

            void triggerEvent(Event event);

            /// Read event
            EventContext read;
            /// Write event
            EventContext write;
            /// Event file handler
            int fd = 0;
            /// The event
            Event m_events = static_cast<Event>(0);
            MutexType m_mutex;
        };


    public:
        explicit IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "");

        ~IOManager() override;

        int addEvent(int fd, Event event, Coroutine::task cb = nullptr);

        bool delEvent(int fd, Event event);

        bool cancelEvent(int fd, Event event);

        bool cancelAll(int fd);

    public:
        static IOManager *GetCurrent();

    protected:
        void tickle() override;

        bool stopping() override;

        void idle() override;

    private:
        void contextResize(size_t size);

    private:
        /// Epoll file handler
        int m_epollFd = 0;
        /// Pipe
        int m_tickleFds[2]{};

        std::atomic<size_t> m_pendingEventCount = {0};
        RWMutexType m_rwmutex;
        std::vector<FdContext *> m_fdContexts;
    };
}

#endif //MOCKER_IOMANAGER_H
